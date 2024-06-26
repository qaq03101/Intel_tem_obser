#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "delay.h"
#include "usart.h"  
#include <stdio.h>
#include "includes.h"
  



//lwip DHCP任务
OS_TCB LwipdhcpTaskTCP;
#define LWIP_DHCP_TASK_PRIO       		4 
#define LWIP_DHCP_STK_SIZE  		    128
CPU_STK * LWIP_DHCP_TASK_STK;
void lwip_dhcp_task(void *pdata); 


//lwip DM9000数据接收处理任务
#define LWIP_DM9000_INPUT_TASK_PRIO		4 
#define LWIP_DM9000_INPUT_TASK_SIZE	    512
CPU_STK * LWIP_DM9000_INPUT_TASK_STK;
OS_TCB LWIP_DM9000TCB;
void lwip_dm9000_input_task(void *pdata); 

//lwip 内核任务
OS_TCB LwipTaskTCP;
CPU_STK * TCPIP_THREAD_TASK_STK;			

OS_SEM dm9000input;					//DM9000接收数据信号量
OS_MUTEX dm9000lock;					//DM9000读写互锁控制信号量


__lwip_dev lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口
extern u32 memp_get_memorysize(void);	
extern u8_t *memp_memory;				
extern u8_t *ram_heap;					




//DM9000数据接收处理任务
void lwip_dm9000_input_task(void *pdata)
{
	ethernetif_input(&lwip_netif);
}

//lwip内核部分,内存申请
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			
	memp_memory=mymalloc(SRAMIN,mempsize);	
	printf("memp_memory内存大小为:%d\r\n",mempsize);
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;
	ram_heap=mymalloc(SRAMIN,ramheapsize);	
	printf("ram_heap内存大小为:%d\r\n",ramheapsize);
	TCPIP_THREAD_TASK_STK=mymalloc(SRAMIN,TCPIP_THREAD_STACKSIZE*4);			
	LWIP_DHCP_TASK_STK=mymalloc(SRAMIN,LWIP_DHCP_STK_SIZE*4);					
	LWIP_DM9000_INPUT_TASK_STK=mymalloc(SRAMIN,LWIP_DM9000_INPUT_TASK_SIZE*4);	 
	if(!memp_memory||!ram_heap||!TCPIP_THREAD_TASK_STK||!LWIP_DHCP_TASK_STK||!LWIP_DM9000_INPUT_TASK_STK)
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip内核部分,内存释放
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
	myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
	myfree(SRAMIN,LWIP_DM9000_INPUT_TASK_STK);
}
//lwip 默认IP设置
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//默认远程IP为:192.168.0.1
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=0;
	lwipx->remoteip[3]=1;
	//MAC地址设置
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//默认本地IP为:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	
} 

//创建dm9000接收任务
void creatdm9000task(){
	
	OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER(); 		
			OSTaskCreate((OS_TCB 	* )&LWIP_DM9000TCB,		
				 (CPU_CHAR	* )"dm9000 task", 		
                 (OS_TASK_PTR )lwip_dm9000_input_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LWIP_DM9000_INPUT_TASK_PRIO,     
                 (CPU_STK   * )&LWIP_DM9000_INPUT_TASK_STK[0],
                 (CPU_STK_SIZE)(LWIP_DM9000_INPUT_TASK_SIZE-1)/10,	
                 (CPU_STK_SIZE)LWIP_DM9000_INPUT_TASK_SIZE-1,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);				

	OS_CRITICAL_EXIT();			


}

//LWIP初始化(LWIP启动的时候使用)
u8 lwip_comm_init(void)
{
	
	
	OS_ERR err;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			
	struct ip_addr netmask; 			
	struct ip_addr gw;      			
	if(lwip_comm_mem_malloc())return 1;	
	
 	OSSemCreate((OS_SEM*)&dm9000input,"dm9000in",0,&err);			
	OSMutexCreate((OS_MUTEX*)&dm9000lock,"dm9000lock",&err);	
	
	if(DM9000_Init())return 2;			
	tcpip_init(NULL,NULL);				
	lwip_comm_default_ip_set(&lwipdev);	
#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	if(Netif_Init_Flag != NULL) 	
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	creatdm9000task();
	
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();		//创建DHCP任务
#endif		
	return 0;
}   

#if LWIP_DHCP
//创建DHCP任务
void lwip_comm_dhcp_creat(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();  
	OSTaskCreate((OS_TCB 	* )&LwipdhcpTaskTCP,		
			 (CPU_CHAR	* )"dhcp task", 		
			 (OS_TASK_PTR )lwip_dhcp_task, 			
			 (void		* )0,					
			 (OS_PRIO	  )LWIP_DHCP_TASK_PRIO,     
			 (CPU_STK   * )&LWIP_DHCP_TASK_STK[0],	
			 (CPU_STK_SIZE)(LWIP_DHCP_STK_SIZE-1)/10,	
			 (CPU_STK_SIZE)LWIP_DHCP_STK_SIZE-1,		
			 (OS_MSG_QTY  )0,					
			 (OS_TICK	  )0,					
			 (void   	* )0,					
			 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
			 (OS_ERR 	* )&err);				
	
	OS_CRITICAL_EXIT();  
}
//删除DHCP任务
void lwip_comm_dhcp_delete(void)
{
	OS_ERR err;
	dhcp_stop(&lwip_netif); 		
	OSTaskDel((OS_TCB*)&LwipdhcpTaskTCP,&err);	//删除DHCP任务
}
//DHCP处理任务
void lwip_dhcp_task(void *pdata)
{
	u32 ip=0,netmask=0,gw=0;
	dhcp_start(&lwip_netif);//开启DHCP 
	lwipdev.dhcpstatus=0;	//正在DHCP
	printf("正在查找DHCP服务器,请稍等...........\r\n");   
	while(1)
	{ 
		printf("正在获取地址...\r\n");
		ip=lwip_netif.ip_addr.addr;		
		netmask=lwip_netif.netmask.addr;
		gw=lwip_netif.gw.addr;			
		if(ip!=0)   					
		{
			lwipdev.dhcpstatus=2;	
 			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//解析出通过DHCP获取到的IP地址
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//解析通过DHCP获取到的子网掩码地址
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//解析出通过DHCP获取到的默认网关
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //通过DHCP服务获取IP地址失败,且超过最大尝试次数
		{  
			lwipdev.dhcpstatus=0XFF;
			//使用静态IP地址
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP服务超时,使用静态IP地址!\r\n");
			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		delay_ms(250);
	}
	lwip_comm_dhcp_delete();//删除DHCP任务 
}
#endif 





















































