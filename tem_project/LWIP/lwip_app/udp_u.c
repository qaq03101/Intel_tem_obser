#include "udp_u.h"
#include "lwip_comm.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "task.h"
 
 
//TCP�ͻ�������
#define UDP_PRIO		6
//�����ջ��С
#define UDP_STK_SIZE	300
//�����ջ
CPU_STK UDP_TASK_STK[UDP_STK_SIZE];
OS_TCB UdpTheardTCP;


u8 udp_recvbuf[UDP_RX_BUFSIZE];	//UDP�������ݻ�����
//UDP������������
char udp_sendbuf[100]="WARSHIP STM32F103 WARSHIP NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP���ݷ��ͱ�־λ

//udp������
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
	udpconn = netconn_new(NETCONN_UDP);  //����һ��UDP����
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //����UDP���ӳɹ�
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //����Ŀ��IP��ַ
		netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//���ӵ�Զ������
		if(err == ERR_OK)//�����
		{
			while(1)
			{
				if((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //������Ҫ����
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf,strlen((char *)udp_sendbuf));
					sentbuf->p->payload = (char*)udp_sendbuf;   	//ָudp_demo_sendbuf��
					err = netconn_send(udpconn,sentbuf);  	//��netbuf�е����ݷ��ͳ�ȥ
					if(err != ERR_OK)
					{
						printf("����ʧ��\r\n");
						netbuf_delete(sentbuf);      //ɾ��buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//������ݷ��ͱ�־
					netbuf_delete(sentbuf);      	//ɾ��buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //��������
				if(recvbuf != NULL)        
				{ 
					OS_CRITICAL_ENTER(); //���ж�
					memset(udp_recvbuf,0,UDP_RX_BUFSIZE);  //���ݽ��ջ���������
					for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
					{
						if(q->len > (UDP_RX_BUFSIZE-data_len)) memcpy(udp_recvbuf+data_len,q->payload,(UDP_RX_BUFSIZE-data_len));//��������
						else memcpy(udp_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > UDP_RX_BUFSIZE) break; //����UDP��������,����	
					}
					OS_CRITICAL_EXIT();  //���ж�
					data_len=0;  //������ɺ�data_lenҪ���㡣
					OSQPost((OS_Q*)&ORDER_Msg,
							(void *)udp_recvbuf,
							(OS_MSG_SIZE)strlen((char *)udp_recvbuf),
							(OS_OPT)OS_OPT_POST_FIFO,
							(OS_ERR*)&err);
					netbuf_delete(recvbuf);      //ɾ��buf
				}else OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err1);  //��ʱ5ms
			}
		}else printf("UDP��ʧ��\r\n");
	}else printf("UDP���Ӵ���ʧ��\r\n");
}


//����UDP�߳�
//����ֵ:0 UDP�����ɹ�
//		���� UDP����ʧ��
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

