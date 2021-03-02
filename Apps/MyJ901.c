/*
 * MyJ901.c
 *
 *  Created on: 2020年9月29日
 *      Author: STARSKY
 */
#include "Apps/MyJ901.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "Apps/delay.h"

float J901_Output[2][3];
float pitchOffset = 5.5;
/*
 * 陀螺仪初始化函数
 * */
void J901_Init(void)
{
    J901_Uart2_Init();
    J901_Uart2_int_Init();
    UARTFIFODisable(UART2_BASE);
}

/*
 * 读取陀螺仪测量参数的使能函数
 * */
void J901_GetData(void)
{
    IntEnable(INT_UART2);
}


/*输入数据计算*/
uint32_t Output_Cal(Data_Mode Mode,uint8_t* Data_Cal,float* output)
{

    switch(Mode)
    {
    case Mode_NONE:
        return Ivalid_Data;
    case Mode_Angular:
        output[0] = ((float)((short)Data_Cal[1]<<8|Data_Cal[0]) *180/ 32768);
            if(output[0]>=180)
            {
                output[0] = output[0] - 360;
            }
        output[1] = ((float)((short)Data_Cal[3]<<8|Data_Cal[2]) *180/ 32768);
            if(output[1]>=90)
            {
                output[1] = output[1] - 360;
            }
            output[1] += pitchOffset;
        output[2] = ((float)((short)Data_Cal[5]<<8|Data_Cal[4]) *180/ 32768);
            if(output[2]>=180)
            {
                output[2] = output[2] - 360;
            }
        return 0;
    case Mode_AngularSP:
        output[0] = ((float)((short)Data_Cal[1]<<8|Data_Cal[0]) *2000/ 32768);
            if(output[0]>=2000)
            {
                output[0] = output[0] - 4000;
            }
        output[1] = ((float)((short)Data_Cal[3]<<8|Data_Cal[2]) *2000/ 32768);
            if(output[1]>=2000)
            {
                output[1] = output[1] - 4000;
            }
        output[2] = ((float)((short)Data_Cal[5]<<8|Data_Cal[4]) *2000/ 32768);
            if(output[2]>=2000)
            {
                output[2] = output[2] - 4000;
            }
            return 0;
    default:
        return Ivalid_Data;

    }

}


/*读取数据，输入读取间隔*/
//void ReadData(uint32_t delay)
//{
//    IntEnable(INT_UART1);
//    delay_1ms
//
//}













/*
 * 串口及其中断初始化
 * 目前使用串口1和陀螺仪通讯
 * */

void J901_Uart2_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTD_BASE+GPIO_O_CR) |= GPIO_PIN_7;
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;//重新锁定

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 115200,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}
void J901_Uart2_int_Init(void)
{
    UARTIntEnable(UART2_BASE, UART_INT_RX|UART_INT_TX);
    UARTIntRegister(UART2_BASE, J901_Uart2_int_exe);
    IntEnable(INT_UART2);
    IntMasterEnable();
}

void J901_Uart2_int_exe(void)
{

     uint32_t data;
     uint32_t status = UARTIntStatus(UART2_BASE, true);
     uint32_t Fail_Cal = 0;
     static uint32_t cnt = 0;
     static uint8_t start_flag = 0;
     static Data_Mode Mode = 0x00;
     static uint8_t Data_Sum = 0;
     static uint8_t Data_Cal[6] ;
     static uint8_t Read_Disable = 0;
     UARTIntClear(UART2_BASE, status);
     while(UARTCharsAvail(UART2_BASE))
     {
           data = UARTCharGetNonBlocking(UART2_BASE);
          // UARTCharPutNonBlocking(UART1_BASE, data);


           switch(data)
           {
           case 0x55:
               start_flag = 1;
               Data_Sum += 0x55;
//               UARTprintf("%c",data);

               break;

           }

//           UARTprintf("%c",data);
//记录读取数据的类型

           if(cnt == 1 && start_flag == 1)
           {
               Data_Sum += data;
//               UARTprintf("%c",data);
               switch(data)
               {
               case 0x00:
                   Mode = Mode_NONE;
                   break;
               case 0x51:
                   Mode = Mode_Acceleration;
                   break;
               case 0x52:
                   Mode = Mode_AngularSP;
                   Read_Disable += 1;
                   break;
               case 0x53:
                   Mode = Mode_Angular;
                   Read_Disable += 1;
                   break;
               case 0x54:
                   Mode = Mode_Magnetic;
               }
           }

           //记录读入参数
           if(cnt>=2 && cnt <= 7 && start_flag == 1)
           {
               Data_Sum += data;
               Data_Cal[cnt - 2] = (uint8_t)data;
              // UARTprintf("%c",data);
           }

           if(cnt>=8 && cnt<=9)
           {
               Data_Sum += data;
           }

//           //加和验证，计算及串口输出结果
           if(cnt == 10 && start_flag == 1)
           {
               //sum
//               UARTprintf("%d %d\n",data,Data_Sum);
               if(Data_Sum == data)
               {
                   switch(Mode)
                   {
                   case Mode_Angular:
                       Fail_Cal = Output_Cal(Mode,Data_Cal,J901_Output[0]);
                       if(Fail_Cal != Ivalid_Data)
                       {
//                           UARTprintf("%d\n",(int)J901_Output[0][2]);
                       }
                       break;
                   case Mode_AngularSP:
                       Fail_Cal = Output_Cal(Mode,Data_Cal,J901_Output[1]);
                       if(Fail_Cal != Ivalid_Data)
                       {
                          // UARTprintf("%d\n",(int)J901_Output[1][0]);
                       }
                       break;
                   }

               }


           }
//           //能开始接受数据才可以加计数器
           if(cnt < 11 && start_flag == 1)
           {
               cnt++;
           }
           //计数器到11，表明一个数据包的大小被接受完成
           if(cnt >= 11 && start_flag == 1)
           {
               Data_Sum = 0;
               Mode = Mode_NONE;
               start_flag = 0;
               cnt = 0;
               if(Read_Disable == 2)
               {
                   Read_Disable = 0;

                   IntDisable(INT_UART2);
               }

           }
     }

}

///*
// * 初始化定时器
// * */
//void _Timer1_32_Init(void)//100Hz
//{
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
//    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC|TIMER_CFG_SPLIT_PAIR);//32位寄存器
//    TimerPrescaleSet(TIMER1_BASE,TIMER_A,timer_prescale - 1);
//    TimerLoadSet(TIMER1_BASE, TIMER_A,SysCtlClockGet()/(timer_prescale * freq));//2^32 = 4294967296,
//    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
//    IntEnable(INT_TIMER1A);
//    IntMasterEnable();
//    TimerEnable(TIMER1_BASE, TIMER_A);
//
//}
//
//void _Timer1_int_exe(void)
//{
//    static uint32_t  timer1_cnt = 0;
//    TimerIntClear(TIMER1_BASE, TimerIntStatus(TIMER1_BASE, true));
//    timer1_cnt++;
//    if(timer1_cnt == 5)
//    {
//        timer1_cnt = 0;
//        J901_GetData();
////        BlueToothSend_float(J901_Output[0],3);
////        GPIOF_LED_TOGGLE();
////        UARTprintf("%d",TimerValueGet(TIMER1_BASE, TIMER_A));
//        //TimerDisable(TIMER1_BASE, TIMER_A);
//
//    }
//
//}
