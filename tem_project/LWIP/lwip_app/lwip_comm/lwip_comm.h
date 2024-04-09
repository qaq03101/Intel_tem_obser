#ifndef __LWIP_COMM_H
#define __LWIP_COMM_H 
#include "dm9000.h" 
 

#define LWIP_MAX_DHCP_TRIES		4   
//lwip���ƽṹ��
typedef struct  
{
	u8 mac[6];      
	u8 remoteip[4];	
	u8 ip[4];       
	u8 netmask[4]; 	
	u8 gateway[4]; 	
	
	vu8 dhcpstatus;	
					//0,δ��ȡDHCP��ַ;
					//1,����DHCP��ȡ״̬
					//2,�ɹ���ȡDHCP��ַ
					//0XFF,��ȡʧ��.
}__lwip_dev;
extern __lwip_dev lwipdev;	

void DM9000_LWIP_CreateTask(void);

void lwip_pkt_handle(void *pdata);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
u8 lwip_comm_init(void);
void lwip_comm_dhcp_creat(void);
void lwip_comm_dhcp_delete(void);
void lwip_comm_destroy(void);
void lwip_comm_delete_next_timeout(void);


#endif













