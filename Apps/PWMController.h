/*
 * PWMController.h
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */

#ifndef APPS_PWMCONTROLLER_H_
#define APPS_PWMCONTROLLER_H_

extern "C"
{
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ints.h"
}

namespace pwm
{

class PWMController
{
private:
    uint32_t base;
    uint32_t generator;
    uint32_t out[2];      //负值表示禁用
    uint32_t outBits[2];

    float frequency;
    float duty[2];

public:
    PWMController(uint32_t base,
                  uint32_t generator,
                  uint32_t out[2],
                  uint32_t outBits[2],
                  float frequency,
                  float duty[2]);
    void setFrequency(float frequency);
    void setDuty(float duty[2]);
    void setOutputState(bool state0, bool state1);
    virtual ~PWMController();
};

} /* namespace pwm */

#endif /* APPS_PWMCONTROLLER_H_ */
