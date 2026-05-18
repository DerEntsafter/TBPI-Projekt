#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

// Pin-Definitionen für ESP32-C6 (kannst du anpassen)
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000 // 100 kHz Standard I2C Frequenz
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define VL53L0X_ADDR                0x29   // Standard I2C-Adresse des Sensors

static const char *TAG = "VL53L0X_App";

// Hilfsfunktion: Ein einzelnes Byte in ein Register schreiben
esp_err_t vl53l0x_write_reg(uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, VL53L0X_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

// Hilfsfunktion: Daten aus einem Register lesen
esp_err_t vl53l0x_read_reg(uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, VL53L0X_ADDR, &reg, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

void app_main(void) {
    ESP_LOGI(TAG, "Initialisiere I2C Master...");

    // 1. I2C Konfiguration
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

    ESP_LOGI(TAG, "I2C initialisiert. Starte Distanzmessung...");

    while (1) {
        // 2. Start der Messung (Register 0x00 auf 0x01 setzen)
        vl53l0x_write_reg(0x00, 0x01);

        // 3. Warten, bis die Messung fertig ist (Status prüfen)
        uint8_t status = 0;
        while ((status & 0x01) == 0) {
            vl53l0x_read_reg(0x14, &status, 1); // RESULT_RANGE_STATUS auslesen
            vTaskDelay(pdMS_TO_TICKS(5));       // Kurze Pause, um den Prozessor zu entlasten
        }

        // 4. Distanz auslesen (2 Bytes, ab Register 0x1E)
        uint8_t dist_data[2] = {0};
        vl53l0x_read_reg(0x1E, dist_data, 2);

        // Die Distanz ist ein 16-Bit Wert (High Byte zuerst)
        uint16_t distance_mm = (dist_data[0] << 8) | dist_data[1];

        // 5. Ausgabe in der Konsole
        // Werte über 8000 mm sind Out-of-Range Fehlercodes des Sensors
        if (distance_mm < 8000) {
            ESP_LOGI(TAG, "Distanz: %u mm", distance_mm);
        } else {
            ESP_LOGW(TAG, "Distanz: Out of Range");
        }

        // 6. Interrupt zurücksetzen, damit der Sensor bereit für die nächste Messung ist
        vl53l0x_write_reg(0x0B, 0x01);

        // 500 ms warten bis zur nächsten Messung
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}