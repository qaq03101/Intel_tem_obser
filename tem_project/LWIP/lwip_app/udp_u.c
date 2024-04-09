#include "udp_u.h"
#include "lwip_comm.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "task.h"
 
 
//TCP客户端任务
#define UDP_PRIO		6
//任务堆栈大小
#define UDP_STK_SIZE	300
//任务堆栈
CPU_STK UDP_TASK_STK[UDP_STK_SIZE];
OS_TCB UdpTheardTCP;


u8 udp_recvbuf[UDP_RX_BUFSIZE];	//UDP接收数据缓冲区
//UDP发送数据内容
char udp_sendbuf[100]="WARSHIP STM32F103 WARSHIP NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP数据发送标志位

//udp任务函数
static void udp_thread(void *arg)
{
	CPU_SR_ALLOC();
	OS_ERR err1;
	err_t err;
	static struct netconn *udpconn;
	static struct netbuf  *recvbuf;
	static struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u32 data_len = 0;
	struct pbuf *q;
	
	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP);  //创建一个UDP链接
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //创建UDP连接成功
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //构造目的IP地址
		netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//连接到远端主机
		if(err == ERR_OK)//绑定完成
		{
			while(1)
			{
				if((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf,strlen((char *)udp_sendbuf));
					sentbuf->p->payload = (char*)udp_sendbuf;   	//指udp_demo_sendbuf组
					err = netconn_send(udpconn,sentbuf);  	//将netbuf中的数据发送出去
					if(err != ERR_OK)
					{
						printf("发送失败\r\n");
						netbuf_delete(sentbuf);      //删除buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//清除数据发送标志
					netbuf_delete(sentbuf);      	//删除buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //接收数据
				if(recvbuf != NULL)        
				{ 
					OS_CRITICAL_ENTER(); //关中断
					memset(udp_recvbuf,0,UDP_RX_BUFSIZE);  //数据接收缓冲区清零
					for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
					{
						if(q->len > (UDP_RX_BUFSIZE-data_len)) memcpy(udp_recvbuf+data_len,q->payload,(UDP_RX_BUFSIZE-data_len));//拷贝数据
						else memcpy(udp_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > UDP_RX_BUFSIZE) break; //超出UDP接收数组,跳出	
					}
					OS_CRITICAL_EXIT();  //开中断
					data_len=0;  //复制完成后data_len要清零。
					OSQPost((OS_Q*)&ORDER_Msg,
							(void *)udp_recvbuf,
							(OS_MSG_SIZE)strlen((char *)udp_recvbuf),
							(OS_OPT)OS_OPT_POST_FIFO,
							(OS_ERR*)&err);
					netbuf_delete(recvbuf);      //删除buf
				}else OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err1);  //延时5ms
			}
		}else printf("UDP绑定失败\r\n");
	}else printf("UDP连接创建失败\r\n");
}


//创建UDP线程
//返回值:0 UDP创建成功
//		其他 UDP创建失败
OS_ERR user_udp_init(void)
{

	OS_ERR err;
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER();
	
	OSTaskCreate((OS_TCB 	* )&UdpTheardTCP,		
			 (CPU_CHAR	* )"udp_thread task", 		
			 (OS_TASK_PTR )udp_thread, 			
			 (void		* )0,					
			 (OS_PRIO	  )UDP_PRIO,     
			 (CPU_STK   * )&UDP_TASK_STK[0],	
			 (CPU_STK_SIZE)UDP_STK_SIZE/10,	
			 (CPU_STK_SIZE)UDP_STK_SIZE,		
			 (OS_MSG_QTY  )0,					
			 (OS_TICK	  )0,					
			 (void   	* )0,					
			 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
			 (OS_ERR 	* )&err);	
	
	OS_CRITICAL_EXIT();		
	
	return err;
}

