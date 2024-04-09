#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
 
#define SYS_LIGHTWEIGHT_PROT    		1							//Ϊ1ʱʹ��ʵʱ����ϵͳ������������,�����ؼ����벻���жϴ��
#define NO_SYS                  		0  							//ʹ��UCOS����ϵͳ
#define MEM_ALIGNMENT           		4  							//ʹ��4�ֽڶ���ģʽ
#define MEM_SIZE                		6*1024						//�ڴ��heap��С
#define MEMP_NUM_PBUF           		16							//memp�ṹ��pbuf����
#define MEMP_NUM_UDP_PCB        		5							//UDPЭ����ƿ�����.
#define MEMP_NUM_TCP_PCB        		5							//ͬʱ���������TCP����
#define MEMP_NUM_TCP_PCB_LISTEN 		6							//�ܹ�������TCP��������
#define MEMP_NUM_TCP_SEG        		10							//���ͬʱ�ڶ����е�TCP������
#define MEMP_NUM_SYS_TIMEOUT    		8							//�ܹ�ͬʱ�����timeout����


#define PBUF_POOL_SIZE          		20							//pbuf�ڴ�ظ���
#define PBUF_POOL_BUFSIZE     			512							//ÿ��pbuf�ڴ�ش�С

#define LWIP_TCP                		1  							//ʹ��TCP
#define TCP_TTL                			255							

#undef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ         		1 							//��TCP�����ݶγ�������ʱ�Ŀ���λ

#undef TCPIP_MBOX_SIZE
#define TCPIP_MBOX_SIZE         		MAX_QUEUE_ENTRIES   		//tcpip�������߳���Ϣ�����С

#undef DEFAULT_TCP_RECVMBOX_SIZE
#define DEFAULT_TCP_RECVMBOX_SIZE       MAX_QUEUE_ENTRIES  

#undef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         MAX_QUEUE_ENTRIES  

		
#define TCP_MSS                			(1500 - 40)	  				//���TCP�ֶ�
#define TCP_SND_BUF            		 	(2*TCP_MSS)					//TCP���ͻ�������С
#define TCP_SND_QUEUELEN       		 	(4* TCP_SND_BUF/TCP_MSS)	//TCP���ͻ�������С
#define TCP_WND               		  	(2*TCP_MSS)					//TCP���ʹ���
#define LWIP_ICMP             		  	1 							
#define LWIP_DHCP             		  	1							
#define LWIP_UDP              		  	1 							
#define UDP_TTL               		  	255 						
#define LWIP_STATS 						0
#define LWIP_PROVIDE_ERRNO 				1


#define LWIP_NETCONN                    1 							//ʹ��NETCON����
#define LWIP_SOCKET                     1							//ʹ��Sicket API
#define LWIP_COMPAT_MUTEX               1		
#define LWIP_SO_RCVTIMEO                1 							//ʹ��netconn�ṹ����recv_timeout,���Ա��������߳�

//�й�ϵͳ��ѡ��
#define TCPIP_THREAD_PRIO				3							
#define TCPIP_THREAD_STACKSIZE          1000						
#define DEFAULT_UDP_RECVMBOX_SIZE       2000
#define DEFAULT_THREAD_STACKSIZE        512


#define LWIP_DEBUG                    	0	 						
#define ICMP_DEBUG                     	LWIP_DBG_OFF 				

#endif /* __LWIPOPTS_H__ */













