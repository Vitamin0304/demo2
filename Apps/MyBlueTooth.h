/*
 * MyBlueTooth.h
 *
 *  Created on: 2020��9��20��
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

/*���������ں궨�壡����*/
#define   Uart_BlueTooth     UART1_BASE
#define UART3_RX_BUFF_SIZE  40

/*�й�openmv����ĺ궨��*/
#define   OpenMv_Reset       0x00
#define   Int_Trigger        0x01
#define   Uart4_Tese         0x02

//AT�������ֻ��������û������λ��,���Ѿ������ڲ���������Ϊ��ǰ���������ʵ�ʱ��ʹ�ã�����ָ����Ч��***************************************
void Bluetooth_Test(void);                                    //�������û��ļ��
void Bluetooth_Name_Get(void);                                //��ȡ��������
void Bluetooth_Name_Set(uint8_t *name);                       //�����������֣�ֻ�е�Ƭ�����������ֲŻ�ı�
void Bluetooth_Addr_Get(void);                                //��ȡ������ַ
void Bluetooth_Addr_Set(uint8_t *addr);                       //����������ַ���ú���������
void Bluetooth_Baud_Get(void);                                //��ȡ����������
void Bluetooth_Baud_Set(uint32_t baud_data);                  //�������������ʣ�ֱ������Ҫ�Ĳ����ʼ���
void Bluetooth_Reset(void);                                   /*ʹ�����ָ���������  ����HC-06  ������9600
                                                              *������������Ƭ�������ҽ���Ƭ�����ڲ����ʵ���9600���ܼ���ͨѶ*/


/*�������ú�����ֻ��������û������λ����ʱ��ʹ�ã�����ָ����Ч��***************************************
**���óɹ������OK��ʾ��������Ƭ������������µĲ�����*/
uint32_t BlueTooth_Init(uint32_t Baud_Data,uint32_t BlueTooBaud_Datanow);//��ʼ������ģ�飨���ô��������Ĳ����ʣ�,��ʼ��ʧ�ܷ���1

//����ģ�鹦�ܺ���**********************************************************************
//void BlueToothputs(uint8_t* str, uint32_t len);               //�����ַ�����������strΪ�ַ�����lenΪ�ַ������ȣ�������'\0'
uint32_t BlueToothSend_float(float* data,uint32_t datalen);         /*�������ݰ���������data[x]={float0,float1,...}
                                                                                                                                                                                �����븡����������׵�ַ��datalenΪ���両�������ݵ�����*/
//void BlueToothSendGraph(float* pomega,float* poutput);
//�������ú���*************************************************************************
void _Uart1_Init(uint32_t Baud_Data);
void _Uart3_Init(uint32_t Baud_Data);
void _Uart1_int_Init(void);
void _Uart3_int_Init(void);

void _Uart3_int_exe(void);
#endif /* MYLIBRARY_MYBLUETOOTH_H_ */
//void BlueToothSendGraph(float* pomega,float* poutput,float* pdistance);
