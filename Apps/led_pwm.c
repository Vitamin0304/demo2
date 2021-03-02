
#include "Apps/led_pwm.h"

void M1PWM567Init(uint32_t freq, float duty, bool state[3])
{
    //����PWMʱ��Ϊϵͳʱ�ӵ�1��Ƶ
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    //��������ΪPWM����
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_1);
    GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_2);
    GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_3);
    //���� PWM1 Generator3��������
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    //���� PWM1 Generator3 ����
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, SysCtlClockGet() / freq - 1);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, SysCtlClockGet() / freq - 1);
    //���� PWM1 Generator3 ռ�ձ�
    uint32_t pwmGenPeriod=PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,
                     pwmGenPeriod * duty - 1);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,
                     pwmGenPeriod * duty - 1);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,
                     pwmGenPeriod * duty - 1);
    //ʹ�� PWM1 ���
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, state[0]);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, state[1]);
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, state[2]);
    //ʹ�� PWM1 ������ģ��
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
}

void M1PWM567SetDuty(float duty, uint32_t ui32Gen, uint32_t ui32PWMOut, uint32_t ui32PWMOutBits)
{
    //���� PWM1 Generator3 ռ�ձ�
    PWMPulseWidthSet(PWM1_BASE, ui32PWMOut, PWMGenPeriodGet(PWM1_BASE, ui32Gen)*duty - 1);
    PWMOutputState(PWM1_BASE, ui32PWMOutBits, true);
    //ʹ�� PWM1 ��������ģ��
    PWMGenEnable(PWM1_BASE, ui32Gen);
}
