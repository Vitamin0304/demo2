/*
 * LMT70.c
 *
 *  Created on: 2020��10��7��
 *      Author: ����
 */

//LMT70���ݲɼ���������
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

/* ��ѹ�¶�ת���ߣ���ѹΪ��v���¶ȴ�-50�ȿ�ʼ������Ϊ10�ȣ���������
 * ����vol2temp_table[0] ��ʾ-50�ȣ�vol2temp_table[1]�ͱ�ʾ-40�ȡ�
 * �˴���û��ɶ�Ӧadcת�������������������Ϳ��Ա���adc��������
 * ��ѹ���µ�������ͬʱҲ����������ٶȡ�
 */
float v2t_tabl[VOL2TEMP_TABLE_SIZE] = {
                                1350.441,1300.593,1250.398,1199.884,1149.070,
                                1097.987,1046.647, 995.050, 943.227, 891.178,
                                 838.882, 786.360, 733.608, 680.654, 627.490,
                                 574.117, 520.551, 466.760, 412.739, 358.164,
                                 302.785
} ;

/* б�ʱ���ʮ��������ʹ�����Բ�ֵ�� */
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

/* �۰���ʵ�� */
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

    /* �������Ա������-1��RT2TEMP_TABLE_SIZE��ʾ���ڱ��� */
    /* ����ֵ����ȡ�±߽��¶Ȳ����� */

    return up ;
}

/** @brief  ��ʼ��lmt70
  * @details �����lmt70��صĵײ�����ĳ�ʼ��������adc��gpio��
  * @retval NONE
  */

/** @brief  ��lmt70��ȡ�¶�����
  * @details ʹ��һ�׵������ʵ�ֵ�ѹ-�¶�ת��
  * @param[in]  vol lmt70�����ѹ
  * @retval ��ѹ-�¶�ת������Ŵ�100�� -5000��+15000��
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
 * ADC��ʼ��
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
 * ���ã�ADC�жϷ�����
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
 * ���ã�ʹ��ADC���ݲɼ���
 * ˵�����ɼ����ʹ���Զ��رգ���Ҫ�ٴ�ʹ�øú�������ʹ�ܡ�
 * */
void LMT_DATA_GET(void)
{
    while(ADCIntStatus(ADC1_BASE, SequenceNum, false))
       {

       }
    ADCProcessorTrigger(ADC1_BASE, SequenceNum);
}
