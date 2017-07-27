/***********************************************************************************************************************
* Copyright (c) Hands Free Team. All rights reserved.
* FileName: hf_link_port.cpp
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
* Description: through this file , you could transplanted hf link easily
***********************************************************************************************************************/

#include "board.h"
#include "hf_link_port.h"

unsigned char hFLinkSendBuffer(unsigned char port_num , unsigned char* buffer, unsigned short int size)
{
    if(port_num == 1){
        while(size--) HF_USART_Put_Char(USART1 , *buffer++);
        return 1;
    }
    else if(port_num == 2){
        while(size--) //HF_USART_Put_Char(USART2 , *buffer++);
				send_signal(*buffer++);
        return 1;
    }
    else if(port_num == 3){
        while(size--) HF_USART_Put_Char(USART3 , *buffer++);
        return 1;
    }
    else if(port_num == 4){
        while(size--) HF_USART_Put_Char(UART4 , *buffer++);
        return 1;
    }
    return 0;
}
void get_master(void)
{
	board.rs485_cmd(1);
	char message[]="i catch the master";
	char *p= message;
	while(*(p)){
	HF_USART_Put_Char(USART2 , *p);
	HF_USART_Put_Char(UART4 , *p);
	p++;
	
	}
	board.rs485_cmd(0);
}
void send_signal(uint8_t signal)
{
	board.rs485_cmd(1);
	
	HF_USART_Put_Char(USART2 , signal);
	
	board.rs485_cmd(0);
}


