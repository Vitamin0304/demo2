/*
 * LMT70.c
 *
 *  Created on: 2020年10月7日
 *      Author: 电脑
 */

//LMT70数据采集方法介绍
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

  */

/* Includes ------------------------------------------------------------------*/
#include "LMT70.h"
float output_lmt70 = 0;
uint32_t SequenceNum = 1;
/** @addtogroup ADS1292R
  * @{
  */
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define VOL2TEMP_TABLE_SIZE 21
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* 电压温度转换边，电压为μv，温度从-50度开始，步进为10度，向上增加
 * 比如vol2temp_table[0] 表示-50度，vol2temp_table[1]就表示-40度。
 * 此处最好换成对应adc转换后的数字输出，这样就可以避免adc输出换算成
 * 电压导致的运算误差，同时也提高了运算速度。
 */
float v2t_tabl[VOL2TEMP_TABLE_SIZE] = {
                                1350.441,1300.593,1250.398,1199.884,1149.070,
                                1097.987,1046.647, 995.050, 943.227, 891.178,
                                 838.882, 786.360, 733.608, 680.654, 627.490,
                                 574.117, 520.551, 466.760, 412.739, 358.164,
                                 302.785
} ;

/* 斜率表，在十度区间内使用线性插值法 */
float slope_tabl[VOL2TEMP_TABLE_SIZE] = {
                                4.985,5.020,5.051,5.081,5.108,
                                5.134,5.160,5.182,5.205,5.230,
                                5.252,5.275,5.295,5.316,5.337,
                                5.357,5.379,5.402,5.458,5.538,
                                5.538
} ;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/* 折半查表实现 */
static int16_t haltserach( float vol)
{
    int16_t low = 0 ;
    int16_t up = VOL2TEMP_TABLE_SIZE ;
    int16_t mid ;

    while ( up >= low)
    {
        mid = ( low + up) >> 1 ;

        if ( v2t_tabl[mid] < vol)
        {
            up = mid - 1 ;
        }
        else if ( vol < v2t_tabl[mid])
        {
            low = mid + 1 ;
        }
        else
        {
            return mid ;
        }
    }

    /* 不在线性表里，返回-1或RT2TEMP_TABLE_SIZE表示不在表内 */
    /* 其他值，则取下边界温度并返回 */

    return up ;
}

/** @brief  初始化lmt70
  * @details 完成与lmt70相关的底层外设的初始化。例如adc，gpio。
  * @retval NONE
  */

/** @brief  从lmt70获取温度数据
  * @details 使用一阶导数查表法实现电压-温度转换
  * @param[in]  vol lmt70输出电压
  * @retval 电压-温度转换结果放大100倍 -5000到+15000度
  */
int32_t lmt70_get_temp( float vol)
{
    int16_t i ;
    int32_t rev = 0 ;

    i = haltserach(vol) ;

    if ( ( i != -1) && ( i != VOL2TEMP_TABLE_SIZE))
    {
        rev = ( ( ( i * 10) - 50) + ( ( v2t_tabl[i] - vol) / slope_tabl[i])) * 100 ;
    }

    return rev ;
}

/*
 * ADC初始化
 * */
void LMT70_ADC1_CH0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE,GPIO_PIN_2);
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL,1);
    ADCReferenceSet(ADC1_BASE, ADC_REF_INT);
    ADCSequenceConfigure(ADC1_BASE,SequenceNum,ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC1_BASE,SequenceNum,0, ADC_CTL_CH8| ADC_CTL_IE | ADC_CTL_END);
    ADCHardwareOversampleConfigure(ADC1_BASE,64);
    ADCIntEnable(ADC1_BASE, SequenceNum);
    ADCIntRegister(ADC1_BASE, SequenceNum, LMT70_ADC1_CH0_Exe);
    IntEnable(INT_ADC1SS1);
    IntMasterEnable();
    ADCSequenceEnable(ADC1_BASE, SequenceNum);

}


/*
 * 作用：ADC中断服务函数
 * */
void LMT70_ADC1_CH0_Exe(void)
{
    static uint32_t cnt = 0;
    static float LMT70_ADC1_SS0_Volt = 0;
    uint32_t ADC1_SS0_CH0 ;


    ADCIntClear(ADC1_BASE,SequenceNum);
    if(cnt != LMT70_cnt_max - 1)
    {

        ADCSequenceDataGet(ADC1_BASE, SequenceNum,&ADC1_SS0_CH0);
        LMT70_ADC1_SS0_Volt += ((float)ADC1_SS0_CH0*(3300.0/4096.0));
        cnt++;
    }
    if(cnt == LMT70_cnt_max - 1)
    {
        cnt = 0;
        ADCSequenceDataGet(ADC1_BASE, SequenceNum,&ADC1_SS0_CH0);
        LMT70_ADC1_SS0_Volt += ((float)ADC1_SS0_CH0*(3300.0/4096.0));
        output_lmt70 = ((float)lmt70_get_temp(LMT70_ADC1_SS0_Volt/LMT70_cnt_max))/100;
        LMT70_ADC1_SS0_Volt = 0;
//        BlueToothSend_float(&output_lmt70,1);
    }
}

/*
 * 作用：使能ADC数据采集。
 * 说明：采集完后，使能自动关闭，需要再次使用该函数进行使能。
 * */
void LMT_DATA_GET(void)
{
    while(ADCIntStatus(ADC1_BASE, SequenceNum, false))
       {

       }
    ADCProcessorTrigger(ADC1_BASE, SequenceNum);
}
