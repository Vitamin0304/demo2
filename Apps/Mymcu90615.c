/*
 * 红外测温传感器函数库
 * 工作电压3——5V
 * UART协议       默认波特率115200
 * 板子引脚使用说明：
 * RXD:PE4
 * TXD:PE5
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "Mymcu90615.h"
//#include "MyBlueTooth.h"
float Tempre_Output = 0;
uint32_t MCU90615_Init(uint32_t Baud_Data,uint32_t nowdata)
{
    if(Baud_Data != nowdata)
    {
        if(nowdata != 9600 && nowdata != 115200)
         {
             UARTprintf("Invalid nowdata!");
             return 1;
         }

         _Uart5_Init(nowdata);
         _Uart5_int_Init();
         UARTFIFODisable(UART5_BASE);
         delay_1ms(1);

         if(Baud_Data == 9600)
         {
             UARTCharPut(UART5_BASE, 0xA5);
             UARTCharPut(UART5_BASE, 0xAE);
             UARTCharPut(UART5_BASE, 0x53);
         }
         else if(Baud_Data == 115200)
         {
             UARTCharPut(UART5_BASE, 0xA5);
             UARTCharPut(UART5_BASE, 0xAF);
             UARTCharPut(UART5_BASE, 0x54);
         }
         else
         {
             UARTprintf("Invalid Baud_Data!");
             return 1;
         }
    }

    _Uart5_Init(Baud_Data);
    _Uart5_int_Init();
    UARTFIFODisable(UART5_BASE);
    return 0;
}

uint32_t MCU90615_ModeSet(uint8_t Mode)
{
    if(Mode == 0)
    {
        UARTCharPut(UART5_BASE, 0xA5);
        UARTCharPut(UART5_BASE, 0x51);
        UARTCharPut(UART5_BASE, 0xF6);
    }
    else if(Mode == 1)
    {
        UARTCharPut(UART5_BASE, 0xA5);
        UARTCharPut(UART5_BASE, 0x52);
        UARTCharPut(UART5_BASE, 0xF7);
    }
    else
    {
        UARTprintf("Invalid Mode Data!");

    }
    return 1;
}

void MCU90615_AutoOutput(void)
{
    UARTCharPut(UART5_BASE, 0xA5);
    UARTCharPut(UART5_BASE, 0x45);
    UARTCharPut(UART5_BASE, 0xEA);
}

void MCU90615_DataGet(void)
{
    UARTCharPut(UART5_BASE, 0xA5);
    UARTCharPut(UART5_BASE, 0x15);
    UARTCharPut(UART5_BASE, 0xBA);
}






/*串口5及其中断的初始化*/
void _Uart5_Init(uint32_t Baud_Data)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5);
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(),Baud_Data, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void _Uart5_int_Init(void)
{
    UARTIntEnable(UART5_BASE, UART_INT_RX|UART_INT_TX);
    UARTIntRegister(UART5_BASE, _Uart5_int_exe);
    IntEnable(INT_UART5);
    IntMasterEnable();
}
/*串口5中断服务函数*/
////普通连接PC
//void _Uart5_int_exe(void)
//{
//
//     uint32_t status = UARTIntStatus(UART5_BASE, true);
//     uint32_t data = 0;
//     static uint8_t data_sum = 0;
//     static uint8_t  start_flag = 1;
//     static uint32_t data_num = 0;
//     static uint32_t  Tobject_high = 0;
//     static uint32_t  Tobject_low  = 0;
//     static uint32_t  Tenviro_high = 0;
//     static uint32_t  Tenviro_low  = 0;
//     uint32_t Output_T = 0;
//     UARTIntClear(UART5_BASE, status);
//     while(UARTCharsAvail(UART5_BASE))
//     {
//
//         data = UARTCharGetNonBlocking(UART5_BASE);
//
//         switch(data_num)
//         {
//         case 0:
//             if(data == 0x5A)
//             {
//                 data_sum += 0x5A;
//                 start_flag = 1;
//             }
//             else
//                 start_flag = 0;
//             break;
//         case 1:
//             if(data == 0x5A)
//             {
//                 data_sum += 0x5A;
//                 start_flag = 1;
//             }
//             else
//                 start_flag = 0;
//             break;
//         case 2:
//             if(data == 0x45)
//             {
//                 data_sum += 0x45;
//                 start_flag = 1;
//             }
//             else
//                 start_flag = 0;
//             break;
//         case 3:
//             if(data == 0x04)
//             {
//                 data_sum += 0x04;
//                 start_flag = 1;
//             }
//             else
//                 start_flag = 0;
//             break;
//         case 4:
//             Tobject_high = data;
//             data_sum += data;
//             break;
//         case 5:
//             Tobject_low  = data;
//             data_sum += data;
//             break;
//         case 6:
//             Tenviro_high = data;
//             data_sum += data;
//             break;
//         case 7:
//             Tenviro_low  = data;
//             data_sum += data;
//             break;
//         case 8:
//             if(data_sum == data)
//             {
//                 start_flag = 1;
//             }
//             else
//                 start_flag = 0;
//             break;
//         }
//
//         if(data_num == 5 && start_flag == 1)
//         {
//             Output_T = ((Tobject_high<<8) | Tobject_low);
//             UARTprintf("%d\n",Output_T);//电脑调试情况下使用
//         }
//
//
//
//         data_num++;
//         if(data_num == 9)
//         {
//             data_num = 0;
//             start_flag = 1;
//             data_sum = 0;
//         }
//     }
//
//}

//连接蓝牙
void _Uart5_int_exe(void)
{

     uint32_t status = UARTIntStatus(UART5_BASE, true);
     uint32_t data = 0;
     static uint32_t data_num = 0;
     static uint32_t flag = 0;
     static uint32_t  Tobject_high = 0;
     static uint32_t  Tobject_low  = 0;

     uint32_t Output_T = 0;
     UARTIntClear(UART5_BASE, status);
     while(UARTCharsAvail(UART5_BASE))
     {

         data = UARTCharGetNonBlocking(UART5_BASE);
                  switch(data_num)
                  {
                  case 0:
                       if(data == 0x5A)
                       {
                           flag = 1;
                       }
                       else
                           flag = 0;
                       break;
                  case 1:
                      if(data == 0x5A)
                      {
                          flag = 1;
                      }
                      else
                          flag = 0;
                      break;
                  case 4:
                      Tobject_high = data;
                      break;
                  case 5:
                      Tobject_low  = data;
                      break;

                  }

         if(data_num == 5 && flag == 1)
         {
             Output_T = ((Tobject_high<<8) | Tobject_low);
             UARTprintf("%d\n",Output_T);
             Tempre_Output = ((float)(Output_T))/100;

         }
         data_num++;
         if(data_num == 9)
         {
             data_num = 0;
             flag = 0;

         }
     }

}



