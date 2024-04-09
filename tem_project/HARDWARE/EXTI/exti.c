#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "beep.h"
#include "includes.h"
#include "task.h"


//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	OS_ERR err;
	OSIntEnter();
	OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
	if(KEY_UP==1)	//WK_UP按键
	{		
		if(isDormancy!=1)
			suspend_task();
		else
			recovery_task();
	}		 
	EXTI->PR=1<<0;  //清除LINE0上的中断标志位  
	OSIntExit();
}
		   

//初始化PA0中断输入.
void EXTIX_Init(void)
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,RTIR); 	//上升沿触发
	MY_NVIC_Init(0,0,EXTI0_IRQn,1);	//抢占0，子优先级0，组1 
}












