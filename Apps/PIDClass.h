/*
 * PIDClass.h
 *
 *  Created on: 2020年9月12日
 *      Author: 电脑
 */

#ifndef APPS_PIDCLASS_H_
#define APPS_PIDCLASS_H_

namespace pid
{

class PIDClass
{
public:
    PIDClass(float* pActualValue, float* pSetValue, float Kp, float Ki, float Kd);
    void Reset();
    virtual void SetKPID(EEPROM_PID_DATA* pidData);
    virtual float Compute() = 0;
    virtual ~PIDClass();

    float Kp,Ki,Kd;  //pid系数
    float output;    //输出
    float nowErr;    //当前偏差值
    float integral;  //积分值
protected:
    float* pSetValue;  //设定值
    float* pActualValue;  //实际值

    float lastErr;   //上一个偏差值

};

class PIDMotorClass : public PIDClass
{
public:
    PIDMotorClass(float* pActualValue, float* pSetValue,float Kp, float Ki, float Kd);
    float Compute();
};

class PIDDistanceClass : public PIDClass
{
public:
    PIDDistanceClass(float* pActualValue, float* pSetValue, float omegaMax, float Kp, float Ki, float Kd);
    float Compute();
    float outputMax;
};

class PIDSeekLineIntClass : public PIDClass
{
public:
    PIDSeekLineIntClass(float* pActualValue, float* pSetValue,float Kp, float Ki, float Kd);
    float Compute();
};
class PIDSeekLineClass : public PIDClass
{
public:
    PIDSeekLineClass(float* pActualValue, float* pSetValue,float Kp, float Ki, float Kd);
    float Compute();
};

class PIDPitchClass : public PIDClass
{
public:
    PIDPitchClass(float* pActualValue, float* pSetValue,float Kp, float Ki, float Kd);
    float Compute();
};

class PIDImageSeekLineClass : public PIDClass
{
public:
    PIDImageSeekLineClass(float* pActualValue,float *continuousActualValue, float* pSetValue,float Kp, float Ki, float Kd, float Kd_angle);
    void ContinuousCompute();
    void SetKPID(EEPROM_PID_DATA* pidData);
    float Compute();
    float *continuousActualValue;
    float K_theta;
};

} /* namespace pid */

#endif /* APPS_PIDCLASS_H_ */
