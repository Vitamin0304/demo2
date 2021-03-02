/*
 * PIDClass.cpp
 *
 *  Created on: 2020年9月12日
 *      Author: 电脑
 */
extern "C"
{
#include "Apps/my_eeprom.h"
}
#include <cmath>
#include <Apps/PIDClass.h>
#define PI 3.1415926f

namespace pid
{

PIDClass::PIDClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd)
{
    this->pActualValue = pActualValue;
    this->pSetValue = pSetValue;
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
    this->lastErr = 0;
    this->integral = 0;
}
PIDClass::~PIDClass()
{
    // TODO Auto-generated destructor stub
}
void PIDClass::Reset()
{
    this->lastErr = 0;
    this->integral = 0;
}
void PIDClass::SetKPID(EEPROM_PID_DATA* pidData)
{
    this->Kp = pidData->Kp;
    this->Ki = pidData->Ki;
    this->Kd = pidData->Kd;
}

PIDMotorClass::PIDMotorClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd)
{
}
float PIDMotorClass::Compute()
{
    if(*pActualValue > 60)
        *pActualValue = *pSetValue - lastErr;

    float P, I, D;
    nowErr = *pSetValue - *pActualValue;
    integral += nowErr;

    P = Kp * nowErr;
    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;
    output /= 50;
    if(output > 0.95)
        output = 0.95;
    else if(output < -0.95)
        output = -0.95;
    return output;
}

PIDDistanceClass::PIDDistanceClass(float* pActualValue, float* pSetValue, float outputMax, float Kp, float Ki, float Kd)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd),outputMax(outputMax)
{
}
float PIDDistanceClass::Compute()
{
    float P, I, D;
    float beta = 1;
    nowErr = *pSetValue * beta - *pActualValue;
    integral += nowErr;

    Kp = 8;
    Ki = 0.001;
    Kd = 3;
//    if(fabs(*pSetValue) > 0.2)
//    {
//        Kp = 4;
//        Ki = 0.001;
//        Kd = 2;
//    }
//    else if(fabs(*pSetValue) > 0.05)
//    {
//        Kp = 7;
//        Ki = 0.001;
//        Kd = 3;
//    }
//    else
//    {
//        Kp = 14;
//        Ki = 0.05;
//        Kd = 6;
//    }

    //积分限幅
    float integral_max = 10;
    I = Ki * integral;
    if(I > integral_max)
        integral = integral_max / Ki;
    if(I < -integral_max)
        integral = -integral_max / Ki;

    P = Kp * nowErr;
    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;
    output *= 20;
    if(output > outputMax)
        output = outputMax;
    else if(output < -outputMax)
        output = -outputMax;
    else if(output > -2 && output < 2)
        output = 0;
    return output;
}

PIDSeekLineIntClass::PIDSeekLineIntClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd)
{
}
float PIDSeekLineIntClass::Compute()
{
    float P, I, D;
    nowErr = *pSetValue - *pActualValue;
    integral += nowErr;

    float reductionSpeed = 2.0f;
    if(nowErr == 0)
    {
        if(integral > reductionSpeed)
            integral -= reductionSpeed;
        else if(integral < -reductionSpeed)
            integral += reductionSpeed;
        else
            integral = 0;
    }

    //积分限幅
    float integral_max = 2;
    I = Ki * integral;
    if(I > integral_max)
        integral = integral_max / Ki;
    if(I < -integral_max)
        integral = -integral_max / Ki;

    P = Kp * nowErr;
    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;

//    const float exp_k = 0.313035285;
//    if(output > 0)
//        output = (exp(output) - 1) * exp_k;
//    else
//        output = (-exp(-output) + 1) * exp_k;

    if(output > 2)
        output = 2;
    else if(output < -2)
        output = -2;
//    else if(output < 0.1 && output > -0.1)
//        output = 0;
    return -output;
}

PIDSeekLineClass::PIDSeekLineClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd)
{

}

float PIDSeekLineClass::Compute()
{
    float P, I, D;
    nowErr = *pSetValue - *pActualValue;
    integral += nowErr;

    float reductionSpeed = 2.0f;
    if(nowErr == 0)
    {
        if(integral > reductionSpeed)
            integral -= reductionSpeed;
        else if(integral < -reductionSpeed)
            integral += reductionSpeed;
        else
            integral = 0;
    }

    I = Ki * integral;
    float integral_max = 2;
    if(I > integral_max)
        integral = integral_max / Ki;
    if(I < -integral_max)
        integral = -integral_max / Ki;

    P = Kp * nowErr;
//    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;

//    const float exp_k = 0.313035285;
//    if(output > 0)
//        output = (exp(output) - 1) * exp_k;
//    else
//        output = (-exp(-output) + 1) * exp_k;

    if(output > 2)
        output = 2;
    if(output < -2)
        output = -2;

    return output;
}

PIDPitchClass::PIDPitchClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd)
{
}
float PIDPitchClass::Compute()
{
    float P, I, D;
    nowErr = *pSetValue - *pActualValue;
    integral += nowErr;

    if(nowErr > -1.43 && nowErr < 1.43)
        nowErr = 0;

    I = Ki * integral;
    float integral_max = 1;
    if(I > integral_max)
        integral = integral_max / Ki;
    if(I < -integral_max)
        integral = -integral_max / Ki;

    P = Kp * nowErr;
//    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;

    if(output > 1)
        output = 1;
    if(output < -1)
        output = -1;

    return output;
}

PIDImageSeekLineClass::PIDImageSeekLineClass(float* pActualValue,float *continuousActualValue, float* pSetValue, float Kp, float Ki, float Kd, float K_theta)
    :PIDClass(pActualValue,pSetValue,Kp,Ki,Kd),K_theta(K_theta),continuousActualValue(continuousActualValue)
{
}
void PIDImageSeekLineClass::SetKPID(EEPROM_PID_DATA* pidData)
{
    PIDClass::SetKPID(pidData);
    this->K_theta = pidData->K_theta;
}
void PIDImageSeekLineClass::ContinuousCompute()
{
    float changeSpeed = 4.5f;
    float err[2];
    err[0] = pActualValue[0] - continuousActualValue[0];
    err[1] = pActualValue[1] - continuousActualValue[1];

    for(int i=0;i<2;++i)
    {
        if(err[i] > changeSpeed)
            continuousActualValue[i] += changeSpeed;
        else if(err[i] < -changeSpeed)
            continuousActualValue[i] -= changeSpeed;
        else
            continuousActualValue[i] = pActualValue[i];
    }
}
float PIDImageSeekLineClass::Compute()
{
    float P, I, D;

    nowErr = (pSetValue[0] - continuousActualValue[0])*cos(continuousActualValue[1]/180*PI);

    integral += nowErr;

//    if(nowErr > -1.43 && nowErr < 1.43)
//        nowErr = 0;
    float reductionSpeed = 4.0f;
    if(nowErr == 0)
    {
        if(integral > reductionSpeed)
            integral -= reductionSpeed;
        else if(integral < -reductionSpeed)
            integral += reductionSpeed;
        else
            integral = 0;
    }

    I = Ki * integral;
    float integral_max = 2;
    if(I > integral_max)
        integral = integral_max / Ki;
    if(I < -integral_max)
        integral = -integral_max / Ki;

    P = Kp * nowErr;
//    I = Ki * integral;
    D = Kd * (nowErr - lastErr);

    lastErr = nowErr;
    output = P + I + D;

    if(output > 2)
        output = 2;
    else if(output < -2)
        output = -2;

    return output;
}

} /* namespace pid */
