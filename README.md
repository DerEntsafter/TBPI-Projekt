# [WIP] ESP32 basierter Datenerfassung mit ToF-Sensor

## Überblick
### 1. Systemarchitektur
<img width="654" height="307" alt="image" src="https://github.com/user-attachments/assets/54301c02-0c22-46c3-8f01-8d681b2c31d6" />

#### 1.1 Aufgaben des Sensors
Entwicklung und Implementierung eines energieeffizienten, eingebetteten Systems auf Basis eines **ESP32-Mikrocontrollers**, das präzise Abstandsdaten erfasst, diese kryptografisch sichert und drahtlos über Bluetooth Low Energy (BLE) bereitstellt.

##### Kernaufgaben des Systems

* **Datenerfassung (ToF-Sensor):** Zyklisches Auslesen von hochpräzisen Entfernungsdaten über einen *Time-of-Flight* (ToF)-Sensor (z. B. via I²C-Schnittstelle).
* **Datenverschlüsselung:** Absicherung der erfassten Rohdaten direkt auf dem ESP32, um die Privatsphäre zu schützen und Manipulationen auf dem Übertragungsweg zu verhindern.
* **Drahtlose Übertragung (BLE Advanced Advertising):** Strukturierung der verschlüsselten Daten in spezielle BLE-Advertising-Pakete. Das System agiert als Beacon und sendet diese Daten zyklisch und verbindungslos (Advanced Advertising) in die Umgebung, sodass autorisierte Empfänger die Daten ohne aktiven Verbindungsaufbau effizient erfassen können.

---

### 2. Technologiestack
* **Hardware:** ESP32-Entwicklungsboard, ToF-Sensor (VL53L0X)
* **Software/Frameworks:** C/C++, ESP-IDF
* **Protokolle & Sicherheit:** BLE 5.x (Non-connectable Advertising), AES-Verschlüsselung

### 3. Aktueller Software Stand

**Bluetooth Modul (inklusive Verschlüsselung):** 

Advanced Advertising: 
* **nicht funktionsfähig**
* aktuell bewirkt das Flashen der Firmware mit aktiviertem Nimble BLE Advanced Advertising einen Defekt des ESP32. Der Grund dahinter konnte noch nicht ausgemacht werden und wird weiter untersucht. Bis dahin ist das Modul nicht funktionsfähig
* mit Standard-Konfiguration: Lagacy Advertising gab es in der Vergangenheit keine Probleme
* **Verwendng auf eigene Gefahr**

Verschlüsselung:
* Verschlüsselung funktioniert aktuell mit hardcoded Schlüsselpaar. Nähere Informationen unter Security.md und [Wikipedia](https://en.wikipedia.org/wiki/Galois/Counter_Mode)
* für Production Code wird Flash-Encryption und Flash-Protection empfohlen

**ToF Modul:**


### 4. Ausblick

**On-Going Advertising Fehlersuche** (Notizen):
Reproduktion des Fehlers:
1. ```idf.py menuconfig```
2. Navigation Bluetooth > Nimble Options> BLE 5.x features > Enable extended advertising/ Enable support for extended advertising v2
3. ```idf.py build```
4. ```idf.py flash```
5. ESp32 wird nicht mehr erkannt ist aber online und wird per BLE Scanner erkannt.
6. [8C_FD_49_19_7C_A2 - 2026-06-10 13_14_49.csv](https://github.com/user-attachments/files/28792578/8C_FD_49_19_7C_A2.-.2026-06-10.13_14_49.csv)

**Refactoring**(Future)
