/*
 * LMT70.h
 *
 *  Created on: 2020��10��7��
 *      Author: ����
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

extern float output_lmt70;//��ȡ���¶�ֵ
#define LMT70_cnt_max  3//1����3 �˲��̶ȣ��ɼ��ļ��ʱ�����ӣ��˲��̶ȿ����ʵ�����3���˲��̶�Խ������Խƽ��������ȡ����Ƶ��Խ��
/**
  ******************************************************************************
  * @file           : lmt70.c
  * @brief          : lmt70���¶�ϵ����ѹ������¶ȴ���������Դ�ļ�
  * @details        : lmt70�ĳ�ʼ���͵�ѹ���¶ȵ�ת�����
  * @author         : Charmander ��Ϊ���ӿƼ� QQ: 228303760
  * @date           : 2020/9/2
  * @version        : V0.1
  * @copyright      : Copyright (C)
  ******************************************************************************
  * @attention
  * -�������ھ���Ӳ����ͨ�����ø��ײ�ӿ�ʵ�ֹ��ܡ�
  * -�ֲ����ṩ�����ֵ�ѹ-�¶�ת����ʽ�ֱ���һ�׵�����������׵�����ʽ�������׵�����ʽ��
  * һ�׵����������������խ�¶ȷ�Χ���¶Ȳ��������׺����׹�ʽ���ڿ��¶ȷ�Χ-50�ȵ�100+��
  * �ṩȫ�ָ�׼ȷ��ת����
  * -����û��fpu��Ԫ��mcu�������¶Ȳ��·�Χ��Ӧ�������Ƽ�ʹ��һ�׵��������
  ******************************************************************************
  * @verbatim
  * �޸���ʷ:
  *      1. ���ڣ�2020/9/2
  *         ���ߣ�Charmander
  *         �޸ģ�
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

/** @brief  ��ʼ��lmt70
  * @details �����lmt70��صĵײ�����ĳ�ʼ��������adc��gpio��
  * @retval NONE
  */


void LMT70_ADC1_CH0_Init(void);

/** @brief  ��lmt70��ȡ�¶�����
  * @details ʹ��һ�׵������ʵ�ֵ�ѹ-�¶�ת��
  * @param[in]  vol lmt70�����ѹ��λmv
  * @retval ��ѹ-�¶�ת������Ŵ�100�� -5000��+15000��
  */
int32_t lmt70_get_temp( float vol) ;

void LMT70_ADC1_CH0_Exe(void);//�жϷ�����
void LMT_DATA_GET(void);//ʹ��ADC���ݲɼ�

#ifdef __cplusplus
}
#endif

#endif /* APPS_LMT70_H_ */
