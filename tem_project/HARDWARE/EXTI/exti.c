#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "beep.h"
#include "includes.h"
#include "task.h"


//�ⲿ�ж�0�������
void EXTI0_IRQHandler(void)
{
	OS_ERR err;
	OSIntEnter();
	OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
	if(KEY_UP==1)	//WK_UP����
	{		
		if(isDormancy!=1)
			suspend_task();
		else
			recovery_task();
	}		 
	EXTI->PR=1<<0;  //���LINE0�ϵ��жϱ�־λ  
	OSIntExit();
}
		   

//��ʼ��PA0�ж�����.
void EXTIX_Init(void)
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,RTIR); 	//�����ش���
	MY_NVIC_Init(0,0,EXTI0_IRQn,1);	//��ռ0�������ȼ�0����1 
}












