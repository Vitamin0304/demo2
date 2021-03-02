/*
 * eeprom.h
 *
 *  Created on: 2020年9月5日
 *      Author: 电脑
 */

#ifndef APPS_MY_EEPROM_H_
#define APPS_MY_EEPROM_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct eeprom_pid_data
{
    float Kp;
    float Ki;
    float Kd;
    float K_theta;
}EEPROM_PID_DATA;

void EEPROMMyInit(void);
void EEPROMPIDWrite(EEPROM_PID_DATA* PIDData,uint32_t addr);
void EEPROMPIDRead(EEPROM_PID_DATA* PIDData,uint32_t addr);

#endif /* APPS_MY_EEPROM_H_ */
