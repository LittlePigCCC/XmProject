/***********************************************************************************************************************
* Copyright (c) Hands Free Team. All rights reserved.
* FileName: can.c
* Contact:  QQ Exchange Group -- 521037187
* Version:  V2.0
*
* LICENSING TERMS:
* The Hands Free is licensed generally under a permissive 3-clause BSD license.
* Contributions are requiredto be made under the same license.
*
* History: 
* <author>      <time>      <version>      <desc>
* mawenke       2015.10.1   V1.0           creat this file
*
* Description: 
***********************************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif 

#include "can.h"
#include "nvic.h"
#include "delay.h"

/***********************************************************************************************************************
* Function:     void HF_CAN_Init(CAN_TypeDef *CANx , uint8_t GPIO_AF)
*
* Scope:        public
*
* Description:  CAN initialization
*
* Arguments:
*
* Return:
*
* Cpu_Time:  
*
* History:
***********************************************************************************************************************/
void HF_CAN_Init(CAN_TypeDef *CANx , uint8_t GPIO_AF)
{
    
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //Multiplexing
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    if(CANx == CAN1){
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
        
        if(GPIO_AF == 0){  // remap gpio
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11| GPIO_Pin_12;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            CAN1_RX0_NVIC_Configuration();
        }
        else if(GPIO_AF == 1){
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8| GPIO_Pin_9;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            CAN1_RX0_NVIC_Configuration();
        }
        else if(GPIO_AF == 2){
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
            GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            CAN1_RX1_NVIC_Configuration();
        }
        
    }
    else if(CANx == CAN2){
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
        
        if(GPIO_AF == 0){
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource5,GPIO_AF_CAN2);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            CAN2_RX0_NVIC_Configuration();
        }
        else if(GPIO_AF == 1){
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
            GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12| GPIO_Pin_13;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            CAN2_RX1_NVIC_Configuration();
        }   
    }
    
    CAN_DeInit(CANx);
    CAN_StructInit(&CAN_InitStructure);
    CAN_InitStructure.CAN_TTCM = DISABLE;			  //MCR-TTCM  enable time touch communicate mode
    CAN_InitStructure.CAN_ABOM = DISABLE;			  //MCR-ABOM  auto manage offline
    CAN_InitStructure.CAN_AWUM = DISABLE;			  //MCR-AWUM  auto wake-up
    CAN_InitStructure.CAN_NART = ENABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	  //BTR-SILM/LBKM   CAN normal communicate mode
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS2_6tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS1_7tq;
    CAN_InitStructure.CAN_Prescaler =3;               //CAN BaudRate 42/(1+6+7)/3=1Mbps
    CAN_Init(CANx, &CAN_InitStructure);
    
    if(CANx==CAN2){CAN_FilterInitStructure.CAN_FilterNumber=14;}
		else {CAN_FilterInitStructure.CAN_FilterNumber=0;}
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;////32位ID
    CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);  // interrupt enable
   // CAN_ITConfig(CANx, CAN_IT_TME, ENABLE);
    
}	

/***********************************************************************************************************************
* Function:     void Can_Tx_Message(void)
*
* Scope:        public
*
* Description:  CAN Send one Message
*
* Arguments:
*
* Return:
*
* Cpu_Time:  
*
* History:
***********************************************************************************************************************/
void HF_CANTX_Message(CAN_TypeDef* CANx , unsigned int ID , uint8_t *TxBuf)
{   
    CanTxMsg TxMessageBuffer;
    uint8_t i;
		uint8_t Length=8;
    TxMessageBuffer.StdId=ID;
	//TxMessageBuffer.ExtId=ID; //Sender ID | Receiver ID
    TxMessageBuffer.RTR=0;
    TxMessageBuffer.IDE=0;
    TxMessageBuffer.DLC=Length;
    
    for(i=0;i<Length;i++)
    {
        TxMessageBuffer.Data[i]=*(TxBuf+i);
    }
    
    CAN_Transmit(CANx, &TxMessageBuffer);  
		//delay_ms(500);
}

#ifdef __cplusplus
}
#endif

