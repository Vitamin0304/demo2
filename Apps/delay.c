/*
 * delay.c
 *
 *  Created on: 2020Äê9ÔÂ10ÈÕ
 *      Author: STARSKY
 */
#include "delay.h"

void delay_1ms(uint32_t delaytime)
{
    uint32_t cnt_1ms = 0;
    cnt_1ms = SysCtlClockGet()/3000;
    SysCtlDelay(delaytime*cnt_1ms);
}

void delay_1us(uint32_t delaytime)
{
    uint32_t cnt_1us = 0;
    cnt_1us = SysCtlClockGet()/3000000;
    SysCtlDelay(delaytime*cnt_1us);
}

void delay_1s(uint32_t delaytime)
{
    uint32_t cnt_1s = 0;
    cnt_1s = SysCtlClockGet()/3;
    SysCtlDelay(delaytime*cnt_1s);
}







