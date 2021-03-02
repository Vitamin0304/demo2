/*
 * motor.h
 *
 *  Created on: 2020年8月16日
 *      Author: 电脑
 */

#ifndef APPS_MOTOR_H_
#define APPS_MOTOR_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define LEFT  0
#define RIGHT 1

enum MotorStatus
{
    stop = 0,
    run,
    brake,  //刹车
    commandGoForward,
    commandGoBack,
    //舵机操作
    commandTurnRight,
    commandTurnLeft,
    commandGoStraight,
    //------
    obstacle,
    seekLine,
    pitch,
    task2,
    imageSeekLine,
    slopeBrake
};

extern float omega_set[2];
extern float omega_actual[2];

extern bool PIDMotorSwitch;

extern enum MotorStatus motorStatus;
extern enum MotorStatus preservedStatus;

extern float defaultSpeed[2];
extern float seekLineSpeed[2];
extern float pitchSpeed[2];
extern float imageSeekLineSpeed[2];
extern float* nowSpeed;
extern bool preservedDirection[2];

void MotorGPIOInit();
void MotorDirectionSet(uint32_t oneSide, bool isForward);
void ServoDirectionSet(float direction);

void MotorSpeedSet(int motor, float duty);


void MotorCommandQuery(uint32_t infraredCommand);
void MotorBlueToothQuery(uint32_t blueToothCommand);

void MotorBrake();
void MotorStop();
void MotorRun();
void MotorGoForward();
void MotorGoBack();

void ServoTurnLeft();
void ServoTurnRight();
void ServoGoStraight();

void MotorStatusManage();


#endif /* APPS_MOTOR_H_ */
