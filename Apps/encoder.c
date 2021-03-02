/*
 * my_timer.c
 *
 *  Created on: 2020年8月12日
 *      Author: 电脑
 */
#include <Apps/encoder.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "Apps/led_pwm.h"
#include "Apps/motor.h"
#include "Apps/MyJ901.h"
#include "Apps/LMT70.h"
//#include "Apps/Mymcu90615.h"
//#define PI 3.1415926f

float WT3CCP0Frequency = 0;
float WT3CCP0Duty = 0;
bool WT3CCP0Completed = false;
uint32_t intStatus;

uint32_t WT0CCP1Counter[2] = {0};
uint32_t WT3CCP1Counter[2] = {0};
uint32_t T1CCP0Counter[2] = {0};
uint32_t T1CCP1Counter[2] = {0};
//电机速度符号，true为正，false为负

int omega_sign[2] = {0,0};

float delay_s = 0;

//进入中断后变为false
bool delayFlag;

void TimerInterruptInit(float delay, bool isPeriodic)
{
    delay_s = delay;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    IntPrioritySet(INT_TIMER2A, 3);
    //使能TIMER0
    if(isPeriodic==true)
    {
        //TimerConfigure(TIMER2_BASE, TIMER_CFG_A_PERIODIC);//周期性计数模式
        TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);//周期性计数模式
    }
    else
    {
        TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);//单次计数模式
    }

    TimerLoadSet(TIMER2_BASE, TIMER_A,SysCtlClockGet() * delay - 1);//计数频率10HZ
    IntEnable(INT_TIMER0A);//NVIC
    //使能TIMER0A
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    //TIMEOUT标志位触发中断
    IntMasterEnable();
    TimerIntRegister(TIMER2_BASE,TIMER_A,Timer2AIntHandler);
    //master interrupt enable API for all interrupts
    TimerEnable(TIMER2_BASE, TIMER_A);
    //TIMER0A开始计数，当计数值等于TimerLoadSet，触发中断
}
void Timer2AIntHandler()
{
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    delayFlag = false;

    WT0CCP1Counter[0] = WT0CCP1Counter[1];
    WT0CCP1Counter[1] = TimerValueGet(WTIMER0_BASE, TIMER_B);
    WT3CCP1Counter[0] = WT3CCP1Counter[1];
    WT3CCP1Counter[1] = TimerValueGet(WTIMER3_BASE, TIMER_B);
//    T1CCP0Counter[0] = T1CCP0Counter[1];
//    T1CCP0Counter[1] = TimerValueGet(TIMER1_BASE, TIMER_A);
//    T1CCP1Counter[0] = T1CCP1Counter[1];
//    T1CCP1Counter[1] = TimerValueGet(TIMER1_BASE, TIMER_B);

//    LMT_DATA_GET();
//    MCU90615_DataGet();
//    J901_GetData();
}
void GetEncoderFreq(float* frequency)
{
    float value;
//    (ticks[0] < ticks[1]) ? (ticks[1] - ticks[0]) : (ticks[1] - ticks[0] + 0xFFFFFF);
    uint32_t ticks = WT0CCP1Counter[1] >= WT0CCP1Counter[0] ?
            WT0CCP1Counter[1] - WT0CCP1Counter[0] :
            WT0CCP1Counter[1] - WT0CCP1Counter[0] + 0xFFFFFF;
    value = (float)(ticks/2.0f/delay_s);
    if(value < 10000)
        frequency[0] = value;

    ticks = WT3CCP1Counter[1] >= WT3CCP1Counter[0] ?
            WT3CCP1Counter[1] - WT3CCP1Counter[0] :
            WT3CCP1Counter[1] - WT3CCP1Counter[0] + 0xFFFFFF;
    value = (float)(ticks/2.0f/delay_s);
    if(value < 10000)
        frequency[1] = value;
}

void T1CCP01IntHandler()
{
    uint32_t intStatus;
    intStatus = TimerIntStatus(TIMER1_BASE, true);
    TimerIntClear(TIMER1_BASE, intStatus);

    static int omega0_sign_before[3] = {0};
    static int omega1_sign_before[3] = {0};

    if(intStatus & TIMER_CAPA_EVENT)        //左边电机编码器
    {
        uint32_t PC5_Status = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_5);
        PC5_Status &= GPIO_PIN_5;
        if(PC5_Status)
            omega_sign[0] = 1;
        else
            omega_sign[0] = -1;

        omega0_sign_before[0] = omega0_sign_before[1];
        omega0_sign_before[1] = omega0_sign_before[2];
        omega0_sign_before[2] = omega_sign[0];

        if(omega0_sign_before[0] == 1 && omega0_sign_before[1] == 1 && omega0_sign_before[2] == 1)
            omega_sign[0] = 1;
        else
            omega_sign[0] = -1;
    }
    else if(intStatus & TIMER_CAPB_EVENT)
    {
        uint32_t PD3_Status = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3);
        PD3_Status &= GPIO_PIN_3;
        if(PD3_Status)
            omega_sign[1] = 1;
        else
            omega_sign[1] = -1;

        omega1_sign_before[0] = omega1_sign_before[1];
        omega1_sign_before[1] = omega1_sign_before[2];
        omega1_sign_before[2] = omega_sign[1];

        if(omega1_sign_before[0] == 1 && omega1_sign_before[1] == 1 && omega1_sign_before[2] == 1)
            omega_sign[1] = 1;
        else
            omega_sign[1] = -1;
    }
}

//PC5
void WTimer0Capture1Init()
{
    // 启用Timer4模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
    // 启用GPIO_M作为脉冲捕捉脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    // 配置GPIO脚为使用Timer4捕捉模式
    GPIOPinConfigure(GPIO_PC5_WT0CCP1);
    GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_5);
    // 为管脚配置弱上拉模式
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 配置使用Timer4的TimerA模块为沿触发加计时模式
    TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT_UP);
    // 使用上降沿触发
    TimerPrescaleSet(WTIMER0_BASE, TIMER_B, 255);//预分频256
    TimerControlEvent(WTIMER0_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    // 设置计数范围为0~0x8FFF
    TimerLoadSet(WTIMER0_BASE, TIMER_B, 0xFFFFFF);
    // 启动捕捉模块
    TimerEnable(WTIMER0_BASE, TIMER_B);
}

//PD3   与红外遥控共用一个定时器
void WTimer3Capture1Init()
{
    // 启用Timer4模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    // 启用GPIO_M作为脉冲捕捉脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // 配置GPIO脚为使用Timer4捕捉模式
    GPIOPinConfigure(GPIO_PD3_WT3CCP1);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_3);
    // 为管脚配置弱上拉模式
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 配置使用Timer4的TimerA模块为沿触发加计时模式
    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_COUNT_UP);
    // 使用上降沿触发
    TimerPrescaleSet(WTIMER3_BASE, TIMER_B, 255);//预分频256
    TimerControlEvent(WTIMER3_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    // 设置计数范围为0~0x8FFF
    TimerLoadSet(WTIMER3_BASE, TIMER_B, 0xFFFFFF);
    // 启动捕捉模块
//    TimerEnable(WTIMER3_BASE, TIMER_B);
    TimerEnable(WTIMER3_BASE, TIMER_BOTH);
}
//PB4 PB5
void Timer1Capture01Init()
{
    // 启用Timer4模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    // 启用GPIO_M作为脉冲捕捉脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    // 配置GPIO脚为使用Timer4捕捉模式
    GPIOPinConfigure(GPIO_PB4_T1CCP0);
    GPIOPinConfigure(GPIO_PB5_T1CCP1);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5);
    // 为管脚配置弱上拉模式
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 配置使用Timer4的TimerA模块为沿触发加计时模式
    TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_TIME_UP);
    // 使用上降沿触发
    TimerPrescaleSet(TIMER1_BASE, TIMER_A, 255);//预分频256
    TimerPrescaleSet(TIMER1_BASE, TIMER_B, 255);//预分频256
    TimerControlEvent(TIMER1_BASE, TIMER_BOTH, TIMER_EVENT_POS_EDGE);
    // 设置计数范围为0~0x8FFF
    TimerLoadSet(TIMER1_BASE, TIMER_A, 0xFFF);
    TimerLoadSet(TIMER1_BASE, TIMER_B, 0xFFF);
    // 注册中断处理函数以响应触发事件
    TimerIntRegister(TIMER1_BASE, TIMER_A, T1CCP01IntHandler);
    TimerIntRegister(TIMER1_BASE, TIMER_B, T1CCP01IntHandler);
    // 系统总中断开
    IntMasterEnable();
    // 时钟中断允许，中断事件为Capture模式中边沿触发
    TimerIntEnable(TIMER1_BASE, TIMER_CAPA_EVENT|TIMER_CAPB_EVENT);
    // NVIC中允许定时器A模块中断
    IntEnable(INT_TIMER1A);
    IntEnable(INT_TIMER1B);
    // 启动捕捉模块
    TimerEnable(TIMER1_BASE, TIMER_BOTH);
}



//PD6
//void WTimer5Capture0Init()
//{
//    // 启用Timer4模块
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);
//    // 启用GPIO_M作为脉冲捕捉脚
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // 配置GPIO脚为使用Timer4捕捉模式
//    GPIOPinConfigure(GPIO_PD6_WT5CCP0);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_6);
//    // 为管脚配置弱上拉模式
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // 配置使用Timer4的TimerA模块为沿触发加计时模式
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP);
//    // 使用上降沿触发
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 255);//预分频256
//    TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
//    // 设置计数范围为0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_A, 0xFFFFFF);
//    // 启动捕捉模块
//    TimerEnable(WTIMER5_BASE, TIMER_A);
//}
////PD7
//void WTimer5Capture1Init()
//{
//    // 启用Timer4模块
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);
//    // 启用GPIO_M作为脉冲捕捉脚
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // 配置GPIO脚为使用Timer4捕捉模式
//    GPIOPinConfigure(GPIO_PD7_WT5CCP1);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_7);
//    // 为管脚配置弱上拉模式
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // 配置使用Timer4的TimerA模块为沿触发加计时模式
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT_UP);
//    // 使用上降沿触发
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_B, 255);//预分频256
//    TimerControlEvent(WTIMER5_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
//    // 设置计数范围为0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_B, 0xFFFFFF);
//    // 启动捕捉模块
//    TimerEnable(WTIMER5_BASE, TIMER_B);
//}


//PWM输入捕获 PD2

//void WTimer3Capture0Init()
//{
//    // 启用Timer4模块
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
//    // 启用GPIO_M作为脉冲捕捉脚
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // 配置GPIO脚为使用Timer4捕捉模式
//    GPIOPinConfigure(GPIO_PD2_WT3CCP0);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_2);
//    // 为管脚配置弱上拉模式
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // 配置使用Timer4的TimerA模块为沿触发加计时模式
//    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
//    // 使用上降沿触发
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 255);//预分频256
//    TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
//    // 设置计数范围为0~0x8FFF
//    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFF);
//    // 注册中断处理函数以响应触发事件
//    TimerIntRegister(WTIMER3_BASE, TIMER_A, WTimer3Capture0IntHandler_2);
//    // 系统总中断开
//    IntMasterEnable();
//    // 时钟中断允许，中断事件为Capture模式中边沿触发
//    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
//    // NVIC中允许定时器A模块中断
//    IntEnable(INT_WTIMER3A);
//    // 启动捕捉模块
//    TimerEnable(WTIMER3_BASE, TIMER_A);
//}
//
//void WTimer3Capture0IntHandler()
//{
//    intStatus = TimerIntStatus(WTIMER3_BASE, TIMER_CAPA_EVENT);
//    TimerIntClear(WTIMER3_BASE, intStatus);
//
//    static uint32_t ticks[3] = { 0.0 };
//    static uint32_t tickCount = 0;
//    uint32_t period[2];
//
//    //ticks[tickCount++] = TimerValueGet(WTIMER2_BASE, TIMER_A);
////    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1|GPIO_PIN_3);
//
//    switch(tickCount)
//    {
//    case 0:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //第一次捕获，改为下降沿
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        break;
//    case 1:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //第二次捕获，改为上升沿
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        break;
//    case 2:
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        period[0] = (ticks[0] < ticks[1]) ? (ticks[1] - ticks[0]) : (ticks[1] - ticks[0] + 0xFFFFFF);
//        period[1] = (ticks[1] < ticks[2]) ? (ticks[2] - ticks[1]) : (ticks[2] - ticks[1] + 0xFFFFFF);
//        WT3CCP0Frequency = SysCtlClockGet()*1.0f/(period[0] + period[1]);
//        WT3CCP0Duty = period[0] * 1.0f / (period[0] + period[1]);
//        WT3CCP0Completed = true;
//        tickCount = 0;
//
//        UARTprintf("Frequency: %d.%d\n", (uint32_t)WT3CCP0Frequency,(uint32_t)(100*(WT3CCP0Frequency-(uint32_t)WT3CCP0Frequency)));
//        WT3CCP0Duty *= 100;
//        UARTprintf("Duty: %d.%d%%\n", (uint32_t)WT3CCP0Duty,(uint32_t)(100*(WT3CCP0Duty-(uint32_t)WT3CCP0Duty)));
//        break;
//    }
//
//    //不计算占空比
////    if(tickCount > 1)
////    {
////        tickCount = 0;
////
////        uint32_t period;
////        if(ticks[0] < ticks[1])
////        {
////            period = ticks[1] - ticks[0];
////        }
////        else
////        {
////            period = ticks[1] - ticks[0] + 0xFFFFFF;
////        }
////        WT2CCP0Frequency = SysCtlClockGet()*1.0f/period;
////        WT2CCP0Completed = true;
////    }
//}

//红外线接收中断

//void WTimer3Capture0IntHandler_2()
//{
//    intStatus = TimerIntStatus(WTIMER3_BASE, TIMER_CAPA_EVENT);
//    TimerIntClear(WTIMER3_BASE, intStatus);
//
////    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
//
//    static uint32_t ticks[2] = { 0.0 };
//    static uint32_t tickCount = 0;
//    static bool leaderCodeFlag = false;
//    static uint32_t bits = 0;
//    static uint32_t WT3CCP0HighTime = 0;
//    static uint32_t data = 0;
//    uint32_t period;
//    uint8_t currentBit;
//
//    switch(tickCount)
//    {
//    case 0:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //第一次捕获，改为下降沿
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        break;
//    case 1:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //第二次捕获，改为上升沿
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        period = (ticks[0] < ticks[1]) ? (ticks[1] - ticks[0]) : (ticks[1] - ticks[0] + 0xFFFFFF);
//        WT3CCP0HighTime = period / 40;
////        UARTprintf("High Time: %d.%d us\n", (uint32_t)WT2CCP0HighTime,(uint32_t)(100*(WT2CCP0HighTime-(uint32_t)WT2CCP0HighTime)));
////        UARTprintf("High Time: %d us\n", WT2CCP0HighTime);
//        tickCount = 0;
//        break;
//    }
//    if(tickCount == 0)
//    {
//        if(WT3CCP0HighTime >= 4000 && WT3CCP0HighTime < 5000)    //4ms-5ms
//        {
//            data = 0;
//            bits = 0;
//            leaderCodeFlag = true;
//            return;
//        }
//        else if(WT3CCP0HighTime >= 1200 && WT3CCP0HighTime < 3000)    //1.2ms-1.8ms
//            currentBit = 1;
//        else if(WT3CCP0HighTime >= 200 && WT3CCP0HighTime < 1000)     //0.2ms-1.0ms
//            currentBit = 0;
//        else
//        {
//            data = 0;
//            bits = 0;
//            leaderCodeFlag = false;
//            return;
//        }
//        if(leaderCodeFlag)
//        {
//            bits++;
//            data <<= 1;
//            data |= currentBit;
//        }
//        if(bits==32)
//        {
//            UARTprintf("bits: %d\ndata: %0X\n",bits,data);
//            MotorCommandQuery(data);
//        }
//    }
//}
//
//void WTimer3Capture0GetFrequency(uint32_t times, float* freq, float* duty)
//{
//    TimerEnable(WTIMER3_BASE, TIMER_A);
//
//    float sumofFrequency = 0;
//    float sumofDuty = 0;
//    uint32_t startTime;
//    uint32_t nowTime;
//    uint32_t diffTime;
//
//    for (int i = 0; i < times; ++i)
//    {
//        startTime = SysTickValueGet();
//        while(WT3CCP0Completed == false)    //等待测量
//        {
//            nowTime = SysTickValueGet();
//            //SysCtlDelay(SysCtlClockGet()*0.0001/3);     //0.01s
//            if(nowTime <= startTime)    //时钟是递减的
//                diffTime = startTime - nowTime;
//            else
//                diffTime = startTime - nowTime + 0xFFFFFF;
//            if(diffTime*1.0f / SysCtlClockGet() > 0.2f)  //超时
//            {
//                WT3CCP0Completed = false;
//                TimerDisable(WTIMER3_BASE, TIMER_A);
//                *freq = 0.0f;
//                *duty = 0.0f;
//                return;
//            }
//        }
//        sumofFrequency += WT3CCP0Frequency;
//        sumofDuty += WT3CCP0Duty;
//        WT3CCP0Completed = false;
//    }
//    TimerDisable(WTIMER3_BASE, TIMER_A);
//    *freq = sumofFrequency / times;
//    *duty = sumofDuty / times;
//    return;
//}
//
