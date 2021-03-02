/*
 * MyOLED.h
 *
 *  Created on: 2020年10月8日
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_MYOLED_H_
#define MYLIBRARY_MYOLED_H_

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
#include "Apps/delay.h"
#include "MyOledFont.h"


#define OLED_CMD  0 //写命令
#define OLED_DATA 1 //写数据

#define OLED_SYST   SYSCTL_PERIPH_GPIOB
#define OLED_PORT   GPIO_PORTB_BASE
#define OLED_SCL    GPIO_PIN_2
#define OLED_SDA    GPIO_PIN_3

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint16_t u16;

void delay(uint32_t t);
void OLED_ClearPoint(u8 x,u8 y);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u8 dat);
void OLED_WR_Byte(u8 dat,u8 cmd);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y);
u32 OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2);
void OLED_DrawCircle(u8 x,u8 y,u8 r);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1);
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1);
void OLED_ScrollDisplay(u8 num,u8 space);
void OLED_WR_BP(u8 x,u8 y);
void OLED_ShowPicture(u8 x0,u8 y0,u8 x1,u8 y1,u8 BMP[]);
void OLED_Init(void);


/**********************************IIC底层函数**************************************/
void OLED_SDIN_Set(void);
void OLED_SDIN_Clr(void);
void OLED_SCLK_Set(void);
void OLED_SCLK_Clr(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);

/*******************************************电动跷跷板专用**************************************************/
extern uint32_t Balance_Time;
void Balance_Display(void);
void TBalance_Display(int* time,uint8_t ifclearsed);
void TTASK_Display(char Task,uint8_t t,uint8_t ifclear);

/*******************************************2020省电赛专用**************************************************/
void TimeUsed_Display(uint32_t timeInt,uint32_t timeDec,uint8_t ifclearsed);
#endif /* MYLIBRARY_MYOLED_H_ */
