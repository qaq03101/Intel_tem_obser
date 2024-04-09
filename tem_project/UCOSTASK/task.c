#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "sram.h"
#include "stm32f10x.h"
#include "string.h"
#include "malloc.h"
#include "usmart.h"	
#include "dm9000.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "includes.h"
#include "lcd.h"
#include "led.h"
#include "task.h"
#include "udp_u.h"
#include "adc.h"
#include <stdio.h>
#include "beep.h"
//----------LED任务
OS_TCB Led0TaskTCB;	
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];

//----------显示任务
OS_TCB DisplayTaskTCB;
CPU_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];

//----------开始任务
OS_TCB StartTaskTCB;
CPU_STK START_TASK_STK[START_STK_SIZE];

//---------发送数据任务
OS_TCB SendUdpTCB;
CPU_STK SEND_UDP_TASK_STK[SEND_UDP_STK_SIZE];

//---------采集温度任务
OS_TCB CapTempTCB;
CPU_STK CAP_TEMP_TASK_STK[CAP_TEMP_STK_SIZE];

//---------检测温度任务
OS_TCB CheckTempTCB;
CPU_STK CHECK_TEMP_TASK_STK[CHECK_TEMP_STK_SIZE];

//---------DHCP启动任务
OS_TCB DhcpStartTCB;
CPU_STK DHCP_START_TASK_STK[DHCP_START_STK_SIZE];

//---------命令处理任务
OS_TCB OrderHandleTCB;
CPU_STK ORDER_HANDLE_TASK_STK[ORDER_HANDLE_STK_SIZE];

//命令队列
OS_Q ORDER_Msg;
//发送数据队列
OS_Q DATA_Msg;
//同步信号
OS_SEM SYN;
//上界与下界
short LIMIT_MAX=9999;
short LIMIT_MIN=9000;
//是否加热过
int isHotOver=0;
//当前状态是否为休眠
int isDormancy=0;
//显示ip
void show_address(u8 mode);
void ip_show(u8 mode);


//发送数据任务
void send_udp_task(void *pdata){
	char *buf={0};
	OS_ERR err;
	OS_MSG_SIZE l;

	while(1){
		buf=OSQPend((OS_Q*)&DATA_Msg,
				(OS_TICK)0,
				(OS_OPT)OS_OPT_PEND_BLOCKING,
				(OS_MSG_SIZE*)&l,
				 NULL,
				(OS_ERR*)&err);
		if (buf!=NULL){
			strlcpy(udp_sendbuf,buf,sizeof(udp_sendbuf));
			udp_flag |= LWIP_SEND_DATA;
		}
		OSSemPost ( (OS_SEM *)&SYN,
			 (OS_OPT)OS_OPT_POST_1,
			 (OS_ERR *)&err);
	}
}


//显示任务
void display_task(void *pdata){
	char *buf={0};
	short temp;
	OS_ERR err;
	OS_MSG_SIZE l;

	while(1){
		LCD_ShowString(100,90,250,20,16,"Low temperature limit: ");
		LCD_ShowString(100,120,250,20,16,"High temperature limit: ");
		LCD_ShowString(100,150,250,20,16,"Current temperature: ");	
		LCD_ShowString(100+25*8,90,250,20,16,".");
		LCD_ShowString(100+26*8,120,250,20,16,".");
		LCD_ShowString(100+23*8,150,250,20,16,".");
		buf=OSQPend((OS_Q*)&DATA_Msg,
					(OS_TICK)0,
					(OS_OPT)OS_OPT_PEND_BLOCKING,
					(OS_MSG_SIZE*)&l,
					 NULL,
					(OS_ERR*)&err);
		if (buf!=NULL){
			sscanf(buf, "%hd", &temp);
			if(temp<0)
			{
				temp=-temp;
				LCD_ShowString(100+20*8,150,16,16,16,"-"); //显示负号
			}
			else
				LCD_ShowString(100+20*8,150,16,16,16," "); //无符号
			LCD_ShowxNum(100+21*8,150,temp/100,2,16,0); //显示整数部分
			LCD_ShowxNum(100+24*8,150,temp%100,2,16, 0X80); //显示小数部分
		}
		
		LCD_ShowxNum(100+23*8,90,LIMIT_MIN/100,2,16,0);  //限制温度显示
		LCD_ShowxNum(100+26*8,90,LIMIT_MIN%100,2,16, 0X80); 
		LCD_ShowxNum(100+24*8,120,LIMIT_MAX/100,2,16,0);  //限制温度显示
		LCD_ShowxNum(100+27*8,120,LIMIT_MAX%100,2,16, 0X80); 
	
	}
}
//dhcp等信息
void dhcp_start_task(void *pdata)
{
	OS_ERR err;
	while(1){
		LCD_ShowString(250,650,210,16,16,"DHCP is starting......"); 
		if(lwipdev.dhcpstatus != 0) 			//开启DHCP
		{
			
			show_address(lwipdev.dhcpstatus );	//显示地址信息
			OSTaskSuspend((OS_TCB*)&DhcpStartTCB,&err); 		//显示完地址信息后挂起自身任务
		}
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
	}
	
}

//led0任务函数
void led0_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		LED0=0;
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
		LED0=1;

		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
	}
}

//采集温度任务
void cap_temp_task(void *p_arg){
	OS_ERR err;
	short temp;
	char buf[10]={0};
	CPU_SR_ALLOC();
	while(1)
	{
		OSSemPend(&SYN,0,OS_OPT_PEND_BLOCKING,0,&err);
		temp=Get_Temprate(); //得到温度值
		sprintf(buf,"%d",temp);
		OSQPost((OS_Q*)&DATA_Msg,
				(void *)buf,
				(OS_MSG_SIZE)strlen(buf),
				(OS_OPT)OS_OPT_POST_FIFO+OS_OPT_POST_ALL,
				(OS_ERR*)&err);
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
	}
	
}

//检测温度
void check_temp_task(void *pdata)
{
	char *buf={0};
	short temp;
	OS_ERR err;
	OS_MSG_SIZE l;
	while(1){
		buf=OSQPend((OS_Q*)&DATA_Msg,
				(OS_TICK)0,
				(OS_OPT)OS_OPT_PEND_BLOCKING,
				(OS_MSG_SIZE*)&l,
				 NULL,
				(OS_ERR*)&err);
		if (buf!=NULL){
			sscanf(buf, "%hd", &temp);
			if (temp>=LIMIT_MAX && isDormancy!=1) {
				isHotOver=1;
				BEEP=1;
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
				BEEP=0;
				suspend_task();
				
			}
			else if (isHotOver==1 && temp<LIMIT_MIN&&isDormancy!=0){
				recovery_task();
				
			}
			
		}
	}
}

//命令处理
void order_handle_task(void *pdata){
	char *buf={0};
	OS_ERR err;
	OS_MSG_SIZE l;
	while(1){
		buf=OSQPend((OS_Q*)&ORDER_Msg,
				(OS_TICK)0,
				(OS_OPT)OS_OPT_PEND_BLOCKING,
				(OS_MSG_SIZE*)&l,
				 NULL,
				(OS_ERR*)&err);
		if (buf!=NULL){
			if (buf[0]=='t'){	//设定上下界温度区间
				sscanf(buf, "t %hd,%hd", &LIMIT_MIN, &LIMIT_MAX);
				isHotOver=0;
			}
			else if(buf[0]=='s' && isDormancy==1){ //恢复系统
				recovery_task();
			}
			else if(buf[0]=='o' && isDormancy==0){ //休眠系统
				suspend_task();
			}
			else if(buf[0]=='q') {  //测试用设定温度值
				sscanf(buf,"q %s",buf);
				OSQPost((OS_Q*)&DATA_Msg,
						(void *)buf,
						(OS_MSG_SIZE)strlen(buf),
						(OS_OPT)OS_OPT_POST_FIFO+OS_OPT_POST_ALL,
						(OS_ERR*)&err);
			}
				
		}
	}
}
//在LCD上显示地址信息
//mode:2 显示DHCP获取到的地址
void show_address(u8 mode)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	ip_show(2);
	OS_CRITICAL_ENTER();

		//创建发送udp任务
	OSTaskCreate((OS_TCB 	* )&SendUdpTCB,		
				 (CPU_CHAR	* )"send udp task", 		
                 (OS_TASK_PTR )send_udp_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )SEND_UDP_TASK_PRIO,     
                 (CPU_STK   * )&SEND_UDP_TASK_STK[0],	
                 (CPU_STK_SIZE)SEND_UDP_STK_SIZE/10,	
                 (CPU_STK_SIZE)SEND_UDP_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
		//创建采集温度任务
	OSTaskCreate((OS_TCB 	* )&CapTempTCB,		
				 (CPU_CHAR	* )"Cap temp task", 		
                 (OS_TASK_PTR )cap_temp_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CAP_TEMP_PRIO,     
                 (CPU_STK   * )&CAP_TEMP_TASK_STK[0],	
                 (CPU_STK_SIZE)CAP_TEMP_STK_SIZE/10,	
                 (CPU_STK_SIZE)CAP_TEMP_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
			//创建检测温度任务
	OSTaskCreate((OS_TCB 	* )&CheckTempTCB,		
				 (CPU_CHAR	* )"Check temp task", 		
                 (OS_TASK_PTR )check_temp_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CHECK_TEMP_PRIO,     
                 (CPU_STK   * )&CHECK_TEMP_TASK_STK[0],	
                 (CPU_STK_SIZE)CHECK_TEMP_STK_SIZE/10,	
                 (CPU_STK_SIZE)CHECK_TEMP_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
				//创建显示任务
	OSTaskCreate((OS_TCB 	* )&DisplayTaskTCB,		
				 (CPU_CHAR	* )"Display task", 		
                 (OS_TASK_PTR )display_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CHECK_TEMP_PRIO,     
                 (CPU_STK   * )&DISPLAY_TASK_STK[0],	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE/10,	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
					//创建命令处理任务
	OSTaskCreate((OS_TCB 	* )&OrderHandleTCB,		
				 (CPU_CHAR	* )"Order handle task", 		
                 (OS_TASK_PTR )order_handle_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )ORDER_HANDLE_PRIO,     
                 (CPU_STK   * )&ORDER_HANDLE_TASK_STK[0],	
                 (CPU_STK_SIZE)ORDER_HANDLE_STK_SIZE/10,	
                 (CPU_STK_SIZE)ORDER_HANDLE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	OS_CRITICAL_EXIT();	
	suspend_task();//进入休眠
}

//挂起显示任务
void suspend_task(){
	char *buf="OVER";
	OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();
	OSTaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);
	OSQPost((OS_Q*)&DATA_Msg,(void *)buf,(OS_MSG_SIZE)strlen(buf),(OS_OPT)OS_OPT_POST_FIFO,(OS_ERR*)&err);
	LCD_Clear(GRAY);
	LCD_ShowString(100,100,210,16,16,"Dormancy!!!!!"); 
	isDormancy=1;
	OS_CRITICAL_EXIT();
}

//恢复显示任务
void recovery_task(){
	char *buf="RECO";
	OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();
	LCD_Clear(WHITE);
	OSQPost((OS_Q*)&DATA_Msg,(void *)buf,(OS_MSG_SIZE)strlen(buf),(OS_OPT)OS_OPT_POST_FIFO,(OS_ERR*)&err);
	OSTaskResume((OS_TCB*)&DisplayTaskTCB,&err);
	isDormancy=0;
	ip_show(2);
	OS_CRITICAL_EXIT();
}	


//ip显示
void ip_show(u8 mode){
	u8 buf[30];
	LCD_Fill(10,50,230,150,WHITE);
	if(mode==2)
	{
		LCD_ShowString(250,650,210,16,16,"NET connection !             "); 
		sprintf((char*)buf,"IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		LCD_ShowString(250,670,210,16,16,buf); 
		sprintf((char*)buf,"GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		LCD_ShowString(250,690,210,16,16,buf); 
	}
}