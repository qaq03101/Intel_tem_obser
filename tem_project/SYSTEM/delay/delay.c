#include "delay.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"					  
#endif


static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,
	
	
#if SYSTEM_SUPPORT_OS							


//֧��UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD						
#define delay_osrunning		OSRunning			
#define delay_ostickspersec	OSCfg_TickRate_Hz	
#define delay_osintnesting 	OSIntNestingCtr		
#endif


//us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   			
	OS_ERR err; 
	OSSchedLock(&err);						
#else										
	OSSchedLock();							
#endif
}

//us����ʱʱ,�ָ��������
void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD   			
	OS_ERR err; 
	OSSchedUnlock(&err);					
#else										
	OSSchedUnlock();						
#endif
}

//����OS�Դ�����ʱ������ʱ
//ticks:��ʱ�Ľ�����
void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err);
#else
	OSTimeDly(ticks);						
#endif 
}
 

void SysTick_Handler(void)
{	
	if(delay_osrunning==1)					
	{
		OSIntEnter();						
		OSTimeTick();       				             
		OSIntExit();       	 				
	}
}
#endif
			   
//��ʼ���ӳٺ���
//SYSCLK:ϵͳʱ��
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS 						
	u32 reload;
#endif
 	SysTick->CTRL&=~(1<<2);					
	fac_us=SYSCLK/8;						
#if SYSTEM_SUPPORT_OS 						
	reload=SYSCLK/8;							   
	reload*=1000000/delay_ostickspersec;	
											//reloadΪ24λ�Ĵ���,���ֵ:16777216,��72M��,Լ��1.86s����	
	fac_ms=1000/delay_ostickspersec;			   
	SysTick->CTRL|=1<<1;   					//����SYSTICK�ж�
	SysTick->LOAD=reload; 					//ÿ1/delay_ostickspersec���ж�һ��	
	SysTick->CTRL|=1<<0;   					//����SYSTICK    
#else
	fac_ms=(u16)fac_us*1000;				  
#endif
}								    

#if SYSTEM_SUPPORT_OS 						
//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*fac_us; 						//��Ҫ�Ľ����� 
	delay_osschedlock();					
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	};
	delay_osschedunlock();												    
}
//��ʱnms
//nms:Ҫ��ʱ��ms��
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)	    
	{		 
		if(nms>=fac_ms)						
		{ 
   			delay_ostimedly(nms/fac_ms);	//OS��ʱ
		}
		nms%=fac_ms;						//OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ    
	}
	delay_us((u32)(nms*1000));				//��ͨ��ʽ��ʱ  
}
#else 
//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//ʱ�����	  		 
	SysTick->VAL=0x00;        				//��ռ�����
	SysTick->CTRL=0x01 ;      				//��ʼ���� 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;      	 			//�رռ�����
	SysTick->VAL =0X00;       				//��ռ�����	 
}
//��ʱnms
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK��λΪHz,nms��λΪms
//��72M������,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;			//ʱ�����
	SysTick->VAL =0x00;           			//��ռ�����
	SysTick->CTRL=0x01 ;          			//��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;      	 			//�رռ�����
	SysTick->VAL =0X00;       				//��ռ�����	  	    
} 
#endif 




























