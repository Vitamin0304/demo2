/*
 * delay.h
 *
 *  Created on: 2020Äê9ÔÂ10ÈÕ
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_DELAY_H_
#define MYLIBRARY_DELAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

void delay_1ms(uint32_t delaytime);
void delay_1us(uint32_t delaytime);
void delay_1s(uint32_t delaytime);


#endif /* MYLIBRARY_DELAY_H_ */
