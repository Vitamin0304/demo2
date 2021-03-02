#ifndef APPS_LED_PWM_H_
#define APPS_LED_PWM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ints.h"

void M1PWM567Init(uint32_t freq, float duty, bool state[3]);
void M1PWM567SetDuty(float duty, uint32_t ui32Gen, uint32_t ui32PWMOut, uint32_t ui32PWMOutBits);

#endif
