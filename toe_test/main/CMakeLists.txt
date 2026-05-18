idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_driver_gpio ESP32_VL53L0X spi_flash)