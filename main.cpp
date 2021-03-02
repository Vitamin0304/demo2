
/**
 * main.c
 */
extern "C" {
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
//#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"
#include "Apps/led_pwm.h"
#include "Apps/encoder.h"
#include "Apps/my_gpio.h"
#include "Apps/my_uart.h"
#include "Apps/motor_timer.h"
#include "Apps/bluetooth.h"
#include "Apps/motor.h"
#include "Apps/CarController.h"
#include <Apps/my_eeprom.h>
#include "Apps/MyJ901.h"
#include "Apps/MyBlueTooth.h"
#include "Apps/cJson.h"
//#include "Apps/LMT70.h"
#include "OLED/MyOLED.h"
#include "Apps/delay.h"
//#include "Apps/Mymcu90615.h"
#include "Apps/buzzer.h"
#define PI 3.1415926f
#define R  0.0325f
}
#include <Apps/PIDClass.h>

using namespace pid;

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

//    bool ledState[3] = { true, false, false };
//    M1PWM567Init(1000, 0.8, ledState);
    EEPROMMyInit();
    //陀螺仪
    J901_Init();
    //LMT70
//    LMT70_ADC1_CH0_Init();
//    GPIOKey1Init();
    GPIOLEDInit();
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
    OLED_Init();
    TimeUsed_Display(0,0,1);

//    MCU90615_Init(115200,115200);
    buzzerInit(3000,0.5);
    UART0Init(115200);
//    UART1Init(115200);
    //0.68 0.0023 0.014
    EEPROM_PID_DATA KpidSeekLineData = {0.5, 0.003, 0.01};
    EEPROM_PID_DATA KpidPicthData = {0.2, 0, 0};
    EEPROM_PID_DATA KpidImageData = {0, 0, 0, 0};

//    EEPROMPIDRead(&KpidData,0x400);
//    EEPROMPIDRead(&KpidPicthData,0x500);
    EEPROMPIDRead(&KpidImageData,0x440);

    BlueToothInit(115200,&KpidImageData);

    _Uart3_Init(115200);
    _Uart3_int_Init();

    SysTickInit();

    CarInit();
    uint32_t times = 0;
    ServoDirectionSet(0);

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

    //while主循环周期，编码器计算周期
    float delay_s = 0.02;
    TimerInterruptInit(delay_s, true);

    float frequency[2] = {0};

    PIDMotorClass pidMotor0(&omega_actual[0], &omega_set[0], 2.4, 0.25, 0.7);
    PIDMotorClass pidMotor1(&omega_actual[1], &omega_set[1], 2.4, 0.25, 0.7);
    float motor_pwm_output[2];

    float velocity_actual[2];   //轮子线速度
    PIDDistanceClass pidDistance0(&distance_actual[0],&distance_set[0],20,12,0,1);
    PIDDistanceClass pidDistance1(&distance_actual[1],&distance_set[1],20,12,0,1);

    PIDSeekLineIntClass pidSeekLineInt(&seek_line_actual,&seek_line_set,0,0.18,0);
    PIDSeekLineClass pidSeekLine(&seek_line_actual_int,&seek_line_set,KpidSeekLineData.Kp,KpidSeekLineData.Ki,KpidSeekLineData.Kd);

    PIDImageSeekLineClass pidImageSeekLine(image_seek_line_actual,image_seek_line_actual_int,image_seek_line_set,KpidImageData.Kp,KpidImageData.Ki,KpidImageData.Kd,0);

    PIDPitchClass pidPitch(&J901_Output[0][1],&pitch_set,KpidPicthData.Kp,KpidPicthData.Ki,KpidPicthData.Kd);

    SysCtlDelay(SysCtlClockGet()*0.5/3);

    while(1)
    {
        pidImageSeekLine.SetKPID(&KpidImageData);
        pidPitch.SetKPID(&KpidPicthData);
        pidSeekLine.SetKPID(&KpidSeekLineData);
        //传感器获取电机转速
        GetEncoderFreq(frequency);
        omega_actual[0] = frequency[0] / 390 * 2 * PI * omega_sign[0];
        omega_actual[1] = frequency[1] / 390 * 2 * PI * omega_sign[1];
        //计算小车速度和路程
        velocity_actual[0] = omega_actual[0] * R;
        velocity_actual[1] = omega_actual[1] * R;
        distance_actual[0] += velocity_actual[0] * delay_s;
        distance_actual[1] += velocity_actual[1] * delay_s;
        //路程环
        if(PIDDistanceSwitch == true)
        {
            if(PIDDistanceRestartFlag == true)
            {
                PIDDistanceRestartFlag = false;
                pidDistance0.Reset();
                pidDistance1.Reset();
            }
            if(nowSpeed == omega_set_output)
            {
                pidDistance0.outputMax = pitchSpeed[0];
                pidDistance1.outputMax = pitchSpeed[1];
            }else
            {
                pidDistance0.outputMax = nowSpeed[0];
                pidDistance1.outputMax = nowSpeed[1];
            }
            omega_set_output[0] = pidDistance0.Compute();
            omega_set_output[1] = pidDistance1.Compute();
            omega_set[0] = omega_set_output[0];
            omega_set[1] = omega_set_output[1];
        }
        else
        {
            distance_actual[0] = 0;
            distance_actual[1] = 0;
            pidDistance0.Reset();
            pidDistance1.Reset();
        }
        CarStatusManage();
        //寻线PID
//        if(carStatus == seekLine || carStatus == task2)
//        {
//            seek_line_actual_int = pidSeekLineInt.Compute();
//            seek_line_output = pidSeekLine.Compute();
//        }
//        else
//        {
//            pidSeekLine.Reset();
//            seek_line_actual = 0;
//            seek_line_output = 0;
//        }
        if(motorStatus == imageSeekLine)
        {
            pidImageSeekLine.ContinuousCompute();
            image_seek_line_output = pidImageSeekLine.Compute();
        }
        else
        {
            pidImageSeekLine.Reset();
//            image_seek_line_actual[0] = 0; image_seek_line_actual[1] = 0;
            image_seek_line_output = 0;
        }
//        if(carStatus == task1 && (omega_actual[0] < 0.01))
//        {
//            pidSeekLine.Reset();
//            ServoDirectionSet(0);
//            seek_line_actual = 0;
//            seek_line_output = 0;
//        }
        //跷跷板
//        if(carStatus == pitch || carStatus == task2)
//        {
//            pitch_output = pidPitch.Compute();
//        }
//        else
//        {
//            pidSeekLine.Reset();
//        }
        //速度环
        MotorStatusManage();
        if(PIDIntegralRestartFlag == true)
        {
            PIDIntegralRestartFlag = false;
            pidMotor0.integral = 90;
            pidMotor1.integral = 90;
        }
        if(PIDTask2EndFlag == true)
        {
            PIDTask2EndFlag = false;
            pidMotor0.integral = 0;
            pidMotor1.integral = 0;
        }
        if(PIDMotorSwitch == true)
        {
            motor_pwm_output[0] = pidMotor0.Compute();
            motor_pwm_output[1] = pidMotor1.Compute();

            for(int i=0;i<2;++i){
                if(motor_pwm_output[i] >= 0)
                {
                    MotorDirectionSet(i, true);
                    MotorSpeedSet(i, motor_pwm_output[i]);
                }
                else
                {
                    MotorDirectionSet(i, false);
                    MotorSpeedSet(i, -motor_pwm_output[i]);
                }
            }
        }
        else
        {
            pidMotor0.Reset();
            pidMotor1.Reset();
        }

        float time = (float)SysTickTime;

//        BlueToothSendGraph(&image_seek_line_output,&omega_set[0],&image_seek_line_actual[0]);
        BlueToothSendGraph(&omega_set[0], &omega_actual[0], &motor_pwm_output[0]);
//        BlueToothSendGraph(&J901_Output[0][0],&J901_Output[0][1],&J901_Output[0][2]);
        float car_flag = car_stop_flag;
        float end_flag = task2_end_flag;
//        BlueToothSendGraph(&task2_step,&end_flag,&car_flag);
//        BlueToothSendGraph(&image_seek_line_output,&omega_actual[1],&task2_step);
        delayFlag = true;
        while(delayFlag);
    }
	return 0;
}
