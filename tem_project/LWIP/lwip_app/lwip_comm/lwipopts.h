#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
 
#define SYS_LIGHTWEIGHT_PROT    		1							//为1时使用实时操作系统的轻量级保护,保护关键代码不被中断打断
#define NO_SYS                  		0  							//使用UCOS操作系统
#define MEM_ALIGNMENT           		4  							//使用4字节对齐模式
#define MEM_SIZE                		6*1024						//内存堆heap大小
#define MEMP_NUM_PBUF           		16							//memp结构的pbuf数量
#define MEMP_NUM_UDP_PCB        		5							//UDP协议控制块数量.
#define MEMP_NUM_TCP_PCB        		5							//同时建立激活的TCP数量
#define MEMP_NUM_TCP_PCB_LISTEN 		6							//能够监听的TCP连接数量
#define MEMP_NUM_TCP_SEG        		10							//最多同时在队列中的TCP段数量
#define MEMP_NUM_SYS_TIMEOUT    		8							//能够同时激活的timeout个数


#define PBUF_POOL_SIZE          		20							//pbuf内存池个数
#define PBUF_POOL_BUFSIZE     			512							//每个pbuf内存池大小

#define LWIP_TCP                		1  							//使用TCP
#define TCP_TTL                			255							

#undef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ         		1 							//当TCP的数据段超出队列时的控制位

#undef TCPIP_MBOX_SIZE
#define TCPIP_MBOX_SIZE         		MAX_QUEUE_ENTRIES   		//tcpip创建主线程消息邮箱大小

#undef DEFAULT_TCP_RECVMBOX_SIZE
#define DEFAULT_TCP_RECVMBOX_SIZE       MAX_QUEUE_ENTRIES  

#undef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         MAX_QUEUE_ENTRIES  

		
#define TCP_MSS                			(1500 - 40)	  				//最大TCP分段
#define TCP_SND_BUF            		 	(2*TCP_MSS)					//TCP发送缓冲区大小
#define TCP_SND_QUEUELEN       		 	(4* TCP_SND_BUF/TCP_MSS)	//TCP发送缓冲区大小
#define TCP_WND               		  	(2*TCP_MSS)					//TCP发送窗口
#define LWIP_ICMP             		  	1 							
#define LWIP_DHCP             		  	1							
#define LWIP_UDP              		  	1 							
#define UDP_TTL               		  	255 						
#define LWIP_STATS 						0
#define LWIP_PROVIDE_ERRNO 				1


#define LWIP_NETCONN                    1 							//使能NETCON函数
#define LWIP_SOCKET                     1							//使能Sicket API
#define LWIP_COMPAT_MUTEX               1		
#define LWIP_SO_RCVTIMEO                1 							//使能netconn结构体中recv_timeout,可以避免阻塞线程

//有关系统的选项
#define TCPIP_THREAD_PRIO				3							
#define TCPIP_THREAD_STACKSIZE          1000						
#define DEFAULT_UDP_RECVMBOX_SIZE       2000
#define DEFAULT_THREAD_STACKSIZE        512


#define LWIP_DEBUG                    	0	 						
#define ICMP_DEBUG                     	LWIP_DBG_OFF 				

#endif /* __LWIPOPTS_H__ */













