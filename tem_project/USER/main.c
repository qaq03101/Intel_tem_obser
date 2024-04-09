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



//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级3：LWIP核心线程,开始任务
//优先级4：DHCP任务(一次性)
//优先级4:DM9000数据接收处理任务
//优先级5:接收命令处理任务,dhcp启动任务
//优先级6:UDP数据发送任务
//优先级7:温度检测任务
//优先级8:采集温度任务
//优先级10:显示信息任务
//优先级11:LED任务
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//中断   DM9000组2,抢占0,子优先0
//		按键休眠组1,抢占0,子优先0


int main(void)
{

	int i;
	OS_ERR err;
	CPU_SR_ALLOC();
	Stm32_Clock_Init(9);//系统时钟设置
	delay_init(72);	  	//延时初始化
	uart_init(72,115200);	 	//串口初始化为115200
	MY_NVIC_PriorityGroupConfig(2);//中断分组配置	
	FSMC_SRAM_Init();		//初始化外部SRAM
	LED_Init();         //LED初始化
	LCD_Init();				//初始化LCD
	BEEP_Init(); //初始化蜂鸣器 IO
	EXTIX_Init(); //初始化外部中断输
	my_mem_init(SRAMIN);		//初始化内部内存池
	my_mem_init(SRAMEX);		//初始化外部内存池
	Adc_Init();
	POINT_COLOR = BLUE; 	//蓝色字体
	LCD_ShowString(30,50,200,20,16,"OS Initing.....");
	OSInit(&err);		//初始化UCOSIII
	LCD_ShowString(30,50,200,20,16,"OS Init Success!");
	LCD_ShowString(30,70,200,20,16,"LWIP Initing.....");
	while(lwip_comm_init()) //lwip初始化
		{
			LCD_ShowString(30,70,200,20,16,"Lwip Init failed!"); 	//lwip初始化失败
			delay_ms(500);
			LCD_Fill(30,70,230,150,WHITE);
			delay_ms(500);
		}
	LCD_ShowString(30,70,200,20,16,"Lwip Init Success!"); 		//lwip初始化成功
	LCD_ShowString(30,90,200,20,16,"UDP Initing.....");
	while(user_udp_init()){
		LCD_ShowString(30,90,200,20,16,"UDP failed");
		delay_ms(500);
		LCD_Fill(30,90,230,170,WHITE);
		delay_ms(500);
	}
	LCD_ShowString(30,90,200,20,16,"UDP Success!       ");
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
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
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //使用时间片轮转
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	
	//信号与通信队列创建
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

	
		//dhcp启动任务		 
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
	//创建LED0任务
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

	
	OSTaskSuspend((OS_TCB*)&StartTaskTCB,&err); //挂起start_task任务
	OS_CRITICAL_EXIT();	
	
}




