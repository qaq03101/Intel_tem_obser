#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "sram.h"
#include "stm32f10x.h"
#include "string.h"
#include "includes.h"
#include "malloc.h"
#include "usmart.h"	
#include "dm9000.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "includes.h"
#include "lcd.h"
#include "led.h"
#include "delay.h"
#include "task.h"
#include "udp_u.h"
#include "adc.h"
#include "beep.h"
#include "exti.h"



//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�3��LWIP�����߳�,��ʼ����
//���ȼ�4��DHCP����(һ����)
//���ȼ�4:DM9000���ݽ��մ�������
//���ȼ�5:�������������,dhcp��������
//���ȼ�6:UDP���ݷ�������
//���ȼ�7:�¶ȼ������
//���ȼ�8:�ɼ��¶�����
//���ȼ�10:��ʾ��Ϣ����
//���ȼ�11:LED����
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
//�ж�   DM9000��2,��ռ0,������0
//		����������1,��ռ0,������0


int main(void)
{

	int i;
	OS_ERR err;
	CPU_SR_ALLOC();
	Stm32_Clock_Init(9);//ϵͳʱ������
	delay_init(72);	  	//��ʱ��ʼ��
	uart_init(72,115200);	 	//���ڳ�ʼ��Ϊ115200
	MY_NVIC_PriorityGroupConfig(2);//�жϷ�������	
	FSMC_SRAM_Init();		//��ʼ���ⲿSRAM
	LED_Init();         //LED��ʼ��
	LCD_Init();				//��ʼ��LCD
	BEEP_Init(); //��ʼ�������� IO
	EXTIX_Init(); //��ʼ���ⲿ�ж���
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
	Adc_Init();
	POINT_COLOR = BLUE; 	//��ɫ����
	LCD_ShowString(30,50,200,20,16,"OS Initing.....");
	OSInit(&err);		//��ʼ��UCOSIII
	LCD_ShowString(30,50,200,20,16,"OS Init Success!");
	LCD_ShowString(30,70,200,20,16,"LWIP Initing.....");
	while(lwip_comm_init()) //lwip��ʼ��
		{
			LCD_ShowString(30,70,200,20,16,"Lwip Init failed!"); 	//lwip��ʼ��ʧ��
			delay_ms(500);
			LCD_Fill(30,70,230,150,WHITE);
			delay_ms(500);
		}
	LCD_ShowString(30,70,200,20,16,"Lwip Init Success!"); 		//lwip��ʼ���ɹ�
	LCD_ShowString(30,90,200,20,16,"UDP Initing.....");
	while(user_udp_init()){
		LCD_ShowString(30,90,200,20,16,"UDP failed");
		delay_ms(500);
		LCD_Fill(30,90,230,170,WHITE);
		delay_ms(500);
	}
	LCD_ShowString(30,90,200,20,16,"UDP Success!       ");
	OS_CRITICAL_ENTER();//�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		
				 (CPU_CHAR	* )"start task", 		
                 (OS_TASK_PTR )start_task, 		
                 (void		* )0,					
                 (OS_PRIO	  )START_TASK_PRIO,    
                 (CPU_STK   * )&START_TASK_STK[0],	
                 (CPU_STK_SIZE)START_STK_SIZE/10,	
                 (CPU_STK_SIZE)START_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while(1);
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //ʹ��ʱ��Ƭ��ת
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	
	//�ź���ͨ�Ŷ��д���
	OSQCreate ( (OS_Q* )&DATA_Msg, 
				 (CPU_CHAR* )"DATA Msg",
				 (OS_MSG_QTY )5,
				 (OS_ERR* )&err);
				 
	OSQCreate ( (OS_Q* )&ORDER_Msg, 
				 (CPU_CHAR* )"ORDER Msg",
				 (OS_MSG_QTY )10,
				 (OS_ERR* )&err);
				 
	OSSemCreate((OS_SEM *)&SYN,
				(CPU_CHAR* )"SYNC_SEM",
				(OS_SEM_CTR )0,
				(OS_ERR* )&err);

	
		//dhcp��������		 
	OSTaskCreate((OS_TCB 	* )&DhcpStartTCB,		
				 (CPU_CHAR	* )"DhcpStart_task", 		
                 (OS_TASK_PTR )dhcp_start_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )DHCP_START_TASK_PRIO,     
                 (CPU_STK   * )&DHCP_START_TASK_STK[0],	
                 (CPU_STK_SIZE)DHCP_START_STK_SIZE/10,	
                 (CPU_STK_SIZE)DHCP_START_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	//����LED0����
	OSTaskCreate((OS_TCB 	* )&Led0TaskTCB,		
				 (CPU_CHAR	* )"led0 task", 		
                 (OS_TASK_PTR )led0_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED0_TASK_PRIO,     
                 (CPU_STK   * )&LED0_TASK_STK[0],	
                 (CPU_STK_SIZE)LED0_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED0_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	

				 
	OSSemPost ( (OS_SEM *)&SYN,
				 (OS_OPT)OS_OPT_POST_1 ,
				 (OS_ERR *)&err);

	
	OSTaskSuspend((OS_TCB*)&StartTaskTCB,&err); //����start_task����
	OS_CRITICAL_EXIT();	
	
}




