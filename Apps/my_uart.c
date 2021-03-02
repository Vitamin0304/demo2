/*
 * my_uart.c
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "Apps/my_uart.h"

char uart0RxBuff[UART0_RX_BUFF_SIZE];
char uart2RxBuff[UART2_RX_BUFF_SIZE];

uint32_t uart2RxLength = 0;

//UART0

void UART0Init(uint32_t ui32Baud)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, ui32Baud, SysCtlClockGet());

    UARTFIFOEnable(UART0_BASE);//先使能
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX2_8, UART_FIFO_RX2_8);//在打开2/8也就是1/4的深度

    UARTIntEnable(UART0_BASE, UART_INT_RX);
    IntPrioritySet(INT_UART0, 0x1);
    UARTIntRegister(UART0_BASE, UART0IntHandler);
    IntEnable(INT_UART0);
    IntMasterEnable();
}

void UART0IntHandler()
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART0_BASE, true);   //get interrupt status
    UARTIntClear(UART0_BASE, ui32Status);           //clear the asserted interrupts

    static uint32_t uart0RxLength = 0;
    while(UARTCharsAvail(UART0_BASE))               //loop while there are chars
    {
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_2);

        if(uart0RxLength >= UART0_RX_BUFF_SIZE)        //缓冲区满
        {
            UARTprintf("%s",uart0RxBuff);
            uart0RxLength = 0;
            break;
        }
        char rxChar = UARTCharGetNonBlocking(UART0_BASE);
        if(rxChar == '\n')
        {
            uart0RxBuff[uart0RxLength++] = '\n';
            uart0RxBuff[uart0RxLength++] = '\0';
            UARTprintf("%s",uart0RxBuff);
            uart0RxLength = 0;
        }
        else if(rxChar == '\r')
        {
            ;
        }
        else
        {
            uart0RxBuff[uart0RxLength++] = rxChar;
        }
    }
}

//UART2

void UART2Init(uint32_t ui32Baud)
{


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;//解锁
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x01;//确认
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;//重新锁定

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), ui32Baud,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTIntEnable(UART2_BASE, UART_INT_RX);
    UARTIntRegister(UART2_BASE, UART2IntHandler);
    IntEnable(INT_UART2);
    IntMasterEnable();
}

void UART2IntHandler()
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART2_BASE, true);   //get interrupt status
    UARTIntClear(UART2_BASE, ui32Status);           //clear the asserted interrupts

    while(UARTCharsAvail(UART2_BASE))               //loop while there are chars
    {
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_2);

        if (uart2RxLength >= UART2_RX_BUFF_SIZE)        //缓冲区满
        {
            UART2puts(uart2RxBuff,uart2RxLength);
            uart2RxLength = 0;
            break;
        }
        char rxChar = UARTCharGetNonBlocking(UART2_BASE);
//        UARTprintf("%c", rxChar);
        if (rxChar == '\n')
        {
            uart2RxBuff[uart2RxLength++] = '\n';
            uart2RxBuff[uart2RxLength++] = '\0';
            UART2puts(uart2RxBuff,uart2RxLength);
            uart2RxLength = 0;
        }
        else if (rxChar == '\r')
        {
            ;
        }
        else
        {
            uart2RxBuff[uart2RxLength++] = rxChar;
        }
    }
}

void UART2puts(char* str, uint32_t len)
{
    for (int i = 0; i < len; ++i)
    {
        UARTCharPut(UART2_BASE, str[i]);
    }
}
