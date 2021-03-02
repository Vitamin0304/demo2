/*
 * my_gpio.c
 *
 *  Created on: 2020��8��12��
 *      Author: ����
 */
#include "Apps/my_gpio.h"

void GPIOKey1Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//������io��������Ϊ�ߵ�ƽ�����ƽ��Ϊ�͵�ƽ����Ϊ�䰴��

    GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_LOW_LEVEL);
    GPIOIntEnable(GPIO_PORTF_BASE,GPIO_PIN_4);
    //�ж�ע�ᣬ�ڶ����������жϷ��������
    GPIOIntRegister(GPIO_PORTF_BASE,IntHandler_GPIOF);
    //Port�ж�ʹ��
    IntEnable(INT_GPIOF);
    //�������ж�
    IntMasterEnable();
}

void GPIOLEDInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2|GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
}

void IntHandler_GPIOF(void)//�жϷ�����
{
    uint32_t intStatus = GPIOIntStatus(GPIO_PORTF_BASE, true);//��ȡ״̬
    float duties[4] = {0.4,0.6,0.8,1.0};

    static uint32_t times = 0;
    if((intStatus & GPIO_PIN_4) == GPIO_PIN_4)//��������ж����ĸ�io�ڵ��ж�
    {
        SysCtlDelay(SysCtlClockGet()*0.03/3);     //0.03s
        //��������
        int32_t key1Status = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
        if((key1Status & GPIO_PIN_4) != GPIO_PIN_4)
        {
            //GPIOģʽ
//            switch(times%3)
//            {
//            case 0:
//                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
//                break;
//            case 1:
//                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
//                break;
//            case 2:
//                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
//                break;
//            }
            //PWMģʽ

            switch(times%4)
            {
            case 0:
                M1PWM567SetDuty(duties[0], PWM_GEN_2, PWM_OUT_5, PWM_OUT_5_BIT);
                break;
            case 1:
                M1PWM567SetDuty(duties[1], PWM_GEN_2, PWM_OUT_5, PWM_OUT_5_BIT);
                break;
            case 2:
                M1PWM567SetDuty(duties[2], PWM_GEN_2, PWM_OUT_5, PWM_OUT_5_BIT);
                break;
            case 3:
                M1PWM567SetDuty(duties[3], PWM_GEN_2, PWM_OUT_5, PWM_OUT_5_BIT);
                break;
            }
            times++;
        }
        while(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
        {
            ;
        }
        GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);//���ָ�����ж�Դ
    }
}

/*
 * �����߶�ȡ
 */
void GPIOPE3Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//������io������Ϊ��ƽ�����ƽ��Ϊ�͵�ƽ����Ϊ�䰴��

    GPIOIntTypeSet(GPIO_PORTE_BASE,GPIO_PIN_3, GPIO_RISING_EDGE);
    GPIOIntEnable(GPIO_PORTE_BASE,GPIO_PIN_3);
    //�ж�ע�ᣬ�ڶ����������жϷ��������
    GPIOIntRegister(GPIO_PORTE_BASE,IntHandler_GPIOE);
    //Port�ж�ʹ��
    IntEnable(INT_GPIOE);
    //�������ж�
    IntMasterEnable();
}

uint32_t GetPE3HighTime(void)
{
    uint32_t t = 0;
    while((GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3) & GPIO_PIN_3) == GPIO_PIN_3)
    {
        SysCtlDelay(SysCtlClockGet()*0.00001/3);  //10us
        t += 10;
        if(t>=500)
            return t;
    }
    return t;
}


bool leaderCodeFlag = false;
uint32_t frameData = 0;

void IntHandler_GPIOE()
{

    uint32_t intStatus = GPIOIntStatus(GPIO_PORTE_BASE, true);//��ȡ״̬

    if((intStatus & GPIO_PIN_3) == GPIO_PIN_3)//��������ж����ĸ�io�ڵ��ж�
    {

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

//            uint32_t times = 0;
        int32_t PE3Status = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3);
        if((PE3Status & GPIO_PIN_3) == GPIO_PIN_3)     //�ߵ�ƽ
        {
            uint32_t pulseTime = GetPE3HighTime();
            uint8_t currentBit;
            if(pulseTime >= 5000)
                goto out;
            else if(pulseTime >= 4000)    //4ms-5ms
                leaderCodeFlag = true;
            else if(pulseTime >= 1200 && pulseTime < 1800)    //1.2ms-1.8ms
                currentBit = 1;
            else if(pulseTime >= 200 && pulseTime < 1000)     //0.2ms-1.0ms
                currentBit = 0;
            else
                goto out;
            if(leaderCodeFlag)
            {
//                    times++;
                frameData <<=1;
                frameData |= currentBit;
            }

        }
        out:
        UARTprintf("%d", frameData);
        leaderCodeFlag = false;
        frameData = 0;

        GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);//���ָ�����ж�Դ
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0);
    }
}
