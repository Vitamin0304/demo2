/*
 * buzzer.h
 *
 *  Created on: 2020��10��13��
 *      Author: ����
 */

#ifndef APPS_BUZZER_H_
#define APPS_BUZZER_H_

#include <stdint.h>
#include <stdbool.h>

void buzzerInit(float freq1,float duty);
void buzzerTurnOn();
void buzzerTurnOff();

#endif /* APPS_BUZZER_H_ */
