/*
 * motor_timer.c
 *
 *  Created on: 2020��8��17��
 *      Author: ����
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

//����ң�س�ʼ��  PD2  �������������һ����ʱ��
void InfraredInit()
{
    // ����Timer4ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    // ����GPIO_M��Ϊ���岶׽��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
    GPIOPinConfigure(GPIO_PD2_WT3CCP0);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_2);
    // Ϊ�ܽ�����������ģʽ
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_COUNT_UP);
    // ʹ���Ͻ��ش���
    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 255);//Ԥ��Ƶ256
    TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    // ���ü�����ΧΪ0~0x8FFF
    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFF);
    // ע���жϴ���������Ӧ�����¼�
    TimerIntRegister(WTIMER3_BASE, TIMER_A, InfraredIntHandler);
    // ϵͳ���жϿ�
    IntMasterEnable();
    // ʱ���ж������ж��¼�ΪCaptureģʽ�б��ش���
    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
    // NVIC������ʱ��Aģ���ж�
    IntEnable(INT_WTIMER3A);
    // ������׽ģ��
//    TimerEnable(WTIMER3_BASE, TIMER_A);
    TimerEnable(WTIMER3_BASE, TIMER_BOTH);
}

//�����߽����ж�
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
        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //��һ�β��񣬸�Ϊ�½���
        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
        break;
    case 1:
        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //�ڶ��β��񣬸�Ϊ������
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
 * ����ٶȿ��Ƴ�ʼ��
 * PWMռ�ձȵ���   Ƶ�� 1kHz   ռ�ձ� 0.4~1.0
 *          PA6    PA7
 * ������    0      1
 */

void MotorPWMInit(float freq, float duty)
{
    //����PWMʱ��Ϊϵͳʱ�ӵ�1��Ƶ
       SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
       //��������ΪPWM����
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
       GPIOPinConfigure(GPIO_PA6_M1PWM2);
       GPIOPinConfigure(GPIO_PA7_M1PWM3);

       GPIOPinTypePWM(GPIO_PORTA_BASE,GPIO_PIN_6);
       GPIOPinTypePWM(GPIO_PORTA_BASE,GPIO_PIN_7);
       //���÷�����
       PWMGenConfigure(PWM1_BASE, PWM_GEN_1,
                       PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
       //��������
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, SysCtlClockGet() / freq - 1);
       //����ռ�ձ�
       uint32_t pwmGenPeriod=PWMGenPeriodGet(PWM1_BASE, PWM_GEN_1);
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,
                        pwmGenPeriod * duty - 1);
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,
                        pwmGenPeriod * duty - 1);
       //ʹ�� PWM1 ���
       PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT, true);
       PWMOutputState(PWM1_BASE, PWM_OUT_3_BIT, true);
       //ʹ�� PWM1 ������ģ��
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

////PWM����  PD6
//void WTimer5Capture0Init()
//{
//    IntPrioritySet(INT_WTIMER5A, 0);
//    // ����Timer4ģ��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);
//    // ����GPIO_M��Ϊ���岶׽��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
//    GPIOPinConfigure(GPIO_PD6_WT5CCP0);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_6);
//    // Ϊ�ܽ�����������ģʽ
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
//    // ʹ���Ͻ��ش���
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 255);//Ԥ��Ƶ256
//    TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
//    // ���ü�����ΧΪ0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_A, 0xFFFFFF);
//    // ע���жϴ���������Ӧ�����¼�
//    TimerIntRegister(WTIMER5_BASE, TIMER_A, WTimer5Capture0IntHandler);
//
//    // ϵͳ���жϿ�
//    IntMasterEnable();
//    // ʱ���ж������ж��¼�ΪCaptureģʽ�б��ش���
//    TimerIntEnable(WTIMER5_BASE, TIMER_CAPA_EVENT);
//    // NVIC������ʱ��Aģ���ж�
//    IntEnable(INT_WTIMER5A);
//    // ������׽ģ��
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
//        while(WT5CCP0Completed == false)    //�ȴ�����
//        {
//            nowTime = SysTickValueGet();
//            //SysCtlDelay(SysCtlClockGet()*0.0001/3);     //0.01s
//            if(nowTime <= startTime)    //ʱ���ǵݼ���
//                diffTime = startTime - nowTime;
//            else
//                diffTime = startTime - nowTime + 0xFFFFFF;
//            if(diffTime*1.0f / SysCtlClockGet() > 0.002f)  //��ʱ
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



