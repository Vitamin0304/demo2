/*
 * MyOledFont.h
 *
 *  Created on: 2020年10月8日
 *      Author: STARSKY
 */

#ifndef MYLIBRARY_MYOLEDFONT_H_
#define MYLIBRARY_MYOLEDFONT_H_
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

//常用ASCII表
//偏移量32
//ASCII字符集
//偏移量32
//大小:12*6
/************************************6*8的点阵************************************/
//12*12 ASCII字符集点阵

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];
extern unsigned char Hzk1[22][16];
extern unsigned char Hzk2[3][72];
extern unsigned char Hzk3[4][128];
extern unsigned char Hzk4[8][512];






#endif /* MYLIBRARY_MYOLEDFONT_H_ */
