/*
 * motor.c
 *
 *  Created on: 2020��8��16��
 *      Author: ����
 */

#include "Apps/encoder.h"
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
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_pwm.h"
#include "utils/uartstdio.h"

#include "Apps/motor_timer.h"
#include "Apps/motor.h"
#include "Apps/MyJ901.h"
#include "Apps/CarController.h"

/*
 *      0           1
 *  ��        ����ֲ�          ��
 */

/*
 * ���ǰ�����˿��Ƴ�ʼ��
 *         ǰ��   ����
 * �������  PA2  PA3
 * �����ұ�  PA4  PA5
 */

//ɲ��ʱ��
#define brakeTime 0.8

float defaultSpeed[2] = {4,4};
float seekLineSpeed[2] = {2,2};
float pitchSpeed[2] = {5,5};
float imageSeekLineSpeed[2] = {4,4};
float* nowSpeed = defaultSpeed;
bool preservedDirection[2] = {true,true};

float servoOffset = -0.05;
float nowServoValue = 0;

enum MotorStatus motorStatus = stop;
enum MotorStatus preservedStatus = stop;

float omega_set[2] = {0};
float omega_actual[2] = {0};

//�������PID����  true��ʾ����PID;
bool PIDMotorSwitch = true;

void MotorGPIOInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

/*
 * ���ǰ�����˿���
 * isForward true��ʾǰ��                    false��ʾ����
 * isLeft    true��ʾ�������ߵ��   false��ʾ�����Ұ�ߵ��
 */
void MotorDirectionSet(uint32_t oneSide, bool isForward)
{
    if(oneSide == LEFT)
    {
        if(isForward)
        {
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
        }
        else
        {
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
        }
    }
    else
    {
        if(isForward)
        {
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_4);
        }
        else
        {
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_5);
        }
    }
}

/*
 * ͨ��PWM��ռ�ձ����õ���ٶ�
 * motor ������
 *        0  1
 * duty  ռ�ձ�  0.5~1.0
 */
void MotorSpeedSet(int motor, float duty)
{
    switch(motor)
    {
    case 0:
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, PWMGenPeriodGet(PWM1_BASE, PWM_GEN_1)*duty - 1);
        PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT, true);
        PWMGenEnable(PWM1_BASE, PWM_GEN_1);
        break;
    case 1:
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3, PWMGenPeriodGet(PWM1_BASE, PWM_GEN_1)*duty - 1);
        PWMOutputState(PWM1_BASE, PWM_OUT_3_BIT, true);
        PWMGenEnable(PWM1_BASE, PWM_GEN_1);
        break;
    }
}

//����������
//���뷶Χ -1 ~ +1����ֵ��ʾ��ת����ֵ��ʾ��ת
void ServoDirectionSet(float direction)
{
    if(direction > 1)
        direction = 1;
    else if(direction < -1)
        direction = -1;

    direction += servoOffset;
    float duty = 0.2f*direction + 0.6f;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_3)*duty - 1);
    PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}

void MotorCommandQuery(uint32_t infraredCommand)
{
    switch(infraredCommand)
    {
    case 0x00FFA25D:    //1�ż�  ����
        motorStatus = run;
        break;
    case 0x00FF629D:    //2�ż�  ֹͣ
        motorStatus = stop;
        break;
    case 0x00FF22DD:    //4�ż� ɲ��
        motorStatus = brake;
        break;
    case 0x00FF18E7:    //ǰ����
        motorStatus = commandGoForward;
        break;
    case 0x00FF4AB5:    //���˼�
        motorStatus = commandGoBack;
        break;
    case 0x00FF10EF:    //���
        motorStatus = commandTurnLeft;
        break;
    case 0x00FF5AA5:    //�Ҽ�
        motorStatus = commandTurnRight;
        break;
    case 0x00FF38C7:    //���ֱ��
        motorStatus = commandGoStraight;
        break;
    case 0x00FFE21D:    //3��  Ѱ��
        motorStatus = seekLine;
        nowSpeed = seekLineSpeed;
        break;
    }
    if(motorStatus != seekLine)
        nowSpeed = defaultSpeed;
}
void MotorBlueToothQuery(uint32_t blueToothCommand)
{
    switch(blueToothCommand)
    {
    case 0x0072756E:    //1�ż�  ����
        motorStatus = run;
        break;
    case 0x00737470:    //2�ż�  ֹͣ
        motorStatus = stop;
        break;
    case 0x00627B6B:    //2�ż�  ֹͣ
        motorStatus = brake;
        break;
    case 0x00676F66:    //ǰ����
        motorStatus = commandGoForward;
        break;
    case 0x00676F62:    //���˼�
        motorStatus = commandGoBack;
        break;
    case 0x00746E6C:    //���
        motorStatus = commandTurnLeft;
        break;
    case 0x00746E72:    //�Ҽ�
        motorStatus = commandTurnRight;
        break;
    case 0x00746E30:    //���ֱ��
        motorStatus = commandGoStraight;
        break;
    case 0x00736B6C:    //3��  Ѱ��
        motorStatus = seekLine;
        break;
    }
}


//С��ָ��ִ��
void MotorStop()
{
    omega_set[0] = 0;
    omega_set[1] = 0;
}
void MotorRun()
{
    if(PIDDistanceSwitch == false)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1];
    }
}
void MotorBrake()
{
    //ɲ��
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_4|GPIO_PIN_5);
    MotorSpeedSet(0, 0.95);
    MotorSpeedSet(1, 0.95);
    //PID���ܹ�
    PIDMotorSwitch = false;
}
////�����ƶ�
//void MotorSlopeBrake()
//{
//    if(J901_Output[0][1] < 0)  //��ͷ����
//    {
//        //����
//        MotorGoBack();
//        //�ƶ�
//        MotorSpeedSet(0, -motor_pwm_output[0]);
//    }
//    if(J901_Output[0][1] > 0)  //��ͷ����
//    {
//        //����
//        MotorGoForward();
//        //�ƶ�
//        MotorSpeedSet(1, -motor_pwm_output[1]);
//    }
//}
void MotorGoForward()
{
//    MotorDirectionSet(LEFT, true);
//    preservedDirection[0] = true;
//    MotorDirectionSet(RIGHT, true);
//    preservedDirection[1] = true;
    omega_set[0] = nowSpeed[0];
    omega_set[1] = nowSpeed[1];
}
void MotorGoBack()
{
//    MotorDirectionSet(LEFT, false);
//    preservedDirection[0] = false;
//    MotorDirectionSet(RIGHT, false);
//    preservedDirection[1] = false;
    omega_set[0] = -nowSpeed[0];
    omega_set[1] = -nowSpeed[1];
}
void ServoTurnLeft()
{
    nowServoValue = -1.0f;
    ServoDirectionSet(nowServoValue);
}
void ServoTurnRight()
{
    nowServoValue = 1.0f;
    ServoDirectionSet(nowServoValue);
}
void ServoGoStraight()
{
    nowServoValue = 0.0f;
    ServoDirectionSet(nowServoValue);
}
//״̬����
void MotorStatusManage()
{
    static int32_t startTime = -1;
    static int32_t nowTime = -1;
    int32_t diffTime = 0;

    if(motorStatus == preservedStatus
            && motorStatus != commandGoForward && motorStatus != commandGoBack)    //ָ��û�ı�
    {
        return;
    }
//    //��ɲ��״̬�ָ�
//    if(preservedStatus == brake)
//    {
//        PIDMotorSwitch = true;
//        MotorDirectionSet(LEFT, preservedDirection[0]);
//        MotorDirectionSet(RIGHT, preservedDirection[1]);
//    }
    if(preservedStatus == seekLine || preservedStatus == imageSeekLine)
    {
        ServoDirectionSet(0);
    }

    switch(motorStatus)
    {
    case run:    //1�ż�  ����
        MotorRun();
        break;
    case stop:    //2�ż�  ֹͣ
        MotorStop();
        break;
    case brake:   //4�ż� ɲ��
//        MotorBrake();
        break;
//    case slopeBrake:
//        MotorStop();
//        break;
    case commandGoForward:    //ǰ����
        MotorGoForward();
        break;
    case commandGoBack:    //���˼�
        MotorGoBack();
        break;
    case commandTurnLeft:    //���
        ServoTurnLeft();
        break;
    case commandTurnRight:    //�Ҽ�
        ServoTurnRight();
        break;
    case commandGoStraight:   //���ֱ��
        ServoGoStraight();
        break;
    case seekLine:

        break;
    case imageSeekLine:

            break;
    case obstacle:            //�����⵽�ϰ���
        if(startTime == -1)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
            startTime = SysTickTime;

            ServoTurnRight();
            PIDMotorSwitch = true;
        }
        else
        {
            nowTime = SysTickTime;
            diffTime = nowTime - startTime;
            //�ӳ�ʱ�䵽  800ms
            if(diffTime * 10 > 800)
            {
                MotorBrake();
                SysCtlDelay(SysCtlClockGet()*brakeTime/3);     //0.01s

                ServoGoStraight();
                MotorDirectionSet(LEFT, preservedDirection[0]);
                MotorDirectionSet(RIGHT, preservedDirection[1]);
                startTime = -1;
                preservedStatus = brake;
                motorStatus = run;
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0);
            }
        }
        return; //������¼״̬���´λ�����switch
    }
    //��¼��ε�״̬,�´β������switch
    preservedStatus = motorStatus;
}

