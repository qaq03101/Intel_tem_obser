#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "includes.h"
 
 
 
#define UDP_RX_BUFSIZE		2000		//定义udp最大接收数据长度
#define UDP_DEMO_PORT			8088	//定义udp连接的本地端口号
#define LWIP_SEND_DATA			0X80    //定义有数据发送

extern char udp_sendbuf[100];
extern u8 udp_flag;		//UDP数据发送标志位
OS_ERR user_udp_init(void);
#endif

