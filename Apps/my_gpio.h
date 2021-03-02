/*
 * my_gpio.h
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */

#ifndef APPS_MY_GPIO_H_
#define APPS_MY_GPIO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "Apps/led_pwm.h"

#include "utils/uartstdio.h"

void GPIOKey1Init(void);
void GPIOLEDInit(void);
void IntHandler_GPIOF(void);

void GPIOPE3Init(void);
uint32_t GetPE3HighTime(void);
void IntHandler_GPIOE();

#endif /* APPS_MY_GPIO_H_ */
