/*
 * eeprom.c
 *
 *  Created on: 2020��9��5��
 *      Author: ����
 */

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"

#include "Apps/my_eeprom.h"
void EEPROMMyInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    SysCtlDelay(2);
}
void EEPROMPIDWrite(EEPROM_PID_DATA* PIDData,uint32_t addr)
{
    EEPROMProgram((uint32_t *)PIDData, addr, sizeof(*PIDData));
}
void EEPROMPIDRead(EEPROM_PID_DATA* PIDData,uint32_t addr)
{
    EEPROMRead((uint32_t *)PIDData, addr, sizeof(*PIDData));
}
