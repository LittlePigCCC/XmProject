#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif
#include "robot_abstract.h"	
#include "board.h"	
#include "math.h"
	
class PLATFORM
{
	public:
		PLATFORM(){
			flag_dir 			=0xff;
			current_h			=0; 
			Pulse_Num 		=0 ;
			Desp_Num  		=0;
			Pulse_counter	=0;
			per_pulse			=1600;
			per_s					=0.505f;
			dest_height   =0;
			plat_state=0;
		}
		void platform_top_call(void);
		void set_per_pulse(uint16_t Pulse_counter_){Pulse_counter = Pulse_counter_;}
		void set_per_s(float per_s_){per_s = per_s_;}
		void set_height(float height_){plat_state=1;dest_height= height_*100;}
		uint8_t flag_dir ;///����̨���������־λ�����ڼ�¼�ܵ���������0xff������̨���ϱ�־��0x01������̨���±�־λ
		float current_h; //��ǰ�߶�
		uint16_t Pulse_Num;    	//Pulse_Num�洢��¼���������
		uint16_t Desp_Num;     	//Desp_Num�洢Ŀ��������
		int Pulse_counter;	//Pulse_counter�洢�ܵ�������(���ڼ���߶ȣ�
		uint16_t per_pulse;    //�������תһȦ������
		float per_s;   //ת��һȦ�ƶ�����2.0cm��(����2.00cm)
		float dest_height;
		
		
	private:
		uint8_t plat_state;
};

extern PLATFORM platform;
#ifdef __cplusplus
}
#endif

#endif
