#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "VL53L0X.h"

// Your ESP32-C6 pins
#define I2C_MASTER_SDA_IO           6
#define I2C_MASTER_SCL_IO           7
#define I2C_MASTER_NUM              I2C_NUM_0 
#define I2C_MASTER_FREQ_HZ          400000 
#define TOF_XSHUT                   8

// Global Calibration Data
const uint32_t cal_refSpadCount = 3;         // <-- Replace with your value
const uint8_t cal_isApertureSpads = 0;       // <-- Replace with your value   
const uint8_t cal_VhvSettings = 31;           // <-- Replace with your value
const uint8_t cal_PhaseCal = 1;              // <-- Replace with your value
const int32_t cal_offsetMicroMeter = 21000;      // <-- Replace with your value
const FixPoint1616_t cal_xTalk = 0;          // <-- Replace with your value

//Forward declaration: tell the compiler this function exists lower down
void mess_avg(VL53L0X& sensor, int num_measurements, int interval_ms);

extern "C" void app_main(void)
{
    // 1. Configure the native ESP-IDF I2C Master Interface
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO; 
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO; 
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    printf("I2C Master initialized successfully.\n");

    // Initialize the VL53L0X Sensor and PASS THE XSHUT PIN
    VL53L0X sensor(I2C_MASTER_NUM, (gpio_num_t)TOF_XSHUT);
    
    printf("VL53L0X fast initialized and ready!\n");
    
    sensor.powerOff();

    // ==========================================
    // 4. Ranging Loop
    // ==========================================
    while (1) {
        mess_avg(sensor, 20, 50); 

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay between reads
    }
}

void mess_avg(VL53L0X& sensor, int num_measurements, int interval_ms) {
    // 1. Wake up and inject calibration data
    if (!sensor.fastInit(cal_refSpadCount, cal_isApertureSpads, cal_VhvSettings, 
                         cal_PhaseCal, cal_offsetMicroMeter, cal_xTalk)) {
        printf("Failed to quickly initialize VL53L0X!\n");
        return; // Abort this batch if wake fails
    }
    
    // 2. Perform measurements
    uint32_t total_distance = 0;
    int successful_reads = 0;

    for (int i = 0; i < num_measurements; i++) {
        uint16_t distance_mm = 0;
        
        // Use the passed sensor reference to read
        bool success = sensor.read(&distance_mm);
        
        if (success) {
            total_distance += distance_mm;
            successful_reads++;
        } else {
            printf("  Reading %d failed or out of range.\n", i + 1);
        }
        
        // Wait for the specified interval before the next measurement
        vTaskDelay(pdMS_TO_TICKS(interval_ms)); 
    }

    // Calculate and print the average if we had at least one good reading
    if (successful_reads > 0) {
        uint32_t average = total_distance / successful_reads;
        printf("--> Average Distance: %lu mm (from %d successful reads)\n\n", average, successful_reads);
    } else {
        printf("--> All measurements failed in this batch.\n\n");
    }

    // 3. Put the sensor back into Hardware Standby to save energy
    sensor.powerOff();
}