#include "includes.h"



//---------------��ʼ����
//�������ȼ�
#ifndef START_TASK_PRIO
#define START_TASK_PRIO		3
#endif
//�����ջ��С
#ifndef START_STK_SIZE
#define START_STK_SIZE 		512
#endif


//������
void start_task(void *p_arg);

//---------------LED����
//�������ȼ�
#ifndef LED0_TASK_PRIO
#define LED0_TASK_PRIO		11
#endif
//�����ջ��С
#ifndef LED0_STK_SIZE
#define LED0_STK_SIZE 		128
#endif



void led0_task(void *p_arg);



//---------------dhcp��������
//�������ȼ�
#ifndef DHCP_START_TASK_PRIO
#define DHCP_START_TASK_PRIO	5
#endif
//�����ջ��С
#ifndef DHCP_START_STK_SIZE
#define DHCP_START_STK_SIZE	128
#endif

//������
void dhcp_start_task(void *pdata);


//---------------��LCD����ʾ��Ϣ����
//�������ȼ�
#ifndef DISPLAY_TASK_PRIO
#define DISPLAY_TASK_PRIO	10
#endif
//�����ջ��С
#ifndef DISPLAY_STK_SIZE
#define DISPLAY_STK_SIZE	128
#endif

//������
void display_task(void *pdata);




//---------------����udp����
//�������ȼ�
#ifndef SEND_UDP_TASK_PRIO
#define SEND_UDP_TASK_PRIO	6
#endif
//�����ջ��С
#ifndef SEND_UDP_STK_SIZE
#define SEND_UDP_STK_SIZE	128
#endif

//������
void send_udp_task(void *pdata);



//---------------�ɼ��¶�
//�������ȼ�
#ifndef CAP_TEMP_PRIO
#define CAP_TEMP_PRIO	7
#endif
//�����ջ��С
#ifndef CAP_TEMP_STK_SIZE
#define CAP_TEMP_STK_SIZE	128
#endif

//������
void cap_temp_task(void *pdata);

//---------------����¶�
//�������ȼ�
#ifndef CHECK_TEMP_PRIO
#define CHECK_TEMP_PRIO	7
#endif
//�����ջ��С
#ifndef CHECK_TEMP_STK_SIZE
#define CHECK_TEMP_STK_SIZE	128
#endif

//������
void check_temp_task(void *pdata);

//---------------�����
//�������ȼ�
#ifndef ORDER_HANDLE_PRIO
#define ORDER_HANDLE_PRIO	5
#endif
//�����ջ��С
#ifndef ORDER_HANDLE_STK_SIZE
#define ORDER_HANDLE_STK_SIZE	128
#endif

//������
void order_handle_task(void *pdata);
void suspend_task(void);
void recovery_task(void);





extern OS_TCB StartTaskTCB;
extern CPU_STK START_TASK_STK[START_STK_SIZE];

extern OS_TCB DhcpStartTCB;
extern CPU_STK DHCP_START_TASK_STK[DHCP_START_STK_SIZE];

extern OS_TCB Led0TaskTCB;
extern CPU_STK LED0_TASK_STK[LED0_STK_SIZE];

extern OS_Q ORDER_Msg;;
extern OS_Q DATA_Msg;;
extern OS_SEM SYN;

extern short LIMIT_MAX;
extern short LIMIT_MIN;
extern int isDormancy;