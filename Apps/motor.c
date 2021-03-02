/*
 * motor.c
 *
 *  Created on: 2020年8月16日
 *      Author: 电脑
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
 *  左        电机分布          右
 */

/*
 * 电机前进后退控制初始化
 *         前进   后退
 * 控制左边  PA2  PA3
 * 控制右边  PA4  PA5
 */

//刹车时间
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

//电机调速PID开关  true表示启用PID;
bool PIDMotorSwitch = true;

void MotorGPIOInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

/*
 * 电机前进后退控制
 * isForward true表示前进                    false表示后退
 * isLeft    true表示控制左半边电机   false表示控制右半边电机
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
 * 通过PWM波占空比设置电机速度
 * motor 电机序号
 *        0  1
 * duty  占空比  0.5~1.0
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

//舵机方向控制
//输入范围 -1 ~ +1，负值表示左转，正值表示右转
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
    case 0x00FFA25D:    //1号键  启动
        motorStatus = run;
        break;
    case 0x00FF629D:    //2号键  停止
        motorStatus = stop;
        break;
    case 0x00FF22DD:    //4号键 刹车
        motorStatus = brake;
        break;
    case 0x00FF18E7:    //前进键
        motorStatus = commandGoForward;
        break;
    case 0x00FF4AB5:    //后退键
        motorStatus = commandGoBack;
        break;
    case 0x00FF10EF:    //左键
        motorStatus = commandTurnLeft;
        break;
    case 0x00FF5AA5:    //右键
        motorStatus = commandTurnRight;
        break;
    case 0x00FF38C7:    //舵机直行
        motorStatus = commandGoStraight;
        break;
    case 0x00FFE21D:    //3键  寻线
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
    case 0x0072756E:    //1号键  启动
        motorStatus = run;
        break;
    case 0x00737470:    //2号键  停止
        motorStatus = stop;
        break;
    case 0x00627B6B:    //2号键  停止
        motorStatus = brake;
        break;
    case 0x00676F66:    //前进键
        motorStatus = commandGoForward;
        break;
    case 0x00676F62:    //后退键
        motorStatus = commandGoBack;
        break;
    case 0x00746E6C:    //左键
        motorStatus = commandTurnLeft;
        break;
    case 0x00746E72:    //右键
        motorStatus = commandTurnRight;
        break;
    case 0x00746E30:    //舵机直行
        motorStatus = commandGoStraight;
        break;
    case 0x00736B6C:    //3键  寻线
        motorStatus = seekLine;
        break;
    }
}


//小车指令执行
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
    //刹车
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_4|GPIO_PIN_5);
    MotorSpeedSet(0, 0.95);
    MotorSpeedSet(1, 0.95);
    //PID功能关
    PIDMotorSwitch = false;
}
////反接制动
//void MotorSlopeBrake()
//{
//    if(J901_Output[0][1] < 0)  //车头朝上
//    {
//        //反接
//        MotorGoBack();
//        //制动
//        MotorSpeedSet(0, -motor_pwm_output[0]);
//    }
//    if(J901_Output[0][1] > 0)  //车头朝下
//    {
//        //反接
//        MotorGoForward();
//        //制动
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
//状态管理
void MotorStatusManage()
{
    static int32_t startTime = -1;
    static int32_t nowTime = -1;
    int32_t diffTime = 0;

    if(motorStatus == preservedStatus
            && motorStatus != commandGoForward && motorStatus != commandGoBack)    //指令没改变
    {
        return;
    }
//    //从刹车状态恢复
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
    case run:    //1号键  启动
        MotorRun();
        break;
    case stop:    //2号键  停止
        MotorStop();
        break;
    case brake:   //4号键 刹车
//        MotorBrake();
        break;
//    case slopeBrake:
//        MotorStop();
//        break;
    case commandGoForward:    //前进键
        MotorGoForward();
        break;
    case commandGoBack:    //后退键
        MotorGoBack();
        break;
    case commandTurnLeft:    //左键
        ServoTurnLeft();
        break;
    case commandTurnRight:    //右键
        ServoTurnRight();
        break;
    case commandGoStraight:   //舵机直行
        ServoGoStraight();
        break;
    case seekLine:

        break;
    case imageSeekLine:

            break;
    case obstacle:            //红外检测到障碍物
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
            //延迟时间到  800ms
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
        return; //跳过记录状态，下次还进入switch
    }
    //记录这次的状态,下次不会进入switch
    preservedStatus = motorStatus;
}

