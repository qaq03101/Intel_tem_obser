#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "includes.h"
 
 
 
#define UDP_RX_BUFSIZE		2000		//����udp���������ݳ���
#define UDP_DEMO_PORT			8088	//����udp���ӵı��ض˿ں�
#define LWIP_SEND_DATA			0X80    //���������ݷ���

extern char udp_sendbuf[100];
extern u8 udp_flag;		//UDP���ݷ��ͱ�־λ
OS_ERR user_udp_init(void);
#endif

