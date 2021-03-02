/*
 * my_timer.c
 *
 *  Created on: 2020��8��12��
 *      Author: ����
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
//����ٶȷ��ţ�trueΪ����falseΪ��

int omega_sign[2] = {0,0};

float delay_s = 0;

//�����жϺ��Ϊfalse
bool delayFlag;

void TimerInterruptInit(float delay, bool isPeriodic)
{
    delay_s = delay;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    IntPrioritySet(INT_TIMER2A, 3);
    //ʹ��TIMER0
    if(isPeriodic==true)
    {
        //TimerConfigure(TIMER2_BASE, TIMER_CFG_A_PERIODIC);//�����Լ���ģʽ
        TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);//�����Լ���ģʽ
    }
    else
    {
        TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);//���μ���ģʽ
    }

    TimerLoadSet(TIMER2_BASE, TIMER_A,SysCtlClockGet() * delay - 1);//����Ƶ��10HZ
    IntEnable(INT_TIMER0A);//NVIC
    //ʹ��TIMER0A
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    //TIMEOUT��־λ�����ж�
    IntMasterEnable();
    TimerIntRegister(TIMER2_BASE,TIMER_A,Timer2AIntHandler);
    //master interrupt enable API for all interrupts
    TimerEnable(TIMER2_BASE, TIMER_A);
    //TIMER0A��ʼ������������ֵ����TimerLoadSet�������ж�
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

    if(intStatus & TIMER_CAPA_EVENT)        //��ߵ��������
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
    // ����Timer4ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
    // ����GPIO_M��Ϊ���岶׽��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
    GPIOPinConfigure(GPIO_PC5_WT0CCP1);
    GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_5);
    // Ϊ�ܽ�����������ģʽ
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
    TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT_UP);
    // ʹ���Ͻ��ش���
    TimerPrescaleSet(WTIMER0_BASE, TIMER_B, 255);//Ԥ��Ƶ256
    TimerControlEvent(WTIMER0_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    // ���ü�����ΧΪ0~0x8FFF
    TimerLoadSet(WTIMER0_BASE, TIMER_B, 0xFFFFFF);
    // ������׽ģ��
    TimerEnable(WTIMER0_BASE, TIMER_B);
}

//PD3   �����ң�ع���һ����ʱ��
void WTimer3Capture1Init()
{
    // ����Timer4ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    // ����GPIO_M��Ϊ���岶׽��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
    GPIOPinConfigure(GPIO_PD3_WT3CCP1);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_3);
    // Ϊ�ܽ�����������ģʽ
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_COUNT_UP);
    // ʹ���Ͻ��ش���
    TimerPrescaleSet(WTIMER3_BASE, TIMER_B, 255);//Ԥ��Ƶ256
    TimerControlEvent(WTIMER3_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    // ���ü�����ΧΪ0~0x8FFF
    TimerLoadSet(WTIMER3_BASE, TIMER_B, 0xFFFFFF);
    // ������׽ģ��
//    TimerEnable(WTIMER3_BASE, TIMER_B);
    TimerEnable(WTIMER3_BASE, TIMER_BOTH);
}
//PB4 PB5
void Timer1Capture01Init()
{
    // ����Timer4ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    // ����GPIO_M��Ϊ���岶׽��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
    GPIOPinConfigure(GPIO_PB4_T1CCP0);
    GPIOPinConfigure(GPIO_PB5_T1CCP1);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5);
    // Ϊ�ܽ�����������ģʽ
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
    TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_TIME_UP);
    // ʹ���Ͻ��ش���
    TimerPrescaleSet(TIMER1_BASE, TIMER_A, 255);//Ԥ��Ƶ256
    TimerPrescaleSet(TIMER1_BASE, TIMER_B, 255);//Ԥ��Ƶ256
    TimerControlEvent(TIMER1_BASE, TIMER_BOTH, TIMER_EVENT_POS_EDGE);
    // ���ü�����ΧΪ0~0x8FFF
    TimerLoadSet(TIMER1_BASE, TIMER_A, 0xFFF);
    TimerLoadSet(TIMER1_BASE, TIMER_B, 0xFFF);
    // ע���жϴ���������Ӧ�����¼�
    TimerIntRegister(TIMER1_BASE, TIMER_A, T1CCP01IntHandler);
    TimerIntRegister(TIMER1_BASE, TIMER_B, T1CCP01IntHandler);
    // ϵͳ���жϿ�
    IntMasterEnable();
    // ʱ���ж������ж��¼�ΪCaptureģʽ�б��ش���
    TimerIntEnable(TIMER1_BASE, TIMER_CAPA_EVENT|TIMER_CAPB_EVENT);
    // NVIC������ʱ��Aģ���ж�
    IntEnable(INT_TIMER1A);
    IntEnable(INT_TIMER1B);
    // ������׽ģ��
    TimerEnable(TIMER1_BASE, TIMER_BOTH);
}



//PD6
//void WTimer5Capture0Init()
//{
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
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP);
//    // ʹ���Ͻ��ش���
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 255);//Ԥ��Ƶ256
//    TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
//    // ���ü�����ΧΪ0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_A, 0xFFFFFF);
//    // ������׽ģ��
//    TimerEnable(WTIMER5_BASE, TIMER_A);
//}
////PD7
//void WTimer5Capture1Init()
//{
//    // ����Timer4ģ��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);
//    // ����GPIO_M��Ϊ���岶׽��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
//    GPIOPinConfigure(GPIO_PD7_WT5CCP1);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_7);
//    // Ϊ�ܽ�����������ģʽ
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
//    TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_COUNT_UP);
//    // ʹ���Ͻ��ش���
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_B, 255);//Ԥ��Ƶ256
//    TimerControlEvent(WTIMER5_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
//    // ���ü�����ΧΪ0~0x8FFF
//    TimerLoadSet(WTIMER5_BASE, TIMER_B, 0xFFFFFF);
//    // ������׽ģ��
//    TimerEnable(WTIMER5_BASE, TIMER_B);
//}


//PWM���벶�� PD2

//void WTimer3Capture0Init()
//{
//    // ����Timer4ģ��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
//    // ����GPIO_M��Ϊ���岶׽��
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//    // ����GPIO��Ϊʹ��Timer4��׽ģʽ
//    GPIOPinConfigure(GPIO_PD2_WT3CCP0);
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_2);
//    // Ϊ�ܽ�����������ģʽ
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    // ����ʹ��Timer4��TimerAģ��Ϊ�ش����Ӽ�ʱģʽ
//    TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
//    // ʹ���Ͻ��ش���
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 255);//Ԥ��Ƶ256
//    TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
//    // ���ü�����ΧΪ0~0x8FFF
//    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFF);
//    // ע���жϴ���������Ӧ�����¼�
//    TimerIntRegister(WTIMER3_BASE, TIMER_A, WTimer3Capture0IntHandler_2);
//    // ϵͳ���жϿ�
//    IntMasterEnable();
//    // ʱ���ж������ж��¼�ΪCaptureģʽ�б��ش���
//    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
//    // NVIC������ʱ��Aģ���ж�
//    IntEnable(INT_WTIMER3A);
//    // ������׽ģ��
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
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //��һ�β��񣬸�Ϊ�½���
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        break;
//    case 1:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //�ڶ��β��񣬸�Ϊ������
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
//    //������ռ�ձ�
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

//�����߽����ж�

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
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);    //��һ�β��񣬸�Ϊ�½���
//        ticks[tickCount++] = TimerValueGet(WTIMER3_BASE, TIMER_A);
//        break;
//    case 1:
//        TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    //�ڶ��β��񣬸�Ϊ������
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
//        while(WT3CCP0Completed == false)    //�ȴ�����
//        {
//            nowTime = SysTickValueGet();
//            //SysCtlDelay(SysCtlClockGet()*0.0001/3);     //0.01s
//            if(nowTime <= startTime)    //ʱ���ǵݼ���
//                diffTime = startTime - nowTime;
//            else
//                diffTime = startTime - nowTime + 0xFFFFFF;
//            if(diffTime*1.0f / SysCtlClockGet() > 0.2f)  //��ʱ
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
