/*
 * bluetooth.c
 *
 *  Created on: 2020年9月10日
 *      Author: 电脑
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "Apps/bluetooth.h"
#include "Apps/motor.h"
#include "utils/uartstdio.h"
#include "CarController.h"
#include "OLED/MyOLED.h"

uint8_t uart1RxBuff[UART1_RX_BUFF_SIZE];

uint32_t uart1RxLength = 0;
//UART1

EEPROM_PID_DATA* BlueToothPIDData;

void BlueToothInit(uint32_t ui32Baud,EEPROM_PID_DATA* PIDData)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), ui32Baud,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
//    UARTFIFODisable(UART1_BASE);
    UARTFIFOEnable(UART1_BASE);//先使能
//    UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);//在打开2/8也就是1/4的深度

    UARTIntEnable(UART1_BASE, UART_INT_RX);
    UARTIntRegister(UART1_BASE, BlueToothIntHandler);
    IntEnable(INT_UART1);
    IntMasterEnable();

    BlueToothPIDData = PIDData;
}

void BlueToothIntHandler()
{
    float data[4];
    FLOAT_UNION floatUnion;
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART1_BASE, true);   //get interrupt status
    UARTIntClear(UART1_BASE, ui32Status);           //clear the asserted interrupts

    while(UARTCharsAvail(UART1_BASE))               //loop while there are chars
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_2);

        uint8_t rxChar = UARTCharGetNonBlocking(UART1_BASE);
        if(uart1RxLength == 0)
        {
            if(rxChar == 0xA5)
                uart1RxBuff[uart1RxLength++] = rxChar;
        }
        else
            uart1RxBuff[uart1RxLength++] = rxChar;
        UARTprintf("%c",rxChar);

        if(uart1RxLength == 19)
        {
            uart1RxLength = 0;
            if(uart1RxBuff[0] == 0xA5 && uart1RxBuff[18] == 0x5A)
            {
                for(int i=0;i<4;i++)
                {
                    for(int j=0;j<4;j++)
                        floatUnion.byte[j] = uart1RxBuff[1+i*4+j];
                    data[i] = floatUnion.value;
                }
                BlueToothPIDData->Kp = data[0];
//                BlueToothPIDData->Ki = data[1];
//                BlueToothPIDData->Kd = data[2];
                board_angle = data[3];
                motorSpeedGain = data[1];
                task2_set_time = data[2];
                if(task2_set_time > 20)
                    task2_set_time = 20;
                else if(task2_set_time < 6)
                    task2_set_time = 6;
                TimeUsed_Display(0,0,0);

                EEPROMPIDWrite(BlueToothPIDData,0x440);
            }
        }

//        if(uart1RxLength == 0)
//        {
//            if(rxChar == 0x21)
//                uart1RxBuff[uart1RxLength++] = rxChar;
//        }
//        else
//            uart1RxBuff[uart1RxLength++] = rxChar;
//
//
//        if(uart1RxLength == 8)
//        {
//            uart1RxLength = 0;
//            if(uart1RxBuff[0] == 0x21 && uart1RxBuff[1] == 0x23 && uart1RxBuff[7] == 0x2E)
//            {
////                BlueToothputs(uart1RxBuff,8);
//                //指令cm
//                if(uart1RxBuff[2] == 0x63 && uart1RxBuff[3] == 0x6D)
//                {
//                    data = uart1RxBuff[4] << 16 | uart1RxBuff[5] << 8 | uart1RxBuff[6];
//                    CarBlueToothQuery(data);
//                    BlueToothputs(&uart1RxBuff[2],5);
//                }
//            }
//        }
    }
}

void BlueToothputs(uint8_t* str, uint32_t len)
{
    for (int i = 0; i < len; ++i)
    {
        UARTCharPut(UART1_BASE, str[i]);
    }
}

void BlueToothSendGraph(float* pomega,float* poutput,float* pdistance)
{
    uint8_t sendBuf[15];
    uint8_t index = 0;
    uint8_t *pdata = (uint8_t*)pomega;
    uint8_t sumData = 0;

    sendBuf[index++] = 0xA5;
    for(uint8_t i = 0; i < 4; ++i, ++pdata)
    {
        sendBuf[index++] = *pdata;
        sumData += *pdata;
    }
    pdata = (uint8_t*)poutput;
    for(uint8_t i = 0; i < 4; ++i, ++pdata)
    {
        sendBuf[index++] = *pdata;
        sumData += *pdata;
    }
    pdata = (uint8_t*)pdistance;
    for(uint8_t i = 0; i < 4; ++i, ++pdata)
    {
        sendBuf[index++] = *pdata;
        sumData += *pdata;
    }
    sendBuf[index++] = sumData;
    sendBuf[index++] = 0x5A;

    BlueToothputs(sendBuf,15);
}

