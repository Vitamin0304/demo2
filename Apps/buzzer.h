/*
 * buzzer.h
 *
 *  Created on: 2020年10月13日
 *      Author: 电脑
 */

#ifndef APPS_BUZZER_H_
#define APPS_BUZZER_H_

#include <stdint.h>
#include <stdbool.h>

void buzzerInit(float freq1,float duty);
void buzzerTurnOn();
void buzzerTurnOff();

#endif /* APPS_BUZZER_H_ */
