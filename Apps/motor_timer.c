/*
 * motor_timer.c
 *
 *  Created on: 2020年8月17日
 *      Author: 电脑
 */
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
#include "Apps/motor_timer.h"
#include "Apps/led_pwm.h"
#include "Apps/motor.h"
#include "Apps/CarController.h"

uint32_t SysTickTime = 0;

void SysTickInit()
{
    SysTickPeriodSet(SysCtlClockGet() * 0.01 - 1);   //0.001s
    SysTickIntRegister(SysTickIntHandler);
    SysTickIntEnable();
    SysTickEnable();

}
void SysTickIntHandler()
{
    SysTickTime++;
}

//红外遥控初始化  PD2  与编码器捕获共用一个定时器
void InfraredInit()
{
    // 启用Timer4模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    // 启用GPIO_M作为脉冲捕捉脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // 配置GPIO脚为使用Timer4捕捉模式
    GPIOPinConfigure(GPIO_PD2_WT3CCP0);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_2);
    // 为管脚配置弱上拉模式
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 配置使用Timer4的TimerA模块为沿触发加计时模式
    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_COUNT_UP);
    // 使用上降沿触发
    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 255);//预分频256
    TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    // 设置计数范围为0~0x8FFF
    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFF);
    // 注册中断处理函数以响应触发事件
    TimerIntRegister(WTIMER3_BASE, TIMER_A, InfraredIntHandler);
    // 系统总中断开
    IntMasterEnable();
    // 时钟中断允许，中断事件为Capture模式中边沿触发
    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
    // NVIC中允许定时器A模块中断
    IntEnable(INT_WTIMER3A);
    // 启动捕捉模块
//    TimerEnable(WTIMER3_BASE, TIMER_A);
    TimerEnable(WTIMER3_BASE, TIMER_BOTH);
}

//红外线接收中断
uint32_t intStatus;

void InfraredIntHandler()
{
    intStatus = TimerIntStatus(WTIMER3_BASE, TIMER_CAPA_EVENT);
    TimerIntClear(WTIMER3_BASE, intStatus);

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);

    static uint32_t ticks[2] = { 0.0 };
    static uint32_t tickCount = 0;
    static bool leaderCodeFlag = false;
    static uint32_t bits = 0;
    static uint32_t WT3CCP0HighTime = 0;
    static uint32_t data = 0;
    uint32_t period;
    uint8_t currentBit;

    switch(tickCount)
    {
    case 0:
        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //第一次捕获，改为下降沿
        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
        break;
    case 1:
        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //第二次捕获，改为上升沿
        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
        period = (ticks[0] < ticks[1]) ? (ticks[1] - ticks[0]) : (ticks[1] - ticks[0] + 0xFFFFFF);
        WT3CCP0HighTime = period / 40;
//        UARTprintf("High Time: %d.%d us\n", (uint32_t)WT2CCP0HighTime,(uint32_t)(100*(WT2CCP0HighTime-(uint32_t)WT2CCP0HighTime)));
//        UARTprintf("High Time: %d us\n", WT2CCP0HighTime);
        tickCount = 0;
        break;
    }
    if(tickCount == 0)
    {
        if(WT3CCP0HighTime >= 4000 && WT3CCP0HighTime < 5000)    //4ms-5ms
        {
            data = 0;
            bits = 0;
            leaderCodeFlag = true;
            return;
        }
        else if(WT3CCP0HighTime >= 1200 && WT3CCP0HighTime < 3000)    //1.2ms-1.8ms
            currentBit = 1;
        else if(WT3CCP0HighTime >= 200 && WT3CCP0HighTime < 1000)     //0.2ms-1.0ms
            currentBit = 0;
        else
        {
            data = 0;
            bits = 0;
            leaderCodeFlag = false;
            return;
        }
        if(leaderCodeFlag)
        {
            bits++;
            data <<= 1;
            data |= currentBit;
        }
        if(bits==32)
        {
            UARTprintf("bits: %d\ndata: %0X\n",bits,data);
//            MotorCommandQuery(data);
            CarCommandQuery(data);
        }
    }
}

/*
 * 电机速度控制初始化
 * PWM占空比调节   频率 1kHz   占空比 0.4~1.0
 *          PA6    PA7
 * 电机序号    0      1
 */

void MotorPWMInit(float freq, float duty)
{
    //设置PWM时钟为系统时钟的1分频
       SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
       //配置引脚为PWM功能
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
       GPIOPinConfigure(GPIO_PA6_M1PWM2);
       GPIOPinConfigure(GPIO_PA7_M1PWM3);

       GPIOPinTypePWM(GPIO_PORTA_BASE,GPIO_PIN_6);
       GPIOPinTypePWM(GPIO_PORTA_BASE,GPIO_PIN_7);
       //配置发生器
       PWMGenConfigure(PWM1_BASE, PWM_GEN_1,
                       PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
       //配置周期
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, SysCtlClockGet() / freq - 1);
       //配置占空比
       uint32_t pwmGenPeriod=PWMGenPeriodGet(PWM1_BASE, PWM_GEN_1);
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,
                        pwmGenPeriod * duty - 1);
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,
                        pwmGenPeriod * duty - 1);
       //使能 PWM1 输出
       PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT, true);
       PWMOutputState(PWM1_BASE, PWM_OUT_3_BIT, true);
       //使能 PWM1 发生器模块
       PWMGenEnable(PWM1_BASE, PWM_GEN_1);
}
// PC4 PWM
void ServoInit(float freq,float duty)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PC4_M0PWM6);
    GPIOPinTypePWM(GPIO_PORTC_BASE,GPIO_PIN_4);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_3,
                  PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, SysCtlClockGet() / freq);
//    uint32_t pwmGenPeriod = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_3);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,
                     SysCtlClockGet() / freq * duty);
    PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}

////PWM捕获  PD6
//void WTimer5Capture0Init()
//{
//    IntPrioritySet(INT_WTIMER5A, 0);
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
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
//    // 使用上降沿触发
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 255);//预分频256
//    TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
//    // 设置计数范围为0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_A, 0xFFFFFF);
//    // 注册中断处理函数以响应触发事件
//    TimerIntRegister(WTIMER5_BASE, TIMER_A, WTimer5Capture0IntHandler);
//
//    // 系统总中断开
//    IntMasterEnable();
//    // 时钟中断允许，中断事件为Capture模式中边沿触发
//    TimerIntEnable(WTIMER5_BASE, TIMER_CAPA_EVENT);
//    // NVIC中允许定时器A模块中断
//    IntEnable(INT_WTIMER5A);
//    // 启动捕捉模块
////    TimerEnable(WTIMER5_BASE, TIMER_A);
//}
//
//float WT5CCP0Frequency;
//float WT5CCP0Duty;
//bool WT5CCP0Completed = false;
//
//void WTimer5Capture0IntHandler()
//{
//    uint32_t intStatus = TimerIntStatus(WTIMER5_BASE, TIMER_CAPA_EVENT);
//    TimerIntClear(WTIMER5_BASE, intStatus);
//
//    static uint32_t ticks[2] = { 0.0 };
//    static uint32_t tickCount = 0;
//    uint32_t period;
//
//    ticks[tickCount++] = TimerValueGet(WTIMER5_BASE, TIMER_A);
////    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
//
//    if(tickCount > 1)
//    {
//        tickCount = 0;
//
//        if(ticks[0] < ticks[1])
//        {
//            period = ticks[1] - ticks[0];
//        }
//        else
//        {
//            period = ticks[1] - ticks[0] + 0xFFFFFF;
//        }
//        WT5CCP0Frequency = SysCtlClockGet()*1.0f/period;
//        WT5CCP0Completed = true;
////        UARTprintf("WT5CCP0 Frequency: %d.%d\n", (uint32_t)WT5CCP0Frequency,(uint32_t)(100*(WT5CCP0Frequency-(uint32_t)WT5CCP0Frequency)));
//    }
//}
//
//float WTimer5Capture0GetFrequency(uint32_t times)
//{
//    TimerEnable(WTIMER5_BASE, TIMER_A);
//
//    float sumofFrequency = 0;
//
//    uint32_t startTime;
//    uint32_t nowTime;
//    uint32_t diffTime;
//
//    for (int i = 0; i < times; ++i)
//    {
//        startTime = SysTickValueGet();
//        while(WT5CCP0Completed == false)    //等待测量
//        {
//            nowTime = SysTickValueGet();
//            //SysCtlDelay(SysCtlClockGet()*0.0001/3);     //0.01s
//            if(nowTime <= startTime)    //时钟是递减的
//                diffTime = startTime - nowTime;
//            else
//                diffTime = startTime - nowTime + 0xFFFFFF;
//            if(diffTime*1.0f / SysCtlClockGet() > 0.002f)  //超时
//            {
//                WT5CCP0Completed = false;
//                TimerDisable(WTIMER5_BASE, TIMER_A);
//                return 0.0f;
//            }
//        }
//        sumofFrequency += WT5CCP0Frequency;
//        WT5CCP0Completed = false;
//    }
//    TimerDisable(WTIMER5_BASE, TIMER_A);
//    return sumofFrequency / times;
//}



