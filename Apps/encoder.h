/*
 * my_timer.h
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */

#ifndef APPS_ENCODER_H_
#define APPS_ENCODER_H_

#include <stdint.h>
#include <stdbool.h>

extern bool delayFlag;

extern int omega_sign[2];

void TimerInterruptInit(float delay, bool isPeriodic);
void Timer2AIntHandler();

void WTimer0Capture1Init();
void WTimer3Capture1Init();
void Timer1Capture01Init();
//void T1CCP01IntHandler();

void GetEncoderFreq(float* frequency);

//void WTimer3Capture0Init();
//void WTimer3Capture0IntHandler();
//void WTimer3Capture0IntHandler_2();
//void WTimer3Capture0GetFrequency(uint32_t times, float* freq, float* duty);

#endif /* APPS_ENCODER_H_ */
