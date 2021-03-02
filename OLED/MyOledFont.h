/*
 * MyOledFont.h
 *
 *  Created on: 2020��10��8��
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

//����ASCII��
//ƫ����32
//ASCII�ַ���
//ƫ����32
//��С:12*6
/************************************6*8�ĵ���************************************/
//12*12 ASCII�ַ�������

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];
extern unsigned char Hzk1[22][16];
extern unsigned char Hzk2[3][72];
extern unsigned char Hzk3[4][128];
extern unsigned char Hzk4[8][512];






#endif /* MYLIBRARY_MYOLEDFONT_H_ */
