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

    // 2. Initialize the VL53L0X Sensor using hard-coded calibration
    VL53L0X sensor(I2C_MASTER_NUM);

    // Paste the values you printed out here:
    uint32_t cal_refSpadCount = 3;         // <-- Replace with your value
    uint8_t cal_isApertureSpads = 0;       // <-- Replace with your value
    uint8_t cal_VhvSettings = 31;           // <-- Replace with your value
    uint8_t cal_PhaseCal = 1;              // <-- Replace with your value
    int32_t cal_offsetMicroMeter = 30000;      // <-- Replace with your value
    FixPoint1616_t cal_xTalk = 0;          // <-- Replace with your value

    if (!sensor.fastInit(cal_refSpadCount, cal_isApertureSpads, cal_VhvSettings, 
                         cal_PhaseCal, cal_offsetMicroMeter, cal_xTalk)) {
        printf("Failed to quickly initialize VL53L0X!\n");
        vTaskDelay(portMAX_DELAY); 
    }
    
    printf("VL53L0X fast initialized and ready!\n");

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