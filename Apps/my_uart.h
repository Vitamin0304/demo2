/*
 * my_uart.h
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */

#ifndef APPS_MY_UART_H_
#define APPS_MY_UART_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

#include "utils/uartstdio.h"

#define UART0_RX_BUFF_SIZE  20
#define UART2_RX_BUFF_SIZE  20

extern char uart0RxBuff[UART0_RX_BUFF_SIZE];
extern char uart2RxBuff[UART2_RX_BUFF_SIZE];

extern uint32_t uart2RxLength;

void UART0Init(uint32_t ui32Baud);
void UART0IntHandler();

void UART2Init(uint32_t ui32Baud);
void UART2IntHandler();
void UART2puts(char* str, uint32_t len);

#endif /* APPS_MY_UART_H_ */
