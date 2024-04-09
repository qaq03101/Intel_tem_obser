#include "netif/ethernetif.h" 
#include "dm9000.h"  
#include "lwip_comm.h" 
#include "malloc.h"
#include "netif/etharp.h"  
#include "string.h"  
#include "includes.h"
 
 
 
extern OS_SEM dm9000input;		//DM9000���������ź���


static err_t low_level_init(struct netif *netif)
{
	netif->hwaddr_len = ETHARP_HWADDR_LEN; //����MAC��ַ����,Ϊ6���ֽ�
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
//���ڷ������ݰ�����ײ㺯��(lwipͨ��netif->linkoutputָ��ú���)
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	DM9000_SendPacket(p);
	return ERR_OK;
}
//���ڽ������ݰ�����ײ㺯��
static struct pbuf * low_level_input(struct netif *netif)
{  
	struct pbuf *p;
	p=DM9000_Receive_Packet();
	return p;
}

//ʵ�ʵĽ��հ���������,�ϲ���dm9000���������װ
err_t ethernetif_input(struct netif *netif)
{
	OS_ERR _err;
	err_t err;
	struct pbuf *p;
	while(1)
	{
		OSSemPend((OS_SEM*)&dm9000input,0,OS_OPT_PEND_BLOCKING ,NULL,&_err);		//�����ź���
		if(_err == OS_ERR_NONE)
		{
			while(1)
			{
				p=low_level_input(netif);   
				if(p!=NULL)
				{
					err=netif->input(p, netif); //����netif�ṹ���е�input�ֶ�(һ������)���������ݰ�
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

//ʹ��low_level_init()��������ʼ������
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














