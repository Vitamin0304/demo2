/*
 * motor_timer.h
 *
 *  Created on: 2020年8月17日
 *      Author: 电脑
 */

#ifndef APPS_MOTOR_TIMER_H_
#define APPS_MOTOR_TIMER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void InfraredInit();
void InfraredIntHandler();
void MotorPWMInit(float freq, float duty);
void WTimer0Capture1Init();
void WTimer5Capture0IntHandler();
float WTimer5Capture0GetFrequency(uint32_t times);
void ServoInit(float freq,float duty);

extern uint32_t SysTickTime;
void SysTickInit();
void SysTickIntHandler();
#endif /* APPS_MOTOR_TIMER_H_ */
