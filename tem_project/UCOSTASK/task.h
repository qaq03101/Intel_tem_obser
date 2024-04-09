#include "includes.h"



//---------------开始任务
//任务优先级
#ifndef START_TASK_PRIO
#define START_TASK_PRIO		3
#endif
//任务堆栈大小
#ifndef START_STK_SIZE
#define START_STK_SIZE 		512
#endif


//任务函数
void start_task(void *p_arg);

//---------------LED任务
//任务优先级
#ifndef LED0_TASK_PRIO
#define LED0_TASK_PRIO		11
#endif
//任务堆栈大小
#ifndef LED0_STK_SIZE
#define LED0_STK_SIZE 		128
#endif



void led0_task(void *p_arg);



//---------------dhcp启动任务
//任务优先级
#ifndef DHCP_START_TASK_PRIO
#define DHCP_START_TASK_PRIO	5
#endif
//任务堆栈大小
#ifndef DHCP_START_STK_SIZE
#define DHCP_START_STK_SIZE	128
#endif

//任务函数
void dhcp_start_task(void *pdata);


//---------------在LCD上显示信息任务
//任务优先级
#ifndef DISPLAY_TASK_PRIO
#define DISPLAY_TASK_PRIO	10
#endif
//任务堆栈大小
#ifndef DISPLAY_STK_SIZE
#define DISPLAY_STK_SIZE	128
#endif

//任务函数
void display_task(void *pdata);




//---------------发送udp任务
//任务优先级
#ifndef SEND_UDP_TASK_PRIO
#define SEND_UDP_TASK_PRIO	6
#endif
//任务堆栈大小
#ifndef SEND_UDP_STK_SIZE
#define SEND_UDP_STK_SIZE	128
#endif

//任务函数
void send_udp_task(void *pdata);



//---------------采集温度
//任务优先级
#ifndef CAP_TEMP_PRIO
#define CAP_TEMP_PRIO	7
#endif
//任务堆栈大小
#ifndef CAP_TEMP_STK_SIZE
#define CAP_TEMP_STK_SIZE	128
#endif

//任务函数
void cap_temp_task(void *pdata);

//---------------检测温度
//任务优先级
#ifndef CHECK_TEMP_PRIO
#define CHECK_TEMP_PRIO	7
#endif
//任务堆栈大小
#ifndef CHECK_TEMP_STK_SIZE
#define CHECK_TEMP_STK_SIZE	128
#endif

//任务函数
void check_temp_task(void *pdata);

//---------------命令处理
//任务优先级
#ifndef ORDER_HANDLE_PRIO
#define ORDER_HANDLE_PRIO	5
#endif
//任务堆栈大小
#ifndef ORDER_HANDLE_STK_SIZE
#define ORDER_HANDLE_STK_SIZE	128
#endif

//任务函数
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