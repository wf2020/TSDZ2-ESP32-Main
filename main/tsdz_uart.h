/*
 * tsdz_uart.h
 *
 *  Created on: 2 set 2019
 *      Author: Max
 */

#ifndef MAIN_TSDZ_UART_H_
#define MAIN_TSDZ_UART_H_

#include "driver/gpio.h"

#define LCD_UART 			UART_NUM_1
#define CT_UART 			UART_NUM_2

#define CT_TX_PIN 			GPIO_NUM_17
#define CT_RX_PIN 			GPIO_NUM_16
#define LCD_TX_PIN 			GPIO_NUM_18
#define LCD_RX_PIN 			GPIO_NUM_19
#define LCD_TX_ALT_PIN 		GPIO_NUM_32  // alternate config for LCD TX Pin
#define LCD_RX_ALT_PIN 		GPIO_NUM_33  // alternate config for LCD RX Pin

void tsdz_uart_init(void);
void tsdz_uart_task(void);

#endif /* MAIN_TSDZ_UART_H_ */
