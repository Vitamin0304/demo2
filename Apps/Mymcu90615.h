/*
 * Mymcu90615.h
 *
 *  Created on: 2020��9��23��
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
/*****************************��ʼ������*******************************************-->*/
/*�������ģ���ʼ�����������ĺ���Ҫ������Ƭ����
 *Ĭ�ϲ�����115200
 *�����ʼ��ʧ�ܷ���1��UART0���"Invalid x!",x�����������
 *Baud_Daya: Ҫ���õĲ�����,����������ֻ����9600����115200
 *nowdata:   ԭ���ĵĲ�����
 * */
uint32_t MCU90615_Init(uint32_t Baud_Data,uint32_t nowdata);


/*�������ģ��ģʽ���ú���(���ú����ڳ�����ɾ���ú�����������¼���ӣ���һ��������������)
 *Ĭ��ģʽ0����ñ���������ú���
 *Mode:  ģʽ���ã�0��ʾ�ϵ�󲻶ϵ������1��ʾ�ϵ粻�Զ�������������ģʽֻ����0��1
 * */
uint32_t MCU90615_ModeSet(uint8_t Mode);
/*<---*****************************��ʼ������**********************************************/





/*********************************�����ʽ�л�ָ��****************************************--->*/
/*
 *ע�⣺����������Ҳ�Ǿ��иı����Mode�Ĺ��ܵģ����������л����ģʽ
 * */
void MCU90615_AutoOutput(void);//�Զ����
void MCU90615_DataGet(void);//Ѱ�������ʹ�øú������ܻ�ȡ�¶�,���Mode = 1һ��ʹ��
/*<---*****************************�����ʽ�л�ָ��**********************************************/




//�������ú���*************************************************************************
void _Uart5_Init(uint32_t Baud_Data);//����������ֻ����9600����115200
void _Uart5_int_Init(void);
//�жϷ�����*************************************************************************
void _Uart5_int_exe(void);

#endif /* MYLIBRARY_MYMCU90615_H_ */
