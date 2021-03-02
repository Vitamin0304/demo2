/*
 * bluetooth.h
 *
 *  Created on: 2020年9月10日
 *      Author: 电脑
 */

#ifndef APPS_BLUETOOTH_H_
#define APPS_BLUETOOTH_H_

#include <Apps/my_eeprom.h>
#include <stdint.h>
#include <stdbool.h>

#define UART1_RX_BUFF_SIZE  20

extern uint8_t uart1RxBuff[UART1_RX_BUFF_SIZE];
extern uint32_t uart1RxLength;
//extern float* BlueToothRxFloats[3];

//定义结构和联合
typedef union
{
    unsigned char byte[4];
    float  value;
}FLOAT_UNION;

void BlueToothInit(uint32_t ui32Baud,EEPROM_PID_DATA* PIDData);
void BlueToothIntHandler();
void BlueToothputs(uint8_t* str, uint32_t len);
void BlueToothSendGraph(float* pomega,float* poutput,float* pdistance);


#endif /* APPS_BLUETOOTH_H_ */
