/*
 * MyJ901.h
 *
 *  Created on: 2020年9月29日
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_MYJ901_H_
#define MYLIBRARY_MYJ901_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


extern float J901_Output[2][3];
extern float pitchOffset;
#define Ivalid_Data  1

#define timer_prescale    8
#define freq              100  //100Hz

typedef enum
{
    Mode_NONE = 0x00,
    Mode_Acceleration = 0x51,
    Mode_AngularSP = 0x52,
    Mode_Angular = 0x53,
    Mode_Magnetic = 0x54
}Data_Mode;

void J901_Init(void);//初始化陀螺仪
void J901_GetData(void);//获得陀螺仪数据
uint32_t Output_Cal(Data_Mode Mode,uint8_t* Data_Cal,float* output);//转换从陀螺仪获取数据为能读数据

//串口及其中断初始化函数
void J901_Uart2_Init(void);
void J901_Uart2_int_Init(void);
void J901_Uart2_int_exe(void);

////初始化定时器
//void _Timer1_32_Init(void);//100Hz
//void _Timer1_int_exe(void);

#endif /* MYLIBRARY_MYJ901_H_ */
