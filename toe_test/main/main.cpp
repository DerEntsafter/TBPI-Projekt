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

    // 2. Initialize the VL53L0X Sensor
    VL53L0X sensor(I2C_MASTER_NUM);

    if (!sensor.init()) {
        printf("Failed to initialize VL53L0X!\n");
        vTaskDelay(portMAX_DELAY); 
    }
    printf("VL53L0X initialized! Base setup complete.\n");

    // ==========================================
    // 3. CALIBRATION ROUTINE
    // ==========================================
    
    // -> Offset Calibration
    printf("\n--- CALIBRATION PHASE ---\n");
    printf("Place a target exactly 100mm from the sensor.\n");
    printf("Starting Offset Calibration in 5 seconds...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (sensor.performOffsetCalibration(100)) {
        printf("Offset Calibration Done.\n");
    } else {
        printf("Offset Calibration Failed.\n");
    }

    // -> Cross-Talk (XTalk) Calibration
    printf("\nPlace a target exactly 400mm from the sensor.\n");
    printf("Starting XTalk Calibration in 5 seconds...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));

    if (sensor.performXTalkCalibration(400)) {
        printf("XTalk Calibration Done.\n");
    } else {
        printf("XTalk Calibration Failed.\n");
    }
    printf("--- CALIBRATION COMPLETE ---\n\n");

    // ==========================================
    // 4. Ranging Loop
    // ==========================================
    while (1) {
        uint16_t distance_mm = 0;
        
        bool success = sensor.read(&distance_mm);
        
        if (success) {
            printf("Distance: %d mm\n", distance_mm);
        } else {
            printf("Measurement failed or out of range.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay between reads
    }
}