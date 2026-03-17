/*
 * ESP-Drone Firmware
 * * Copyright 2019-2020  Espressif Systems (Shanghai) 
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "nvs_flash.h"

#include "stm32_legacy.h"
#include "platform.h"
#include "system.h"
#include "commander.h" // Added to control thrust

#define DEBUG_MODULE "APP_MAIN"
#include "debug_cf.h"

// --- YOUR CUSTOM AUTONOMOUS FLIGHT SEQUENCE ---
void autonomous_flight_task(void *pvParameters) {
    // 1. Wait 10 seconds after boot-up so you can put it on the ground safely
    vTaskDelay(pdMS_TO_TICKS(10000)); 

    setpoint_t sp;
    memset(&sp, 0, sizeof(sp)); // Clear the flight data

    // Thrust values go from 0 to 65535. Adjust these later based on the drone's weight!
    uint16_t takeoff_thrust = 37000;
    uint16_t landing_thrust = 26000;

    // 2. Going Up! (Spins motors for ~2 seconds)
    sp.thrust = takeoff_thrust;
    for(int i = 0; i < 5; i++) {
        commanderSetSetpoint(&sp, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // 3. Hold for 1 second (Uses slightly less thrust to hover)
    sp.thrust = takeoff_thrust - 4000; 
    for(int i = 0; i < 20; i++) {
        commanderSetSetpoint(&sp, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // 4. Coming Down (Lowers motors for ~1.5 seconds)
    sp.thrust = landing_thrust;
    for(int i = 0; i < 15; i++) {
        commanderSetSetpoint(&sp, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // 5. Cut Motors Off
    sp.thrust = 0;
    commanderSetSetpoint(&sp, 3);

    // Delete this task so it doesn't run again until the drone is restarted
    vTaskDelete(NULL); 
}
// ----------------------------------------------


void app_main()
{
    /* initialize nvs flash prepare for Wi-Fi */
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /*Initialize the platform.*/
    if (platformInit() == false) {
        while (1);//if  firmware is running on the wrong hardware, Halt
    }

    /*launch the system task */
    systemLaunch();

    // --- START YOUR CUSTOM TASK ---
    xTaskCreate(autonomous_flight_task, "AutoFlight", 2048, NULL, 3, NULL);

}