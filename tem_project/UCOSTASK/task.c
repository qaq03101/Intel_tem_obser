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
//----------LED����
OS_TCB Led0TaskTCB;	
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];

//----------��ʾ����
OS_TCB DisplayTaskTCB;
CPU_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];

//----------��ʼ����
OS_TCB StartTaskTCB;
CPU_STK START_TASK_STK[START_STK_SIZE];

//---------������������
OS_TCB SendUdpTCB;
CPU_STK SEND_UDP_TASK_STK[SEND_UDP_STK_SIZE];

//---------�ɼ��¶�����
OS_TCB CapTempTCB;
CPU_STK CAP_TEMP_TASK_STK[CAP_TEMP_STK_SIZE];

//---------����¶�����
OS_TCB CheckTempTCB;
CPU_STK CHECK_TEMP_TASK_STK[CHECK_TEMP_STK_SIZE];

//---------DHCP��������
OS_TCB DhcpStartTCB;
CPU_STK DHCP_START_TASK_STK[DHCP_START_STK_SIZE];

//---------���������
OS_TCB OrderHandleTCB;
CPU_STK ORDER_HANDLE_TASK_STK[ORDER_HANDLE_STK_SIZE];

//�������
OS_Q ORDER_Msg;
//�������ݶ���
OS_Q DATA_Msg;
//ͬ���ź�
OS_SEM SYN;
//�Ͻ����½�
short LIMIT_MAX=9999;
short LIMIT_MIN=9000;
//�Ƿ���ȹ�
int isHotOver=0;
//��ǰ״̬�Ƿ�Ϊ����
int isDormancy=0;
//��ʾip
void show_address(u8 mode);
void ip_show(u8 mode);


//������������
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


//��ʾ����
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
				LCD_ShowString(100+20*8,150,16,16,16,"-"); //��ʾ����
			}
			else
				LCD_ShowString(100+20*8,150,16,16,16," "); //�޷���
			LCD_ShowxNum(100+21*8,150,temp/100,2,16,0); //��ʾ��������
			LCD_ShowxNum(100+24*8,150,temp%100,2,16, 0X80); //��ʾС������
		}
		
		LCD_ShowxNum(100+23*8,90,LIMIT_MIN/100,2,16,0);  //�����¶���ʾ
		LCD_ShowxNum(100+26*8,90,LIMIT_MIN%100,2,16, 0X80); 
		LCD_ShowxNum(100+24*8,120,LIMIT_MAX/100,2,16,0);  //�����¶���ʾ
		LCD_ShowxNum(100+27*8,120,LIMIT_MAX%100,2,16, 0X80); 
	
	}
}
//dhcp����Ϣ
void dhcp_start_task(void *pdata)
{
	OS_ERR err;
	while(1){
		LCD_ShowString(250,650,210,16,16,"DHCP is starting......"); 
		if(lwipdev.dhcpstatus != 0) 			//����DHCP
		{
			
			show_address(lwipdev.dhcpstatus );	//��ʾ��ַ��Ϣ
			OSTaskSuspend((OS_TCB*)&DhcpStartTCB,&err); 		//��ʾ���ַ��Ϣ�������������
		}
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);
	}
	
}

//led0������
void led0_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		LED0=0;
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
		LED0=1;

		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
	}
}

//�ɼ��¶�����
void cap_temp_task(void *p_arg){
	OS_ERR err;
	short temp;
	char buf[10]={0};
	CPU_SR_ALLOC();
	while(1)
	{
		OSSemPend(&SYN,0,OS_OPT_PEND_BLOCKING,0,&err);
		temp=Get_Temprate(); //�õ��¶�ֵ
		sprintf(buf,"%d",temp);
		OSQPost((OS_Q*)&DATA_Msg,
				(void *)buf,
				(OS_MSG_SIZE)strlen(buf),
				(OS_OPT)OS_OPT_POST_FIFO+OS_OPT_POST_ALL,
				(OS_ERR*)&err);
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
	}
	
}

//����¶�
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

//�����
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
			if (buf[0]=='t'){	//�趨���½��¶�����
				sscanf(buf, "t %hd,%hd", &LIMIT_MIN, &LIMIT_MAX);
				isHotOver=0;
			}
			else if(buf[0]=='s' && isDormancy==1){ //�ָ�ϵͳ
				recovery_task();
			}
			else if(buf[0]=='o' && isDormancy==0){ //����ϵͳ
				suspend_task();
			}
			else if(buf[0]=='q') {  //�������趨�¶�ֵ
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
//��LCD����ʾ��ַ��Ϣ
//mode:2 ��ʾDHCP��ȡ���ĵ�ַ
void show_address(u8 mode)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	ip_show(2);
	OS_CRITICAL_ENTER();

		//��������udp����
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
		//�����ɼ��¶�����
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
			//��������¶�����
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
				//������ʾ����
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
					//�������������
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
	suspend_task();//��������
}

//������ʾ����
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

//�ָ���ʾ����
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


//ip��ʾ
void ip_show(u8 mode){
	u8 buf[30];
	LCD_Fill(10,50,230,150,WHITE);
	if(mode==2)
	{
		LCD_ShowString(250,650,210,16,16,"NET connection !             "); 
		sprintf((char*)buf,"IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		LCD_ShowString(250,670,210,16,16,buf); 
		sprintf((char*)buf,"GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		LCD_ShowString(250,690,210,16,16,buf); 
	}
}