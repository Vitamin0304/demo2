/*
 * Mymcu90615.h
 *
 *  Created on: 2020年9月23日
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_MYMCU90615_H_
#define MYLIBRARY_MYMCU90615_H_
#include "delay.h"
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
#include "driverlib/i2c.h"

extern float Tempre_Output;
/*****************************初始化部分*******************************************-->*/
/*红外测温模块初始化函数（更改后需要重启单片机）
 *默认波特率115200
 *如果初始化失败返回1，UART0输出"Invalid x!",x代表错误类型
 *Baud_Daya: 要配置的波特率,波特率设置只能是9600或者115200
 *nowdata:   原来的的波特率
 * */
uint32_t MCU90615_Init(uint32_t Baud_Data,uint32_t nowdata);


/*红外测温模块模式设置函数(设置后需在程序中删除该函数，重新烧录板子，再一次热重启才有用)
 *默认模式0，最好别用这个设置函数
 *Mode:  模式设置，0表示上电后不断地输出，1表示上电不自动输出才能输出，模式只能是0或1
 * */
uint32_t MCU90615_ModeSet(uint8_t Mode);
/*<---*****************************初始化部分**********************************************/





/*********************************输出格式切换指令****************************************--->*/
/*
 *注意：这两个函数也是具有改变输出Mode的功能的，可以自由切换输出模式
 * */
void MCU90615_AutoOutput(void);//自动输出
void MCU90615_DataGet(void);//寻访输出，使用该函数才能获取温度,配合Mode = 1一起使用
/*<---*****************************输出格式切换指令**********************************************/




//串口配置函数*************************************************************************
void _Uart5_Init(uint32_t Baud_Data);//波特率设置只能是9600或者115200
void _Uart5_int_Init(void);
//中断服务函数*************************************************************************
void _Uart5_int_exe(void);

#endif /* MYLIBRARY_MYMCU90615_H_ */
