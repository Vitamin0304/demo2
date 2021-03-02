#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "driverlib/gpio.h"
#include "CarController.h"
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
#include "Apps/motor.h"
#include "Apps/motor_timer.h"
#include "Apps/encoder.h"
#include "Apps/MyJ901.h"
#include "Apps/bluetooth.h"
#include "OLED/MyOLED.h"
#include "Apps/buzzer.h"

float distance_actual[2] = {0.0f};
float distance_set[2] = {-0.015f,-0.015f};
float omega_set_output[2];
bool PIDDistanceSwitch = false;
bool PIDDistanceRestartFlag = false;

float seek_line_set = 0.0f;
float seek_line_actual = 0.0f;
float seek_line_actual_int = 0.0f;
float seek_line_output = 0.0f;
bool seek_line_end = false;

//��0λ��ֱ�߽ؾ࣬��1λ��ֱ����б��
float image_seek_line_set[2] = {0};
float image_seek_line_actual[2] = {0};
float image_seek_line_actual_int[2] = {0};
float image_seek_line_output = 0;

uint8_t car_stop_flag = 0;//0Ϊ����ͷδ��⵽�յ�
bool task2_end_flag = false;
float task2_set_time = 10;
bool PIDIntegralRestartFlag = false;
float board_angle = 0;
float motorSpeedGain = 0;
bool PIDTask2EndFlag = false;

float pitch_set = 0;
float pitch_output = 0;
float pitch_step = 0;

float task1_step = 0;
float task2_step = 0;

int32_t startTime = -1;
int32_t startTime_2 = -1;
int32_t nowTime = -1;
int32_t diffTime = 0;

enum MotorStatus carStatus = stop;

/*
 * �����ʼ��������Ҫ������������
 */
void CarInit()
{
    MotorGPIOInit();
    MotorPWMInit(10000, 0);
    MotorDirectionSet(LEFT, true);
    MotorDirectionSet(RIGHT, true);
    ServoInit(400, 0.6);  //�����ʼ����0.4��45��    0.6ֱ��    0.8��45��
    InfraredInit();
    //������
    WTimer0Capture1Init();
    WTimer3Capture1Init();
    Timer1Capture01Init();
//    InfraredAvoidanceInit();
    SeekLineInit();
}

void CarCommandQuery(uint32_t infraredCommand)
{
    switch(infraredCommand)
    {
    case 0x00FFA25D:    //1�ż�  ����
        PIDDistanceSwitch = false;
//        startTime = SysTickTime;
//        CarSetDistance(0.1);
        carStatus = run;
        break;
    case 0x00FF629D:    //2�ż�  ֹͣ
        PIDDistanceSwitch = false;

//        diffTime = SysTickTime - startTime;
//        TimeUsed_Display(diffTime/100,diffTime%100,0);
        carStatus = stop;
        break;
    case 0x00FF22DD:    //4�ż� ɲ��
        CarBrake();
        carStatus = brake;
        break;
    case 0x00FF18E7:    //ǰ����
        carStatus = commandGoForward;
        break;
    case 0x00FF4AB5:    //���˼�
        carStatus = commandGoBack;
        break;
    case 0x00FF10EF:    //���
        carStatus = commandTurnLeft;
        break;
    case 0x00FF5AA5:    //�Ҽ�
        carStatus = commandTurnRight;
        break;
    case 0x00FF38C7:    //���ֱ��
        carStatus = commandGoStraight;
        break;
    case 0x00FFE21D:    //3��  Ѱ��
        carStatus = seekLine;
        nowSpeed = seekLineSpeed;
        break;
    case 0x00FF02FD:    //5��  ���ΰ�ƽ��
        carStatus = pitch;
        nowSpeed = pitchSpeed;
        break;
    case 0x00FFC23D:    //6��  ��������
        carStatus = task2;
        nowSpeed = imageSeekLineSpeed;


        imageSeekLineSpeed[0] = 0.01344*task2_set_time*task2_set_time-0.5816*task2_set_time+8.174;
        if(task2_set_time>=19)
            imageSeekLineSpeed[0]-=0.08;
        imageSeekLineSpeed[1] = imageSeekLineSpeed[0];

        PIDIntegralRestartFlag = true;


        PIDDistanceSwitch = false;
        car_stop_flag = 0;
        TimeUsed_Display(0,0,0);
        break;
    case 0x00FFE01F:    //7��  ͼ��Ѱ��
        carStatus = imageSeekLine;
        nowSpeed = seekLineSpeed;
        PIDDistanceSwitch = false;
        break;
    }
    if(carStatus != pitch && carStatus != seekLine && carStatus != task2 && carStatus != imageSeekLine)
        nowSpeed = defaultSpeed;

}
void CarBlueToothQuery(uint32_t blueToothCommand)
{
    switch(blueToothCommand)
    {
    case 0x0072756E:    //1�ż�  ����
        carStatus = run;
        break;
    case 0x00737470:    //2�ż�  ֹͣ
        carStatus = stop;
        break;
    case 0x00627B6B:    //2�ż�  ֹͣ
        carStatus = brake;
        break;
    case 0x00676F66:    //ǰ����
        carStatus = commandGoForward;
        break;
    case 0x00676F62:    //���˼�
        carStatus = commandGoBack;
        break;
    case 0x00746E6C:    //���
        carStatus = commandTurnLeft;
        break;
    case 0x00746E72:    //�Ҽ�
        carStatus = commandTurnRight;
        break;
    case 0x00746E30:    //���ֱ��
        carStatus = commandGoStraight;
        break;
    case 0x00736B6C:    //3��  Ѱ��
        carStatus = seekLine;
        break;
    }
}
void CarStatusManage()
{
    switch(carStatus)
    {
    case run:
//        PIDDistanceSwitch = true;
        motorStatus = run;
        break;
    case stop:
        motorStatus = stop;
        break;
    case brake:
        motorStatus = brake;
        break;
    case commandTurnLeft:    //���
        motorStatus = commandTurnLeft;
        break;
    case commandTurnRight:    //�Ҽ�
        motorStatus = commandTurnRight;
        break;
    case commandGoStraight:   //���ֱ��
        motorStatus = commandGoStraight;
        break;
    case commandGoForward:
//        if(CarGoForward() == FINAL_STEP)
//            carStatus = run;
        motorStatus = commandGoForward;
        break;
    case commandGoBack:
//        if(CarGoBack() == FINAL_STEP)
//            carStatus = run;
        motorStatus = commandGoBack;
        break;
    case obstacle:
        if(CarAvoidObstacle() == FINAL_STEP)
            carStatus = run;
        break;
    case seekLine:
        CarSeekLine_ZeroAngle(seek_line_output);
        motorStatus = seekLine;
        break;
    case pitch:
        pitch_step = CarPitch(pitch_output);
        if(pitch_set == FINAL_STEP)
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);
        break;
    case task2:
        task2_step = CarTask2_2();
        if(task2_step == FINAL_STEP)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
            carStatus = stop ;
        }
        break;
    case imageSeekLine:
        CarSeekLine(image_seek_line_output,0);
        motorStatus = imageSeekLine;
        break;
    }
}

int CarGoForward()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        if(preservedDirection[0] == false || preservedDirection[1] == false)
        {
            motorStatus = brake;
            step++;
        }
        else
        {
            preservedStatus = brake;
            motorStatus = run;
            step = 0;
            return FINAL_STEP;
        }
        break;
    case 1:   //�ȴ����ͣ��
        if(omega_actual[0] < 0.5 && omega_actual[1] < 0.5)
        {
            MotorStop();
            MotorGoForward();
            step++;
        }
        break;
    case 2:
        preservedStatus = brake;
        motorStatus = run;
        step = 0;
        return FINAL_STEP;
    }
    return step;
}
int CarGoBack()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        if(preservedDirection[0] == true || preservedDirection[1] == true)
        {
            motorStatus = brake;
            step++;
        }
        else
        {
            preservedStatus = brake;
            motorStatus = run;
            step = 0;
            return FINAL_STEP;
        }
        break;
    case 1:   //�ȴ����ͣ��
        if(omega_actual[0] < 0.5 && omega_actual[1] < 0.5)
        {
            MotorStop();
            MotorGoBack();
            step++;
        }
        break;
    case 2:
        preservedStatus = brake;
        motorStatus = run;
        step = 0;
        return FINAL_STEP;
    }
    return step;
}
int CarAvoidObstacle()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
        startTime = SysTickTime;
        ServoTurnRight();
        PIDMotorSwitch = true;
        step++;
        break;
    case 1:
        nowTime = SysTickTime;
        diffTime = nowTime - startTime;
        //�ӳ�ʱ�䵽  800ms
        if(diffTime * 10 > 1600)
            step++;
        break;
    case 2:
        if(CarGoForward() == FINAL_STEP)
        {
            ServoGoStraight();
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0);
            step = 0;
            startTime = -1;
            return FINAL_STEP;
        }
    }
    return step;
}

//ʮ�� k=0.455
void CarSeekLine(float input,float k)
{
//    if(preservedDirection[0] == false || preservedDirection[1] == false )
//        input = 0;
//        MotorGoForward();
    if(PIDDistanceSwitch == true && (distance_set[0]<=0||distance_set[1]<=0))
    {
        ServoDirectionSet(0);
        return;
    }

    ServoDirectionSet(input/2);
//    if(carStatus == imageSeekLine)
//    {
//        omega_set[0] = nowSpeed[0];
//        omega_set[1] = nowSpeed[1];
//        return;
//    }
    if(input == 0)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 0 && input <= 1)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1] * (1-input*0.5);
    }
    else if(input >= -1 && input < 0)
    {
        omega_set[0] = nowSpeed[0] * (1+input*0.5);
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 1 && input <= 3)
    {
        omega_set[0] = nowSpeed[0] * (1+(input-1)*k); //����
        omega_set[1] = nowSpeed[1] * 0.5;
    }
    else if(input < -1 && input >= -3)
    {
        omega_set[0] = nowSpeed[0] * 0.5;
        omega_set[1] = nowSpeed[1] * (1-(input+1)*k);
    }
//    else if(input > 2 && input <= 3)
//    {
//        omega_set[0] = nowSpeed[0] * (1.0*k+1);
//        omega_set[1] = nowSpeed[1] * (2-input);
//    }
//    else if(input < -2 && input >= -3)
//    {
//        omega_set[0] = nowSpeed[0] * (2+input);
//        omega_set[1] = nowSpeed[1] * (-1-1.0*k);
//    }
}
void CarSeekLine_BigAngle(float input,float k)
{
//    if(preservedDirection[0] == false || preservedDirection[1] == false )
//        input = 0;
//        MotorGoForward();
    if(PIDDistanceSwitch == true && (distance_set[0]<=0||distance_set[1]<=0))
    {
        ServoDirectionSet(0);
        return;
    }

    ServoDirectionSet(input/2);
//    if(carStatus == imageSeekLine)
//    {
//        omega_set[0] = nowSpeed[0];
//        omega_set[1] = nowSpeed[1];
//        return;
//    }
    if(input == 0)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 0 && input <= 1)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1]* (1-input*0.8);
    }
    else if(input >= -1 && input < 0)
    {
        omega_set[0] = nowSpeed[0] * (1+input*0.8);
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 1 && input <= 3)
    {
        omega_set[0] = nowSpeed[0] * (1+(input-1)*k); //����
        omega_set[1] = nowSpeed[1] * 0.8;
    }
    else if(input < -1 && input >= -3)
    {
        omega_set[0] = nowSpeed[0] * 0.8;
        omega_set[1] = nowSpeed[1] * (1-(input+1)*k);
    }
//    else if(input > 2 && input <= 3)
//    {
//        omega_set[0] = nowSpeed[0] * (1.0*k+1);
//        omega_set[1] = nowSpeed[1] * (2-input);
//    }
//    else if(input < -2 && input >= -3)
//    {
//        omega_set[0] = nowSpeed[0] * (2+input);
//        omega_set[1] = nowSpeed[1] * (-1-1.0*k);
//    }
}
void CarSeekLine_ZeroAngle(float input)
{
//    if(preservedDirection[0] == false || preservedDirection[1] == false )
//        input = 0;
//        MotorGoForward();
    if(PIDDistanceSwitch == true && (distance_set[0]<=0||distance_set[1]<=0))
    {
        ServoDirectionSet(0);
        return;
    }

    ServoDirectionSet(input/2);
//    if(carStatus == imageSeekLine)
//    {
//        omega_set[0] = nowSpeed[0];
//        omega_set[1] = nowSpeed[1];
//        return;
//    }
    if(input == 0)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 0 && input <= 1)
    {
        omega_set[0] = nowSpeed[0];
        omega_set[1] = nowSpeed[1] * (1-input);
    }
    else if(input >= -1 && input < 0)
    {
        omega_set[0] = nowSpeed[0] * (1+input);
        omega_set[1] = nowSpeed[1];
    }
    else if(input > 1 && input <= 2)
    {
        omega_set[0] = nowSpeed[0]; //����
        omega_set[1] = 0;
    }
    else if(input < -1 && input >= -2)
    {
        omega_set[0] = 0;
        omega_set[1] = nowSpeed[1];
    }
//    else if(input > 2 && input <= 3)
//    {
//        omega_set[0] = nowSpeed[0];
//        omega_set[1] = nowSpeed[1] * (2-input);
//    }
//    else if(input < -2 && input >= -3)
//    {
//        omega_set[0] = nowSpeed[0] * (2+input);
//        omega_set[1] = nowSpeed[1];
//    }
}
void CarBrake()
{
    PIDDistanceSwitch = true;
    PIDDistanceRestartFlag = true;
    distance_actual[0] = 0;
    distance_actual[1] = 0;
    distance_set[0] = 0;
    distance_set[1] = 0;
    motorStatus = brake;
}
void CarSetDistance(float distance)
{
    PIDDistanceSwitch = true;
    PIDDistanceRestartFlag = true;
    distance_actual[0] = 0;
    distance_actual[1] = 0;
    distance_set[0] = distance;
    distance_set[1] = distance;
    motorStatus = run;
}

int CarTask2()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        startTime_2 = SysTickTime;
        step++;
        break;
    case 1:
        if(car_stop_flag == 0)
        {
            if(board_angle < 3)
                CarSeekLine(image_seek_line_output,0);
            if(board_angle > 9 && board_angle < 10.2)
                CarSeekLine(image_seek_line_output,0.455);
            else
                CarSeekLine(image_seek_line_output,motorSpeedGain);
            motorStatus = imageSeekLine;
        }
        else    //����ͷ��⵽�յ�
        {
            task2_end_flag = false;
            nowSpeed = imageSeekLineSpeed;
            motorStatus = run;
            step++;
        }
        break;
    case 2:
        if(task2_end_flag == true)  //С�������־��
        {
            CarBrake();
            startTime = SysTickTime;
            diffTime = SysTickTime - startTime_2;
            TimeUsed_Display(diffTime/100,diffTime%100,0);
            buzzerTurnOn();
            step++;
        }
        break;
    case 3:
        if((SysTickTime - startTime) * 10 > 1000)
        {
            buzzerTurnOff();
            step = 0;
            return FINAL_STEP;
        }
    }
    return step;
}
void multipleImageSeekLine()
{
    if(board_angle < 9)
        CarSeekLine_ZeroAngle(image_seek_line_output);
    else if(board_angle > 40)
        CarSeekLine_BigAngle(image_seek_line_output,motorSpeedGain);
    else
        CarSeekLine(image_seek_line_output,motorSpeedGain);
    motorStatus = imageSeekLine;
}
int CarTask2_2()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        startTime = SysTickTime;
        startTime_2 = SysTickTime;
        step++;
        break;
    case 1:
        if((SysTickTime - startTime) * 10 > 1500)
        {
            if(board_angle>10.2 && board_angle<32)
                step++;
            else
                step = 4;
        }
        else
        {
            multipleImageSeekLine();
        }
        break;
    case 2:
        if(image_seek_line_output > 1.8)
        {
            step++;
        }
        else
        {
            multipleImageSeekLine();
        }
        break;
    case 3:
        if(image_seek_line_output<1.5)
        {
            step++;
        }
        else
        {
            ServoDirectionSet(1);//�ҹ�
            omega_set[0] = nowSpeed[0];
            omega_set[1] = nowSpeed[0]*0.5;
        }
        break;
    case 4:
        if(car_stop_flag == 0)
        {
            multipleImageSeekLine();
        }
        else    //����ͷ��⵽�յ�
        {
            task2_end_flag = false;
            nowSpeed = imageSeekLineSpeed;
            motorStatus = run;
            step++;
        }
        break;
    case 5:
        if(task2_end_flag == true)  //С�������־��
        {
            PIDTask2EndFlag = true;
            MotorStop();
            startTime = SysTickTime;
            diffTime = SysTickTime - startTime_2;
            TimeUsed_Display(diffTime/100,diffTime%100,0);
            buzzerTurnOn();
            step++;
        }
        break;
    case 6:
        if((SysTickTime - startTime) * 10 > 1000)
        {
            buzzerTurnOff();
            step = 0;
            return FINAL_STEP;
        }
    }
    return step;
}



int CarPitch_2(float input)
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        CarSetDistance(-input * 0.2);
        step++;
        break;
    case 1:
        if(fabs(distance_actual[0]-distance_set[0]) < 0.005)
        {
            CarBrake();
            startTime = SysTickTime;
            step++;
        }
        break;
    case 2:
        if((SysTickTime - startTime) * 10 > 1500)
        {
            if(fabs(input) < 0.01)
            {
                CarBrake();
                step = 0;
                return FINAL_STEP;
            }
            else
                step = 0;
        }
        break;
    }
    return step;
}

int CarPitch(float input)
{
    static int32_t step = 0;
    static int startDirection = 0;    //��ʼ����
    static int nowDirection = 0;      //���ڷ���
    static float x[2] = {0};
    static float x_mid = 0;
    const float stepLength = 0.1;

    if(input > 0)
        nowDirection = -1;
    else if(input < 0)
        nowDirection = 1;
    else
        nowDirection = 0;

    x_mid = (x[0]+x[1])/2;
//    BlueToothSendGraph(&x[0], &x[1], &pitch_step);

    switch(step)
    {
    case 0:
        if(nowDirection == -1) //��ͷ����
        {
            if(startDirection == 0)
                startDirection = nowDirection;
            CarSetDistance(-stepLength);
        }
        else if(nowDirection == 1)  //��ͷ����
        {
            if(startDirection == 0)
                startDirection = nowDirection;
            CarSetDistance(stepLength);
        }
        else
        {
            CarBrake();
            step = 0;
            startTime = -1;
            x[0] = 0; x[1] = 0;
            startDirection = 0;
            return FINAL_STEP;
        }
        step++;
        break;
    case 1:
        if(fabs(distance_actual[0])>0.02 && fabs(omega_actual[0]) < 0.01)
        {
//            if(distance_actual[0] < 0)
//            {
//                x[0] -= (distance_actual[0] + distance_actual[1])/2;
//            }
//            else if(distance_actual[0] > 0)
//                x[1] += (distance_actual[0] + distance_actual[1])/2;
            CarBrake();
            startTime = SysTickTime;
            step++;
        }
        break;
    case 2:
        if((SysTickTime - startTime) * 10 > 1500)
        {
            if(nowDirection == startDirection)
            {
                step = 0;
            }
            else
            {
                x[0] = 0;
                x[1] = 0.2;
                step++;
            }
        }
        break;
    case 3:
        if(nowDirection == 1)
        {
            CarSetDistance(x_mid - x[0]);
            x[0] = x_mid;
        }
        else if(nowDirection == -1)
        {
            CarSetDistance(x_mid - x[1]);
            x[1] = x_mid;
        }
        else
        {
            step = 0;
            startTime = -1;
            x[0] = 0; x[1] = 0;
            startDirection = 0;
            CarBrake();
            return FINAL_STEP;
        }
        if(distance_set[0]<0.01 && distance_set[0]>0)
            CarSetDistance(0.01);
        else if(distance_set[0]<0 && distance_set[0]>-0.01)
            CarSetDistance(-0.01);

        step++;
        break;
    case 4:
        if(fabs(distance_actual[0])>0.007 && fabs(omega_actual[0]) < 0.01)
        {
            CarBrake();
            startTime = SysTickTime;
            step++;
        }
        break;
    case 5:
        if((SysTickTime - startTime) * 10 > 3000)
        {
            step = 3;
        }
        break;
    }
    return step;
}

int CarTask1()
{
    static int32_t step = 0;
    switch(step)
    {
    case 0:
        PIDDistanceSwitch = false;
        motorStatus = run;
        if(J901_Output[0][1] < 0)
        {
            CarBrake();
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
            startTime = SysTickTime;
            step++;
        }
        break;
    case 1:
        if((SysTickTime-startTime) * 10 > 500)
        {
            step++;
        }
        break;
    case 2:
//        nowSpeed = omega_set_output;
        pitch_step = CarPitch(pitch_output);
        if(pitch_step == FINAL_STEP)  //����C
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
            startTime = SysTickTime;
            step++;
        }
        break;
    case 3:
        //�ӳ�ʱ��
        if((SysTickTime-startTime) * 10 > 5000)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
            CarSetDistance(0.36);
            motorStatus = seekLine;
            startTime = SysTickTime;
            step++;
        }
        break;
    case 4:   //����B
        if(fabs(distance_actual[0])>0.01 && fabs(omega_actual[0]) < 0.01)
        {
            CarBrake();

            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
            startTime = SysTickTime;
            step++;
        }
        break;
    case 5:
        //�ӳ�ʱ��
        if((SysTickTime-startTime) * 10 > 5000)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
            CarSetDistance(-0.7);
            nowSpeed = pitchSpeed;
            startTime = SysTickTime;
            step++;
        }
        break;
    case 6:    //����A
        if(fabs(distance_actual[0])>0.01 && fabs(omega_actual[0]) < 0.01)
        {
            CarBrake();
            step = 0;
            startTime = -1;
            return FINAL_STEP;
        }
    }
    return step;
}
/*
 * �������  ǰPE0  ��Ѱ��PE123�жϺϲ�
 */
void InfraredAvoidanceInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//������io��������Ϊ�ߵ�ƽ�����ƽ��Ϊ�͵�ƽ����Ϊ�䰴��

    GPIOIntTypeSet(GPIO_PORTE_BASE,GPIO_PIN_0, GPIO_LOW_LEVEL);
    GPIOIntEnable(GPIO_PORTE_BASE,GPIO_INT_PIN_0);
    //�ж�ע�ᣬ�ڶ����������жϷ��������
    GPIOIntRegister(GPIO_PORTE_BASE,IntHandler_GPIOE_SeekLine_Infrared);
    //Port�ж�ʹ��
    IntEnable(INT_GPIOE);
    //�������ж�
    IntMasterEnable();
}

//void IntHandler_GPIOE_Infrared(void)//��������жϷ�����
//{
//    uint32_t intStatus = GPIOIntStatus(GPIO_PORTD_BASE, true);//��ȡ״̬
//
//    if((intStatus & GPIO_PIN_0) == GPIO_PIN_0)//��������ж����ĸ�io�ڵ��ж�
//    {
//        SysCtlDelay(SysCtlClockGet()*0.04/3);     //0.03s
//        //��������
//        uint32_t PDStatus = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);
//        if((PDStatus & GPIO_PIN_0) != GPIO_PIN_0 && motorStatus != stop && motorStatus != seekLine)
//        {
//            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
//            motorStatus = obstacle;
//            nowSpeed = preservedSpeed;
//
//            MotorStop();
//            SysCtlDelay(SysCtlClockGet()*0.4/3);     //0.01s
//            //�����ת
//            MotorDirectionSet(LEFT, false);
//            MotorDirectionSet(RIGHT, false);
//            SysCtlDelay(SysCtlClockGet()*0.2/3);     //0.01s
//            MotorRun();
//            SysCtlDelay(SysCtlClockGet()*0.2/3);     //0.01s
//        }
//        while(!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0))
//        {
//            ;
//        }
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0);
//        GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);//���ָ�����ж�Դ
//    }
//}

/*
 * ����Ѱ��   GPIO PE123
 */
void SeekLineInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//������io��������Ϊ�ߵ�ƽ�����ƽ��Ϊ�͵�ƽ����Ϊ�䰴��

    GPIOIntTypeSet(GPIO_PORTE_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_BOTH_EDGES);
    IntPrioritySet(INT_GPIOE,0x01);
    GPIOIntEnable(GPIO_PORTE_BASE,GPIO_INT_PIN_1|GPIO_INT_PIN_2|GPIO_INT_PIN_3);
    //�ж�ע�ᣬ�ڶ����������жϷ��������
    GPIOIntRegister(GPIO_PORTE_BASE,IntHandler_GPIOE_SeekLine_Infrared);
    //Port�ж�ʹ��
    IntEnable(INT_GPIOE);
    //�������ж�
    IntMasterEnable();
}
void IntHandler_GPIOE_SeekLine_Infrared()
{
    uint32_t intStatus = GPIOIntStatus(GPIO_PORTE_BASE, true);//��ȡ״̬

    if((intStatus & (GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)))//��������ж����ĸ�io�ڵ��ж�
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
//        SysCtlDelay(SysCtlClockGet()*0.005/3);     //0.03s

        uint32_t PEStatus = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
        PEStatus &= (GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

        switch(PEStatus)
        {
        case GPIO_PIN_1|GPIO_PIN_3:       //101  �м�
            seek_line_actual = 0;
            break;
        case GPIO_PIN_3:     //001 011 ƫ��
            seek_line_actual = 1;
            break;
        case GPIO_PIN_2|GPIO_PIN_3:
            seek_line_actual = 2;
            break;
        case GPIO_PIN_1:     //100 110 ƫ��
            seek_line_actual = -1;
            break;
        case GPIO_PIN_1|GPIO_PIN_2:
            seek_line_actual = -2;
            break;
        case 0:    //000 �յ�
            seek_line_end = true;
            break;
        default:
//            seek_line_actual = 0;
            break;
        }
        if(PEStatus != 0)
            seek_line_end = false;

        if(PEStatus < 14)  //111Ϊ7
        {
            task2_end_flag = true;
        }
    }
    else if((intStatus & GPIO_PIN_0) == GPIO_PIN_0)//��������ж����ĸ�io�ڵ��ж�
    {
        SysCtlDelay(SysCtlClockGet()*0.02/3);     //0.03s
        //��������
        uint32_t PDStatus = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);
        if((PDStatus & GPIO_PIN_0) != GPIO_PIN_0 && carStatus != stop && carStatus != seekLine
                && carStatus != brake && carStatus != commandGoBack && carStatus != commandGoForward)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);
            carStatus = obstacle;
            nowSpeed = defaultSpeed;
            //�ȴ�С���������Ϸ�Χ
            //ɲ��
            MotorBrake();
            SysCtlDelay(SysCtlClockGet()*0.8/3);     //0.01s
            //�����ת
            MotorSpeedSet(0, 0);
            MotorSpeedSet(1, 0);
            MotorGoBack();
            SysCtlDelay(SysCtlClockGet()*0.1/3);     //0.01s
            MotorSpeedSet(0, 0.3);
            MotorSpeedSet(1, 0.3);
            SysCtlDelay(SysCtlClockGet()*0.1/3);     //0.01s
            MotorSpeedSet(0, 0.7);
            MotorSpeedSet(1, 0.7);
            SysCtlDelay(SysCtlClockGet()*0.2/3);     //0.01s
        }
        while(!GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0))
        {
            ;
        }
    }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0);
    GPIOIntClear(GPIO_PORTE_BASE, intStatus);//���ָ�����ж�Դ
}

