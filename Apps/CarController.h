/*
 * CarController.h
 *
 *  Created on: 2020年9月17日
 *      Author: 电脑
 */

#ifndef APPS_CARCONTROLLER_H_
#define APPS_CARCONTROLLER_H_

#define FINAL_STEP       -1

//#define pitch_actual J901_Output[0][1]
#include <stdint.h>
#include <stdbool.h>

extern float distance_actual[2];
extern float distance_set[2];
extern float omega_set_output[2];
extern bool PIDDistanceSwitch;
extern bool PIDDistanceRestartFlag;
extern enum MotorStatus carStatus;

extern float seek_line_set;
extern float seek_line_actual;
extern float seek_line_actual_int;
extern float seek_line_output;

extern float image_seek_line_set[2];
extern float image_seek_line_actual[2];
extern float image_seek_line_actual_int[2];
extern float image_seek_line_output;

extern uint8_t car_stop_flag;
extern bool task2_end_flag;
extern float task2_set_time;
extern bool PIDIntegralRestartFlag;
extern float board_angle;
extern float motorSpeedGain;
extern bool PIDTask2EndFlag;

extern float pitch_set;
extern float pitch_output;
extern float pitch_step;

extern float task1_step;
extern float task2_step;

void CarInit();

void CarCommandQuery(uint32_t infraredCommand);
void CarBlueToothQuery(uint32_t blueToothCommand);
void CarStatusManage();

void CarSeekLine_BigAngle(float input,float k);
void CarSeekLine_ZeroAngle(float input);
void CarSeekLine(float input,float k);
int CarGoForward();
int CarGoBack();
int CarAvoidObstacle();
void CarBrake();
void CarSetDistance(float distance);
int CarPitch(float input);
int CarPitch_2(float input);
int CarTask1();
int CarTask2();
int CarTask2_2();

void InfraredAvoidanceInit(void);
//void IntHandler_GPIOE_Infrared(void);

void SeekLineInit();
void IntHandler_GPIOE_SeekLine_Infrared();

#endif /* APPS_CARCONTROLLER_H_ */
