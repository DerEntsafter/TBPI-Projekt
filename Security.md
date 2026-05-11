# Security und sicherer Datenaustausch
## Allgemein und Keyerzeugung

Das Konzept soll auf einem Pre Shared Key Schlüsselaustausch aufbauen.

Dazu nutzen wir zu erst einen Algorithmus um die Keys zu erzeugen.

Die Bibliothek die verwendet wird, wird über **pip install pynacl bleak** installiert 


```python
from nacl.utils import random
from nacl.encoding import HexEncoder

key = random(32)

print(HexEncoder.encode(key).decode())

```
## Sender 

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sodium.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define PSK_HEX "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF"

typedef struct {
    uint32_t counter;
    char msg[8];
} Payload;

int main() {

    if (sodium_init() < 0) {
        return 1;
    }

    unsigned char key[crypto_secretbox_KEYBYTES];

    sodium_hex2bin(
        key,
        sizeof(key),
        PSK_HEX,
        strlen(PSK_HEX),
        NULL,
        NULL,
        NULL
    );

    Payload payload;

    payload.counter = 1;

    strncpy(payload.msg, "HELLO", sizeof(payload.msg));

    unsigned char nonce[crypto_secretbox_NONCEBYTES];

    randombytes_buf(nonce, sizeof(nonce));

    unsigned char ciphertext[
        sizeof(Payload) + crypto_secretbox_MACBYTES
    ];

    crypto_secretbox_easy(
        ciphertext,
        (unsigned char*)&payload,
        sizeof(payload),
        nonce,
        key
    );

    /*
        BLE Advertisement Payload
        -------------------------
        [nonce | ciphertext]
    */

    unsigned char adv_data[31];

    memset(adv_data, 0, sizeof(adv_data));

    int offset = 0;

    memcpy(adv_data + offset, nonce, 12);
    offset += 12;

    memcpy(
        adv_data + offset,
        ciphertext,
        sizeof(ciphertext)
    );

    offset += sizeof(ciphertext);

    int dev_id = hci_get_route(NULL);

    if (dev_id < 0) {
        perror("No Bluetooth device");
        return 1;
    }

    int sock = hci_open_dev(dev_id);

    if (sock < 0) {
        perror("HCI open failed");
        return 1;
    }

    /*
        BlueZ raw HCI command
    */

    uint8_t buf[32];

    memset(buf, 0, sizeof(buf));

    buf[0] = offset + 1;
    buf[1] = 0xFF;

    memcpy(buf + 2, adv_data, offset);

    if (hci_send_cmd(
            sock,
            0x08,
            0x0008,
            sizeof(buf),
            buf) < 0) {

        perror("Set advertising data failed");

        return 1;
    }

    uint8_t enable[] = {0x01};

    if (hci_send_cmd(
            sock,
            0x08,
            0x000A,
            sizeof(enable),
            enable) < 0) {

        perror("Enable advertising failed");

        return 1;
    }

    printf("Encrypted BLE advertisement sent\n");

    sleep(5);

    close(sock);

    return 0;
}
```

## Empfänger

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sodium.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define PSK_HEX "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF"

typedef struct {
    uint32_t counter;
    char msg[8];
} Payload;

int main() {

    if (sodium_init() < 0) {
        return 1;
    }

    unsigned char key[crypto_secretbox_KEYBYTES];

    sodium_hex2bin(
        key,
        sizeof(key),
        PSK_HEX,
        strlen(PSK_HEX),
        NULL,
        NULL,
        NULL
    );

    int dev_id = hci_get_route(NULL);

    int sock = hci_open_dev(dev_id);

    if (sock < 0) {
        perror("HCI open failed");
        return 1;
    }

    hci_le_set_scan_parameters(
        sock,
        0x01,
        htobs(0x0010),
        htobs(0x0010),
        0x00,
        0x00,
        1000
    );

    hci_le_set_scan_enable(
        sock,
        0x01,
        1,
        1000
    );

    printf("Scanning...\n");

    unsigned char buf[HCI_MAX_EVENT_SIZE];

    while (1) {

        int len = read(sock, buf, sizeof(buf));

        if (len < 0)
            continue;

        evt_le_meta_event *meta =
            (evt_le_meta_event*)(buf + (1 + HCI_EVENT_HDR_SIZE));

        if (meta->subevent != EVT_LE_ADVERTISING_REPORT)
            continue;

        uint8_t *ptr = meta->data + 1;

        /*
            Skip BLE parsing complexity
            Assume payload location
        */

        unsigned char nonce[crypto_secretbox_NONCEBYTES];

        memcpy(nonce, ptr + 2, 12);

        unsigned char *ciphertext =
            ptr + 14;

        int cipher_len =
            sizeof(Payload) + crypto_secretbox_MACBYTES;

        unsigned char decrypted[sizeof(Payload)];

        if (crypto_secretbox_open_easy(
                decrypted,
                ciphertext,
                cipher_len,
                nonce,
                key) == 0) {

            Payload *p = (Payload*)decrypted;

            printf("\nMESSAGE RECEIVED\n");
            printf("COUNTER: %u\n", p->counter);
            printf("MSG: %s\n", p->msg);

        } else {

            printf("Decrypt failed\n");
        }
    }

    close(sock);

    return 0;
}
```



