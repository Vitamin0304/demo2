/*
 * buzzer.c
 *
 *  Created on: 2020年10月13日
 *      Author: 电脑
 */
#include "Apps/buzzer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_pwm.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
// PE4 PWM
void buzzerInit(float freq1,float duty)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinConfigure(GPIO_PE4_M0PWM4);
    GPIOPinTypePWM(GPIO_PORTE_BASE,GPIO_PIN_4);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2,
                  PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, SysCtlClockGet() / freq1);
//    uint32_t pwmGenPeriod = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_3);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,
                     SysCtlClockGet() / freq1 * duty);
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true);
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
}

void buzzerTurnOn()
{
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
}

void buzzerTurnOff()
{
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, false);
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);
}
