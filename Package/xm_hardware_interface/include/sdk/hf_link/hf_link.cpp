/***********************************************************************************************************************
* Copyright (c) Hands Free Team. All rights reserved.
* FileName: hf_link.cpp
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
* Description: This file defined hands_free_robot simple communications protocol
*              please read Hands Free Link Manua.doc for detail
***********************************************************************************************************************/

#include "hf_link.h"
#include "stdio.h"
#include <string.h>


namespace xm_hw
{
    namespace internal
    {
        // use for sending message using 485
        void send_signal(uint8_t signal)
        {
            board.rs485_cmd(1);
            
            HF_USART_Put_Char(USART2 , signal);
            
            board.rs485_cmd(0);
        }

        unsigned char hFLinkSendBuffer(unsigned char port_num , unsigned char* buffer, unsigned short int size)
        {
            if(port_num == 1){
                while(size--) 
                    HF_USART_Put_Char(USART1 , *buffer++);
                return 1;
            }
            else if(port_num == 2){
                while(size--) //HF_USART_Put_Char(USART2 , *buffer++);
                    send_signal(*buffer++);//due to we use the 485 communicate for hf_link
                return 1;
            }
            else if(port_num == 3){
                while(size--) 
                    HF_USART_Put_Char(USART3 , *buffer++);
                return 1;
            }
            else if(port_num == 4){
                while(size--) 
                    HF_USART_Put_Char(UART4 , *buffer++);
                return 1;
            }
            return 0;
        }

        // use for testing
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
        
    }

unsigned char HFLink::byteAnalysisCall(const unsigned char rx_byte)
{
    unsigned char package_update=0;
    unsigned char receive_message_update=0;
    receive_message_update=receiveFiniteStates(rx_byte) ;  //Jump communication status

    if( receive_message_update ==1)
    {
        	
		receive_message_update = 0;
        receive_message_count++;
        package_update=packageAnalysis();
        if(package_update==1) receive_package_count++;
					
        return package_update;
    }
	
    return 0;
}

/***********************************************************************************************************************
* Function:     void HFLink::Receive_Analysis(unsigned char rx_data)
*
* Scope:        Analysis the data from hf_link host
*
* Description:
*
* Arguments:
*
* Return:
*
* Cpu_Time:    stm32f4+fpu(1 us)
*
* History:
***********************************************************************************************************************/
unsigned char HFLink::receiveFiniteStates(const unsigned char rx_data)
{

    switch (receive_state_)
    {
    case WAITING_FF1:
        if (rx_data == 0xff)
        {
            receive_state_ = WAITING_FF2;
            receive_check_sum_ =0;
            receive_message_length_ = 0;
            byte_count_=0;
            receive_check_sum_ += rx_data;
        }
        break;

    case WAITING_FF2:
        if (rx_data == 0xff)
        {
            receive_state_ = SENDER_ID;
            receive_check_sum_ += rx_data;
        }
        else
            receive_state_ = WAITING_FF1;
        break;

    case SENDER_ID:
        rx_message_.sender_id = rx_data ;
        if (rx_message_.sender_id == friend_id)  //id check
        {
            receive_check_sum_ += rx_data;
            receive_state_ = RECEIVER_ID;
        }
        else
        {
#if HF_LINK_NODE_MODEL == 0
            send_signal(0xa1);
#endif
            receive_state_ = WAITING_FF1;
        }
        break;

    case RECEIVER_ID:
        rx_message_.receiver_id = rx_data ;
        if (rx_message_.receiver_id == my_id)  //id check
        {
            receive_check_sum_ += rx_data;
            receive_state_ = RECEIVE_LEN_H;
        }
        else
        {
#if HF_LINK_NODE_MODEL == 0
            send_signal(0xa2);
#endif
            receive_state_ = WAITING_FF1;
        }
        break;

    case RECEIVE_LEN_H:
        receive_check_sum_ += rx_data;
        receive_message_length_ |= rx_data<<8;
        receive_state_ = RECEIVE_LEN_L;
        break;

    case RECEIVE_LEN_L:
        receive_check_sum_ += rx_data;
        receive_message_length_ |= rx_data;
        rx_message_.length = receive_message_length_;
        receive_state_ = RECEIVE_PACKAGE;
        break;

    case RECEIVE_PACKAGE:
        receive_check_sum_ += rx_data;
        rx_message_.data[byte_count_++] = rx_data;
        if(byte_count_ >= receive_message_length_)
        {
            receive_state_ = RECEIVE_CHECK;
            receive_check_sum_=receive_check_sum_%255;
        }
        break;

    case RECEIVE_CHECK:
        if(rx_data == (unsigned char)receive_check_sum_)
        {
            receive_check_sum_=0;
            receive_state_ = WAITING_FF1;
//#if HF_LINK_NODE_MODEL == 0
//            send_signal(0xa0);
//#endif
            return 1 ;
        }
        else
        {
#if HF_LINK_NODE_MODEL == 0
            send_signal(0xa3);
#endif
            receive_state_ = WAITING_FF1;
        }
        break;
    default:
        receive_state_ = WAITING_FF1;
    }

    return 0;
}

/***********************************************************************************************************************
* Function:     void HFLink::packageAnalysis_3WD(void)
*
* Scope:
*
* Description:
*
* Arguments:
*
* Return:
*
* Cpu_Time:    stm32f4+fpu(1 us)
*
* History:
***********************************************************************************************************************/
unsigned char HFLink::packageAnalysis(void)
{
    unsigned char analysis_state=0;
    void* single_command;

    command_state_ = (Command)rx_message_.data[0];

    if (hf_link_node_model== 0)  //the slave need to check the SHAKING_HANDS"s state
    {
        if(shaking_hands_state==0 && command_state_ != SHAKING_HANDS) //if not  shaking hands
        {
            sendStruct(SHAKING_HANDS  , (unsigned char *)single_command, 0);
			
            return 1;
        }
    }

    switch (command_state_)
    {
    case SHAKING_HANDS :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_global_coordinate , sizeof(my_robot->measure_global_coordinate));
        break;
		
    case SET_ROBOT_SPEED :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->expect_robot_speed , sizeof(my_robot->expect_robot_speed));
        break;

    case READ_ROBOT_SPEED :
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_robot_speed , sizeof(my_robot->measure_robot_speed));
        break;


    case READ_GLOBAL_COORDINATE ://9
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_global_coordinate , sizeof(my_robot->measure_global_coordinate));
        break;

    case READ_ROBOT_COORDINATE :
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_robot_coordinate , sizeof(my_robot->measure_robot_coordinate));
        break;

    case CLEAR_COORDINATE_DATA :
        if (hf_link_node_model==0)
        {
            sendStruct(command_state_ , (unsigned char *)single_command , 0);
            analysis_state=1;
            receive_package_renew[(unsigned char)command_state_] = 1 ;
        }
        break;

    
    case SET_HEAD_1 :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->expect_head1_state , sizeof(my_robot->expect_head1_state ));
        break;

    case READ_HEAD_1 :
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_head1_state , sizeof(my_robot->measure_head1_state));
        break;

    case SET_HEAD_2 :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->expect_head2_state, sizeof(my_robot->expect_head2_state));
        break;

    case READ_HEAD_2 :
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_head2_state , sizeof(my_robot->measure_head2_state));
        break;
			 
		case SET_CLAW :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->claw_state_, sizeof(my_robot->claw_state_));
        break;
				
		case SET_ARM_TOTAL :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->expect_arm_state, sizeof(my_robot->expect_arm_state));
        break;
				 
		case READ_ARM_TOTAL :
        analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->measure_arm_state , sizeof(my_robot->measure_arm_state));
        break;
				 
		case PLAT_MOVE :
        analysis_state=setCommandAnalysis(command_state_ , (unsigned char *)&my_robot->plat_height, sizeof(my_robot->plat_height));
        break;
		case READ_PLAT :
				analysis_state=readCommandAnalysis(command_state_ , (unsigned char *)&my_robot->plat_height_back, sizeof(my_robot->plat_height_back));
        break;
    default :
        analysis_state=0;
        break;

    }

    rx_message_.sender_id=0;    //clear flag
    rx_message_.receiver_id=0;
    rx_message_.length=0;
    rx_message_.data[0]=0;

    return analysis_state;
}

unsigned char HFLink::readCommandAnalysis(const Command command_state , unsigned char* p , const unsigned short int len)
{    
    if (hf_link_node_model==1)
    { // master   , means the slave feedback a package to master , and the master save this package
        if((rx_message_.length-1) != len)
        {
#if HF_LINK_NODE_MODEL == 1
            //printf("I'm a master , can not read the message from slave , the length is not mathcing to struct \n");
#endif
            return 0;
        }
        memcpy(p, &rx_message_.data[1] , len);
        receive_package_renew[(unsigned char)command_state] = 1 ;
    }
    else if(hf_link_node_model==0)
    { // slave   , means the master pub a read command to slave ,and the slave feedback the a specific info to him
        sendStruct(command_state  , p , len);

        receive_package_renew[(unsigned char)command_state] = 1 ;
    }
    return 1;
}

unsigned char HFLink::setCommandAnalysis(const Command command_state , unsigned char* p , const unsigned short int len)
{
    void* ack;
    if (hf_link_node_model==1)
    { // master  , the slave can set the master's data ,so this code means received the slave's ack
        if(command_state==SHAKING_HANDS)
        {
            shaking_hands_state=1;  //wait he master send SHAKING_HANDS

        }
        else
        {   
        }
        receive_package_renew[(unsigned char)command_state] = 1 ;
    }
    else if(hf_link_node_model==0)
    { // slave  , means the master pub a set command to slave ,and the slave save this package then feed back a ack
        if((rx_message_.length-1) != len)
        {
#if HF_LINK_NODE_MODEL == 0
						send_signal(len);
            send_signal(0xb1);//��ӡ���󳤶�
#endif
            return 0;
        }
        memcpy(p , &rx_message_.data[1] , len);
        if(command_state==SHAKING_HANDS)
			{
				shaking_hands_state=1;   //SHAKING_HANDS not need ack to master
				get_master();
			}
		else sendStruct(command_state  , (unsigned char *)ack , 0); //ack , tell the master , i receive your set package
        receive_package_renew[(unsigned char)command_state] = 1 ;   //update receive flag , and wait the cpu to deal
    }
    return 1;
}

/***********************************************************************************************************************
* Function:    void HFLink::sendCommand(Command command)
*
* Scope:       public
*
* Description: send a command or data to the friend_id
*              this function is olny belongs to master
*
*
* Arguments:
*
* Return:
*
* Cpu_Time:
*
* History:
***********************************************************************************************************************/

unsigned char HFLink::masterSendCommand(const Command command_state)
{

    //else  means master send a command to slave
    unsigned char analysis_state =1;
    void* single_command;
    if(hf_link_node_model != 1){return 0;}
    
    switch (command_state)
    {
    case SHAKING_HANDS :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        shaking_hands_state = 1;
        sendStruct(command_state , (unsigned char *)&my_robot->measure_global_coordinate , sizeof(my_robot->measure_global_coordinate));
        break;
 
    case SET_ROBOT_SPEED :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->expect_robot_speed , sizeof(my_robot->expect_robot_speed));
        break;
 
    case READ_ROBOT_SPEED :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;
 
    case READ_GLOBAL_COORDINATE :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;
 
    case READ_ROBOT_COORDINATE :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;
 
    case CLEAR_COORDINATE_DATA :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;
 
    case SET_HEAD_1 :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->expect_head1_state , sizeof(my_robot->expect_head1_state ));
        break;
 
    case READ_HEAD_1 :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;
 
    case SET_HEAD_2 :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->expect_head2_state, sizeof(my_robot->expect_head2_state));
        break;
 
    case READ_HEAD_2 :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)single_command , 0);
        break;

    case SET_CLAW :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->claw_state_, sizeof(my_robot->claw_state_));
        break;

    case SET_ARM_TOTAL :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->expect_arm_state, sizeof(my_robot->expect_arm_state));
        break;
                 
    case READ_ARM_TOTAL :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state , (unsigned char *)&my_robot->measure_arm_state , sizeof(my_robot->measure_arm_state));
        break;
                 
    case PLAT_MOVE :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state ,  (unsigned char *)&my_robot->plat_height, sizeof(my_robot->plat_height));      
        break;

    case READ_PLAT :
        receive_package_renew[(unsigned char)command_state] = 0 ;
        sendStruct(command_state ,  (unsigned char *)&my_robot->plat_height_back, sizeof(my_robot->plat_height_back));      
        break;

    default :
        analysis_state=0;
        break;

    }

    return analysis_state;
}

/***********************************************************************************************************************
* Function:    void HFLink::sendStruct(const Command command_state , unsigned char* p , const unsigned short int len)
*
* Scope:       private
*
* Description:
* len =0       send a Single command to the friend
*              if i am slave , it can be  feed back a ack to master or request instructions  like SHAKING_HANDS
*              if i am master , it can be some request instructions like READ_ROBOT_SYSTEM_INFO READ_xxx
*
*
* len>0 :      send a Struct command to the friend hf_link nodeif
*              if i am slave , then means feed back  a  struc(valid data) to master
*              if i am master , then means set a a  struc(valid data)to slave
* Arguments:
*
* Return:
*
* Cpu_Time:
*
* History:
***********************************************************************************************************************/
void HFLink::sendStruct(const Command command_state , unsigned char* p , const unsigned short int len)
{
    tx_message_.sender_id = my_id;
    tx_message_.receiver_id = friend_id;
    tx_message_.length=len+1;
    tx_message_.data[0] = (unsigned char)command_state;
    if (len == 0)
    {

    }
    else if(len > 0)
    {
        memcpy(&tx_message_.data[1] , p , len);
    }
    sendMessage();
}

/***********************************************************************************************************************
* Function:    void HF_Link::sendMessage(void)
*
* Scope:
*
* Description:  send a message to hf_link node
*
* Arguments:
*
* Return:
*
* Cpu_Time:    stm32f4+fpu(1 us)
*
* History:
***********************************************************************************************************************/
void HFLink::sendMessage(void)
{
    unsigned short int tx_i;
    unsigned int check_sum_=0;

    tx_buffer[0]=0xff;
    check_sum_ += 0xff;

    tx_buffer[1]=0xff;
    check_sum_ += 0xff;

    tx_buffer[2]=tx_message_.sender_id;
    check_sum_ += tx_buffer[2];

    tx_buffer[3]=tx_message_.receiver_id;
    check_sum_ += tx_buffer[3];

    tx_buffer[4]=(unsigned char)( tx_message_.length >> 8); //LEN_H
    check_sum_ += tx_buffer[4];

    tx_buffer[5]=(unsigned char)tx_message_.length;   //LEN_L
    check_sum_ += tx_buffer[5];

    for(tx_i=0; tx_i < tx_message_.length ; tx_i++)   //package
    {
        tx_buffer[6+tx_i]=tx_message_.data[tx_i];
        check_sum_ += tx_buffer[6+tx_i];
    }
    check_sum_=check_sum_%255;
    tx_buffer[6+tx_i] = check_sum_;

    tx_buffer_length = 7 + tx_i;

	
	if(communication_mode==0)
	{
		port_num=2;
	}
	else if(communication_mode==1)
	{
		port_num=4;
	}
#if HF_LINK_NODE_MODEL==0
    if(hf_link_node_model==0){
        hFLinkSendBuffer(port_num , tx_buffer , tx_buffer_length);
    }
#endif

    send_packag_count++;
}

}//use namespace xm_hw

