/*
 * main.c
 *
 *  Created on: 10 set 2019
 *      Author: Max
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_app_format.h"

#include "tsdz_bt.h"
#include "tsdz_uart.h"
#include "tsdz_nvs.h"
#include "tsdz_data.h"
#include "tsdz_ota.h"
#include "tsdz_utils.h"
#include "tsdz_ds18b20.h"
#include "tsdz_tmp112.h"

#define APP_MAIN "app_main"

void mainTask(void * pvParameters);

TaskHandle_t mainTaskHandle = NULL;

void app_main(void)
{
	tsdz_nvs_init();
	tsdz_nvs_read_cfg();
	ESP_LOGI(APP_MAIN, "cfg read done");

	tsdz_bt_init();
	ESP_LOGI(APP_MAIN, "bt init done");

	// wait to avoid STM8 bootloader activation
	vTaskDelay(pdMS_TO_TICKS(600));

	tsdz_uart_init();
	ESP_LOGI(APP_MAIN, "uart init done");

	tsdz_tmp112_init();
	ESP_LOGI(APP_MAIN, "TMP112 init done");

	xTaskCreate( mainTask, "main_task", 2048, NULL, 5, &mainTaskHandle );
	if (mainTaskHandle == NULL) {
		ESP_LOGE(APP_MAIN, "main_task Start Task Error\n");
		ESP_LOGE(APP_MAIN, "ESP will restart in 3 seconds\n");
		vTaskDelay(pdMS_TO_TICKS(3000));
		esp_restart();
	}
}

void mainTask(void * pvParameters) {
	const TickType_t delay10ms = pdMS_TO_TICKS(10);

	// offset the periodic calls to avoid to call all the task on the same cycle
	static uint8_t bt_task_count   = 0;
	static uint8_t data_task_count = 1;
	static uint8_t motor_temp_task_count = 2;
	static uint8_t pcb_temp_task_count = 3;

	ESP_LOGI(APP_MAIN, "Main task started");

	TickType_t xLastWakeUpTime = xTaskGetTickCount();

	while (1) {
		// delay 10 ms from previous call
		vTaskDelayUntil(&xLastWakeUpTime, delay10ms);

		// uart send/receive task
		// run every 10msec
		tsdz_uart_task();

		// BT notification task
		// run every esp32_cfg.bt_update_delay * 10ms (default 250 ms or 4 notifications/sec)
		if (++bt_task_count >= (esp32_cfg.bt_update_delay)) {
			tsdz_bt_update();
			bt_task_count = 0;
		}

		// data calculation task (battery Wh consumption)
		// run every 100ms (10 * 10ms)
		if (++data_task_count >= 10) {
			tsdz_data_update();
			data_task_count = 0;
		}

		// read motor temperature every 1 sec (100 * 10ms)
		if (tsdz_cfg.ui8_esp32_temp_control) {
			motor_temp_task_count++;
			if (motor_temp_task_count==20) {
				// issue the start conversion command
				tzdz_ds18b20_start();
			} else if (motor_temp_task_count == 100) {
				// after 750 ms the conversion (12 bit) is done and the value could be readed
				tsdz_ds18b20_read();
				motor_temp_task_count = 0;
			}
		}

		// read PCB temperature every 1 sec (100 * 10ms)
		if (++pcb_temp_task_count >= 100) {
			tsdz_tmp112_read();
			pcb_temp_task_count = 0;
		}
	}
}
