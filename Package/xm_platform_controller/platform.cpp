#include "platform.h"

PLATFORM platform;

void PLATFORM::platform_top_call(void)
{
	if(plat_state==1){
		plat_state=0;
		HF_PWM_Set_State(TIM3,1);//DISABLE
		Pulse_Num=0;
	if(dest_height>current_h)
	{
		board.Foreward_Move();
		flag_dir=0xff;   ///�����ƶ�����־λΪ0xff��
	}
	else
	{
		board.Backward_Move();
		flag_dir=0x01;    ///�����ƶ�����־λΪ0x01
	}
		
	HF_PWM_Set_State(TIM3,0);//ENSABLE   
	Desp_Num=(int)(fabs(dest_height-current_h)/per_s*per_pulse);     //����õ�Ŀ������ĸ�����������
		}
	

}
