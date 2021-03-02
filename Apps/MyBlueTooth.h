/*
 * MyBlueTooth.h
 *
 *  Created on: 2020年9月20日
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_MYBLUETOOTH_H_
#define MYLIBRARY_MYBLUETOOTH_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/fpu.h"
#include "Apps/delay.h"
#include "utils/uartstdio.h"
#include "Apps/bluetooth.h"

/*！！！串口宏定义！！！*/
#define   Uart_BlueTooth     UART1_BASE
#define UART3_RX_BUFF_SIZE  40

/*有关openmv命令的宏定义*/
#define   OpenMv_Reset       0x00
#define   Int_Trigger        0x01
#define   Uart4_Tese         0x02

//AT命令函数（只能在蓝牙没连接上位机,且已经将串口波特率配置为当前蓝牙波特率的时候使用，否则指令无效）***************************************
void Bluetooth_Test(void);                                    //对蓝牙好坏的检测
void Bluetooth_Name_Get(void);                                //获取蓝牙名字
void Bluetooth_Name_Set(uint8_t *name);                       //设置蓝牙名字，只有单片机重启后名字才会改变
void Bluetooth_Addr_Get(void);                                //获取蓝牙地址
void Bluetooth_Addr_Set(uint8_t *addr);                       //设置蓝牙地址，该函数不可用
void Bluetooth_Baud_Get(void);                                //获取蓝牙波特率
void Bluetooth_Baud_Set(uint32_t baud_data);                  //设置蓝牙波特率，直接填想要的波特率即可
void Bluetooth_Reset(void);                                   /*使蓝牙恢复出厂设置  名字HC-06  波特率9600
                                                              *需重新启动单片机，并且将单片机串口波特率调至9600才能继续通讯*/


/*蓝牙配置函数（只能在蓝牙没连接上位机的时候使用，否则指令无效）***************************************
**配置成功后会有OK显示，重启单片机后才适用于新的波特率*/
uint32_t BlueTooth_Init(uint32_t Baud_Data,uint32_t BlueTooBaud_Datanow);//初始化蓝牙模块（配置串口蓝牙的波特率）,初始化失败返回1

//蓝牙模块功能函数**********************************************************************
//void BlueToothputs(uint8_t* str, uint32_t len);               //发送字符串给蓝牙，str为字符串，len为字符串长度，不算上'\0'
uint32_t BlueToothSend_float(float* data,uint32_t datalen);         /*发送数据包给蓝牙，data[x]={float0,float1,...}
                                                                                                                                                                                即传入浮点型数组的首地址；datalen为传输浮点型数据的数量*/
//void BlueToothSendGraph(float* pomega,float* poutput);
//串口配置函数*************************************************************************
void _Uart1_Init(uint32_t Baud_Data);
void _Uart3_Init(uint32_t Baud_Data);
void _Uart1_int_Init(void);
void _Uart3_int_Init(void);

void _Uart3_int_exe(void);
#endif /* MYLIBRARY_MYBLUETOOTH_H_ */
//void BlueToothSendGraph(float* pomega,float* poutput,float* pdistance);
