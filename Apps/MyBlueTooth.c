/*
 * MyBlueTooth.c
 *
 *  Created on: 2020年9月20日
 *      Author: STARSKY
 * 使用步骤：
 * 1.使用前自行对Uart0初始化，用于与计算机的交互;
 * 2.在头文件修改Uart的宏定义以设置要使用的Uart串口(1或3)
 * Uart1: PB0(RXD)/PB1(TXD)
 * Uart3: PC6(RXD)/PC7(TXD)
 * 如果需要使用某个串口的另一对引脚，可在本文件中查询“串口初始化”关键字进行修改
 * 3.注册中断，在本文件中查询关键字“串口中断服务函数”对中断的功能实现进行编程
 * 4.如有需要，使用AT指令配置蓝牙名字、波特率、恢复出厂设置等
 * 5.蓝牙模块初始化配置
 * 6.使用蓝牙模块功能函数
 *
 */
#include <string.h>
#include <math.h>
#include "Apps/CarController.h"
#include "Apps/MyBlueTooth.h"
#include "Apps/cJson.h"
void Bluetooth_Test(void)
{
    uint8_t command[2] = "AT";
    BlueToothputs(command, 2);
}

void Bluetooth_Name_Get(void)
{
    uint8_t command[8] = "AT+NAME ";
    BlueToothputs(command, 7);
}

void Bluetooth_Name_Set(uint8_t *name)
{
    BlueToothputs("AT+NAME", 7);
    BlueToothputs(name, strlen((char*)name));
}

void Bluetooth_Addr_Get(void)
{
    BlueToothputs("AT+LADDR", 8);
}

void Bluetooth_Addr_Set(uint8_t *addr)
{
    //UARTprintf("%d",strlen("AT+LADDRaa:bb:cc:dd:ee:ff"));
    BlueToothputs("AT+LADDR98:da:09:60:04:ca",25);
    //BlueToothputs(addr, strlen((char*)addr));
}

void Bluetooth_Baud_Get(void)
{
    BlueToothputs("AT+BAUD",7);
}

void Bluetooth_Baud_Set(uint32_t baud_data)
{
    uint8_t baud[1] = {0};
    baud_data = baud_data/1200;
    if(baud_data <= 32)
    {
        for(uint8_t i = 1;i < 7;i++)
        {
            if(baud_data == pow(2,i - 1))
            {
                baud[0] = 0x30 + i;
                break;
            }
        }
    }
    else
    {
        switch(baud_data)
        {
        case 48:
            baud[0] = 0x30 + 7;
            break;
        case 96:
            baud[0] = 0x30 + 8;
            break;
        case 192:
            baud[0] = 0x30 + 9;
            break;
        }
    }

    BlueToothputs("AT+BAUD",7);
    BlueToothputs(baud,1);
}


void Bluetooth_Reset(void)
{
    BlueToothputs("AT+DEFAULT",10);
}


uint32_t BlueTooth_Init(uint32_t Baud_Data,uint32_t BlueTooBaud_Datanow)
{
    FPUEnable();
    FPULazyStackingEnable();
    if(Uart_BlueTooth == UART3_BASE)
    {
        _Uart3_Init(BlueTooBaud_Datanow);
        UARTFIFODisable(UART3_BASE);
        _Uart3_int_Init();
        Bluetooth_Baud_Set(Baud_Data);
        delay_1ms(1000);
        Bluetooth_Baud_Get();
        delay_1ms(1000);
        _Uart3_Init(Baud_Data);
        UARTFIFODisable(UART3_BASE);
        _Uart3_int_Init();
    }
    else if(Uart_BlueTooth == UART1_BASE)
    {
        _Uart1_Init(BlueTooBaud_Datanow);
        UARTFIFODisable(UART1_BASE);
        _Uart1_int_Init();
        Bluetooth_Baud_Set(Baud_Data);
        delay_1ms(1000);
        Bluetooth_Baud_Get();
        delay_1ms(1000);
        _Uart1_Init(Baud_Data);
        UARTFIFODisable(UART1_BASE);
        _Uart1_int_Init();
    }
    else
    {
        UARTprintf("BlueTooth Initial has Failed!");
        return 1;
    }
    return 0;
}









//void BlueToothputs(uint8_t* str, uint32_t len)
//{
//    for (int i = 0; i < len; ++i)
//    {
//        UARTCharPut(Uart_BlueTooth, str[i]);
//    }
//}

//void BlueToothSendGraph(float* pomega,float* poutput)
//{
//    uint8_t sendBuf[11];
//    uint8_t index = 0;
//    uint8_t *pdata = (uint8_t*)pomega;
//    uint8_t sumData = 0;
//
//    sendBuf[index++] = 0xA5;
//    for(uint8_t i = 0; i < 4; ++i, ++pdata)
//    {
//        sendBuf[index++] = *pdata;
//        sumData += *pdata;
//    }
//    pdata = (uint8_t*)poutput;
//    for(uint8_t i = 0; i < 4; ++i, ++pdata)
//    {
//        sendBuf[index++] = *pdata;
//        sumData += *pdata;
//    }
////    pdata = (uint8_t*)pdistance;
////    for(uint8_t i = 0; i < 4; ++i, ++pdata)
////    {
////        sendBuf[index++] = *pdata;
////        sumData += *pdata;
////    }
//    sendBuf[index++] = sumData;
//    sendBuf[index++] = 0x5A;
//
//    BlueToothputs(sendBuf,11);
//}

//void BlueToothSend(float* pomega,uint32_t datalen)
//{
//    uint8_t *p ;
//    p = (uint8_t *)malloc((datalen*4+3)*sizeof(uint8_t));
////    uint8_t p[datalen*4+3];
//    uint8_t index = 0;
//    uint8_t *pdata = (uint8_t*)pomega;
//    uint8_t sumData = 0;
//
//    p[index++] = 0xA5;
//
//    for(uint8_t i = 0; i < datalen ; ++i)
//    {
//        pdata = (uint8_t*)pomega+4 * i;
//        for(uint8_t j = 0; j < 4; ++j, ++pdata)
//            {
//                p[index++] = *pdata;
//                sumData += *pdata;
//            }
//
//    }
//
//    p[index++] = sumData;
//    p[index++] = 0x5A;
//
//    BlueToothputs(p,datalen*4+3);
//}

uint32_t BlueToothSend_float(float* data,uint32_t datalen)
{
    if(datalen >= 7||datalen<=0)
    {
        UARTprintf("Illigal Data!!!");
        return 1;
    }


    uint8_t DataSend[27];
    uint8_t *pdata = (uint8_t*)data;
    uint8_t sumData = 0;

    DataSend[0] = 0xA5;

    for(uint8_t i = 0; i < datalen ; ++i)
    {
        pdata = (uint8_t*)data+4 * i;
        for(uint8_t j = 0; j < 4; ++j, ++pdata)
            {
                DataSend[1+j+i*4] = *pdata;
                sumData     += *pdata;
            }

    }
    switch(datalen)
    {

    case 1:
        DataSend[5] = sumData;
        DataSend[6] = 0x5A;
        break;
    case 2:
        DataSend[9] = sumData;
        DataSend[10] = 0x5A;
        break;
    case 3:
        DataSend[13] = sumData;
        DataSend[14] = 0x5A;
        break;
    case 4:
        DataSend[17] = sumData;
        DataSend[18] = 0x5A;
        break;
    case 5:
        DataSend[21] = sumData;
        DataSend[22] = 0x5A;
        break;
    case 6:
        DataSend[25] = sumData;
        DataSend[26] = 0x5A;
        break;
    }


    BlueToothputs(DataSend,datalen*4+3);
    return 0;
}

/*串口初始化**********************************************************/
void _Uart1_Init(uint32_t Baud_Data)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1);
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(),Baud_Data, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}
void _Uart3_Init(uint32_t Baud_Data)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
    GPIOPinConfigure(GPIO_PC6_U3RX);
    GPIOPinConfigure(GPIO_PC7_U3TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7);

    UARTConfigSetExpClk(UART3_BASE, SysCtlClockGet(),Baud_Data, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
//    UARTFIFODisable(UART3_BASE);
//    UARTFIFODisable(UART1_BASE);
    UARTFIFOEnable(UART3_BASE);//先使能
//    UARTFIFOLevelSet(UART3_BASE, UART_FIFO_TX7_8, UART_FIFO_RX7_8);//在打开2/8也就是1/4的深度
}
/*串口中断配置**********************************************************/
void _Uart1_int_Init(void)
{
    UARTIntEnable(UART1_BASE, UART_INT_RX|UART_INT_TX);
    IntEnable(INT_UART1);
    IntMasterEnable();
}

void _Uart3_int_Init(void)
{
    UARTIntEnable(UART3_BASE, UART_INT_RX);
    UARTIntRegister(UART3_BASE, _Uart3_int_exe);
    IntEnable(INT_UART3);
    IntMasterEnable();
}
/*串口中断服务函数**********************************************************/
void _Uart1_int_exe(void)
{
//    static uint32_t flag = 0;
    uint32_t status = UARTIntStatus(UART1_BASE, true);
    uint32_t data = 0;
    UARTIntClear(UART1_BASE, status);
    while(UARTCharsAvail(UART1_BASE))
    {

        data = UARTCharGetNonBlocking(UART1_BASE);
        UARTprintf("%c",(uint8_t)data);

    }

}
//OPENMV命令模式
//void _Uart3_int_exe(void)
//{
//     static uint8_t stop_flag = 0;
//     static uint32_t flag = 0;
//     static uint8_t command_flag = 0;
//     uint32_t status = UARTIntStatus(UART3_BASE, true);
//     uint32_t data = 0;
//     UARTIntClear(UART3_BASE, status);
//     while(UARTCharsAvail(UART3_BASE))
//     {
//
//         data = UARTCharGetNonBlocking(UART3_BASE);
//         switch(data)
//         {
//         case 0x00:
//
//             break;
//         case 0x01:
//             if(stop_flag && command_flag)
//             {
//                 stop_flag = 0;
//                 command_flag = 0;
//                 for(uint32_t i = 0;i < 5;i++)
//                 {
//                     GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_PIN_4);
//                     delay_1ms(20);
//                     GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, 0x00);
//                     delay_1ms(20);
//                 }
//
//             }
//             else if(command_flag == 0)
//            {
//                 GPIOF_LED_TOGGLE();
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_PIN_4);
//                 delay_1ms(5);
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, 0x00);
//                 delay_1ms(5);
//                 stop_flag = 1;
//            }
//
//
//             break;
//         case 0x02:
//             if(command_flag == 0)
//             {
//                 GPIOF_LED_TOGGLE();
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_PIN_4);
//                 delay_1ms(5);
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, 0x00);
//                 delay_1ms(5);
//                 UARTCharPut(UART3_BASE,'b');
//                 command_flag = 1;
//                 stop_flag = 1;
//
//             }
//             break;
//         case 0x03:
//             UARTCharPut(UART3_BASE,'c');
//             break;
//         case 0x04:
//             if(command_flag == 0)
//             {
//                 GPIOF_LED_TOGGLE();
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_PIN_4);
//                 delay_1ms(5);
//                 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, 0x00);
//                 delay_1ms(5);
//                 UARTCharPut(UART3_BASE,'d');
//                 command_flag = 1;
//                 stop_flag = 1;
//
//             }
//             break;
//         default:
//             UARTprintf("%c",data);
//         }
//         //UARTprintf("%c",(uint8_t)data);
//
//     }
//
//}


////红外测温模式
//void _Uart3_int_exe(void)
//{
//
//     static uint32_t flag = 0;
//     uint32_t status = UARTIntStatus(UART3_BASE, true);
//     uint32_t data = 0;
//     UARTIntClear(UART3_BASE, status);
//     while(UARTCharsAvail(UART3_BASE))
//     {
////
//         data = UARTCharGetNonBlocking(UART3_BASE);
//         UARTprintf("%c",(uint8_t)data);
////         switch(data)
////         {
////         case 0x00:
////             MCU90615_DataGet();
////             break;
////         case Int_Trigger://0x01
////             GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, GPIO_PIN_4);
////             delay_1ms(5);
////             GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4, 0x00);
////             delay_1ms(5);
////
////             break;
////         case 0x02:
////             UARTCharPut(UART4_BASE,'b');
////             break;
////         case 0x03:
////             UARTCharPut(UART4_BASE,'c');
////             break;
////         default:
////             UARTprintf("%c",data);
////         }
//
////
//     }
//
//}

uint8_t uart3RxBuff[UART3_RX_BUFF_SIZE];

uint32_t uart3RxLength = 0;
/*
 * 作用：蓝牙获取Openmv识别目标相对摄像头的坐标
 * */
void _Uart3_int_exe(void)
{
     uint32_t status = UARTIntStatus(UART3_BASE, true);
     uint8_t rxChar;
     UARTIntClear(UART3_BASE, status);
//     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2|GPIO_PIN_3);
     while(UARTCharsAvail(UART3_BASE))
     {
         rxChar = UARTCharGetNonBlocking(UART3_BASE);

         if(uart3RxLength == 0)
         {
             if(rxChar == '{')
             {
                 uart3RxBuff[uart3RxLength++] = rxChar;
             }
         }
         else
             uart3RxBuff[uart3RxLength++] = rxChar;

//         UARTprint f("%c",rxChar);

         if(uart3RxLength != 0 && rxChar == '}')
         {
             uart3RxLength = 0;
             if(uart3RxBuff[0] == '{')
             {
                 cJSON* root = cJSON_Parse(uart3RxBuff);
                 if (!root)
                 {
//                     UARTprintf("Error before: [%s]\n",cJSON_GetErrorPtr());
                 }
                 else
                 {
                     image_seek_line_actual[0] = cJSON_GetObjectItem(root, "rho_err")->valuedouble;
                     image_seek_line_actual[1] = cJSON_GetObjectItem(root, "theta_err")->valuedouble;
                     car_stop_flag += cJSON_GetObjectItem(root, "car_StopFlag")->valueint;
//                     BlueToothSendGraph(&image_seek_line_actual[0],&image_seek_line_actual[1],&image_seek_line_output);
                     cJSON_Delete(root);
                 }
             }
         }
     }
}





