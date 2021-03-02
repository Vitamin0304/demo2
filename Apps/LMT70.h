/*
 * LMT70.h
 *
 *  Created on: 2020年10月7日
 *      Author: 电脑
 */

#ifndef APPS_LMT70_H_
#define APPS_LMT70_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/fpu.h"
#include "driverlib/adc.h"
#include "delay.h"

extern float output_lmt70;//获取的温度值
#define LMT70_cnt_max  3//1——3 滤波程度，采集的间隔时间增加，滤波程度可以适当大于3，滤波程度越大曲线越平滑，但获取数据频率越低
/**
  ******************************************************************************
  * @file           : lmt70.c
  * @brief          : lmt70负温度系数电压输出型温度传感器驱动源文件
  * @details        : lmt70的初始化和电压到温度的转换输出
  * @author         : Charmander 有为电子科技 QQ: 228303760
  * @date           : 2020/9/2
  * @version        : V0.1
  * @copyright      : Copyright (C)
  ******************************************************************************
  * @attention
  * -不依赖于具体硬件，通过调用个底层接口实现功能。
  * -手册上提供了三种电压-温度转换方式分别是一阶导数查表法，二阶导数公式法、三阶导数公式法
  * 一阶导数查表法适用于人体窄温度范围的温度测量，二阶和三阶公式法在宽温度范围-50度到100+度
  * 提供全局更准确的转换。
  * -对于没有fpu单元的mcu和人体温度测温范围的应用我们推荐使用一阶导数查表法。
  ******************************************************************************
  * @verbatim
  * 修改历史:
  *      1. 日期：2020/9/2
  *         作者：Charmander
  *         修改：
  * @endverbatim
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/** @defgroup ADS1292R ADS1292R
  * @{
  */
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup ADS1292R_Exported_Functions ADS1292R Exported Functions
  * @{
  */

/** @brief  初始化lmt70
  * @details 完成与lmt70相关的底层外设的初始化。例如adc，gpio。
  * @retval NONE
  */


void LMT70_ADC1_CH0_Init(void);

/** @brief  从lmt70获取温度数据
  * @details 使用一阶导数查表法实现电压-温度转换
  * @param[in]  vol lmt70输出电压单位mv
  * @retval 电压-温度转换结果放大100倍 -5000到+15000度
  */
int32_t lmt70_get_temp( float vol) ;

void LMT70_ADC1_CH0_Exe(void);//中断服务函数
void LMT_DATA_GET(void);//使能ADC数据采集

#ifdef __cplusplus
}
#endif

#endif /* APPS_LMT70_H_ */
