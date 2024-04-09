#include "netif/ethernetif.h" 
#include "dm9000.h"  
#include "lwip_comm.h" 
#include "malloc.h"
#include "netif/etharp.h"  
#include "string.h"  
#include "includes.h"
 
 
 
extern OS_SEM dm9000input;		//DM9000接收数据信号量


static err_t low_level_init(struct netif *netif)
{
	netif->hwaddr_len = ETHARP_HWADDR_LEN; //设置MAC地址长度,为6个字节
	netif->hwaddr[0]=lwipdev.mac[0]; 
	netif->hwaddr[1]=lwipdev.mac[1]; 
	netif->hwaddr[2]=lwipdev.mac[2];
	netif->hwaddr[3]=lwipdev.mac[3];
	netif->hwaddr[4]=lwipdev.mac[4];
	netif->hwaddr[5]=lwipdev.mac[5];
	netif->mtu=1500; 
	netif->flags = NETIF_FLAG_BROADCAST|NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP; 
	return ERR_OK;
} 
//用于发送数据包的最底层函数(lwip通过netif->linkoutput指向该函数)
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	DM9000_SendPacket(p);
	return ERR_OK;
}
//用于接收数据包的最底层函数
static struct pbuf * low_level_input(struct netif *netif)
{  
	struct pbuf *p;
	p=DM9000_Receive_Packet();
	return p;
}

//实际的接收包工作函数,上层用dm9000接收任务封装
err_t ethernetif_input(struct netif *netif)
{
	OS_ERR _err;
	err_t err;
	struct pbuf *p;
	while(1)
	{
		OSSemPend((OS_SEM*)&dm9000input,0,OS_OPT_PEND_BLOCKING ,NULL,&_err);		//请求信号量
		if(_err == OS_ERR_NONE)
		{
			while(1)
			{
				p=low_level_input(netif);   
				if(p!=NULL)
				{
					err=netif->input(p, netif); //调用netif结构体中的input字段(一个函数)来处理数据包
					if(err!=ERR_OK)
					{
						LWIP_DEBUGF(NETIF_DEBUG,("ethernetif_input: IP input error\n"));
						pbuf_free(p);
						p = NULL;
					} 
				}else break; 
			}
		}
	}
} 

//使用low_level_init()函数来初始化网络
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif!=NULL",(netif!=NULL));
#if LWIP_NETIF_HOSTNAME			//LWIP_NETIF_HOSTNAME 
	netif->hostname="lwip";  	
#endif 
	netif->name[0]=IFNAME0; 	
	netif->name[1]=IFNAME1; 	
	netif->output=etharp_output;
	netif->linkoutput=low_level_output;
	low_level_init(netif); 		
	return ERR_OK;
}














