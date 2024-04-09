#define SYS_ARCH_GLOBALS

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "includes.h"
#include "delay.h"
#include "arch/sys_arch.h"
#include "malloc.h"
#include "os_cfg_app.h"

//����lwip�еĿ���Ϣ
const void *const pvNullPointer = (mem_ptr_t *)0xffffffff;

// ����һ����Ϣ����
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	OS_ERR err;
	if (size > MAX_QUEUE_ENTRIES)
		size = MAX_QUEUE_ENTRIES;
	OSQCreate((OS_Q *)mbox,				
			  (CPU_CHAR *)"LWIP Quiue", 
			  (OS_MSG_QTY)size,			
			  (OS_ERR *)&err);			
	if (err == OS_ERR_NONE)
		return ERR_OK;
	return ERR_MEM;
}

// �ͷŲ�ɾ��һ����Ϣ����
void sys_mbox_free(sys_mbox_t *mbox)
{
	OS_ERR err;
#if OS_CFG_Q_FLUSH_EN > 0u
	OSQFlush(mbox, &err);
#endif
	OSQDel((OS_Q *)mbox,
		   (OS_OPT)OS_OPT_DEL_ALWAYS,
		   (OS_ERR *)&err);
	LWIP_ASSERT("OSQDel ", err == OS_ERR_NONE);
}

// ����Ϣ�����з���һ����Ϣ
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	OS_ERR err;
	CPU_INT08U i = 0;

	if (msg == NULL) //��ϢΪ��,���滻��lwip���ض���nullֵ
		msg = (void *)&pvNullPointer;

	while (i < 10) 
	{
		OSQPost((OS_Q *)mbox,
				(void *)msg,
				(OS_MSG_SIZE)strlen(msg),
				(OS_OPT)OS_OPT_POST_FIFO,
				(OS_ERR *)&err);
		if (err == OS_ERR_NONE)
			break;
		i++;
		OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_HMSM_STRICT, &err); // ��ʱ 5ms
	}
	LWIP_ASSERT("sys_mbox_post error!\n", i != 10);
}


err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	OS_ERR err;

	if (msg == NULL)
		msg = (void *)&pvNullPointer;
	OSQPost((OS_Q *)mbox,
			(void *)msg,
			(OS_MSG_SIZE)sizeof(msg),
			(OS_OPT)OS_OPT_POST_FIFO,
			(OS_ERR *)&err);
	if (err != OS_ERR_NONE)
		return ERR_MEM;
	return ERR_OK;
}

// �ȴ���Ϣ�����е���Ϣ
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	OS_ERR err;
	OS_MSG_SIZE size;
	u32_t ucos_timeout, timeout_new;
	void *temp;
	if (timeout != 0)
	{
		ucos_timeout = (timeout * OS_CFG_TICK_RATE_HZ) / 1000;  //ת��Ϊ����
		if (ucos_timeout < 1)
			ucos_timeout = 1; // ���� 1 ������
	}
	else
		ucos_timeout = 0;
	timeout = OSTimeGet(&err); // ��ȡϵͳʱ��

	temp = OSQPend((OS_Q *)mbox,
				   (OS_TICK)ucos_timeout,
				   (OS_OPT)OS_OPT_PEND_BLOCKING,
				   (OS_MSG_SIZE *)&size,
				   (CPU_TS *)0,
				   (OS_ERR *)&err);
	if (msg != NULL)
	{
		
		if (temp == (void *)&pvNullPointer)
			*msg = NULL;
		else
			*msg = temp;
	}
	if (err == OS_ERR_TIMEOUT)
		timeout = SYS_ARCH_TIMEOUT; // ����ʱ
	else
	{
		LWIP_ASSERT("OSQPend ", err == OS_ERR_NONE);
		timeout_new = OSTimeGet(&err);

		if (timeout_new >= timeout)
			timeout_new = timeout_new - timeout;
		else
			timeout_new = 0xffffffff - timeout + timeout_new;
		timeout = timeout_new * 1000 / OS_CFG_TICK_RATE_HZ + 1;
	}
	return timeout;  //����������Ϣ����ʱ��
}

// ���Ի�ȡ��Ϣ
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox, msg, 1); 
}
// ���һ����Ϣ�����Ƿ���Ч
int sys_mbox_valid(sys_mbox_t *mbox)
{
	if (mbox->NamePtr)
		return (strcmp(mbox->NamePtr, "?Q")) ? 1 : 0;
	else
		return 0;
}
// ����һ����Ϣ����Ϊ��Ч
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	if (sys_mbox_valid(mbox))
		sys_mbox_free(mbox);
}
// ����һ���ź���
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	OS_ERR err;
	OSSemCreate((OS_SEM *)sem,
				(CPU_CHAR *)"LWIP Sem",
				(OS_SEM_CTR)count,
				(OS_ERR *)&err);
	if (err != OS_ERR_NONE)
		return ERR_MEM;
	LWIP_ASSERT("OSSemCreate ", sem != NULL);
	return ERR_OK;
}
// �ȴ�һ���ź���
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	OS_ERR err;
	u32_t ucos_timeout, timeout_new;
	if (timeout != 0)
	{
		ucos_timeout = (timeout * OS_CFG_TICK_RATE_HZ) / 1000;
		if (ucos_timeout < 1)
			ucos_timeout = 1;
	}
	else
		ucos_timeout = 0;
	timeout = OSTimeGet(&err);
	OSSemPend(sem, ucos_timeout, OS_OPT_PEND_BLOCKING, 0, &err); // �����ź���
	if (err == OS_ERR_TIMEOUT)
		timeout = SYS_ARCH_TIMEOUT; // ����ʱ
	else
	{
		timeout_new = OSTimeGet(&err);
		if (timeout_new >= timeout)
			timeout_new = timeout_new - timeout;
		else
			timeout_new = 0xffffffff - timeout + timeout_new;
		// ���������Ϣ��ʹ�õ�ʱ��
		timeout = (timeout_new * 1000 / OS_CFG_TICK_RATE_HZ + 1);
	}
	return timeout;
}
// ����һ���ź���
void sys_sem_signal(sys_sem_t *sem)
{
	OS_ERR err;
	OSSemPost(sem, OS_OPT_POST_1, &err); // �����ź���
	LWIP_ASSERT("OSSemPost ", err == OS_ERR_NONE);
}
// �ͷŲ�ɾ��һ���ź���
void sys_sem_free(sys_sem_t *sem)
{
	OS_ERR err;
	OSSemDel(sem, OS_OPT_DEL_ALWAYS, &err);
	LWIP_ASSERT("OSSemDel ", err == OS_ERR_NONE);
	sem = NULL;
}
// ��ѯһ���ź�����״̬,��Ч����Ч
int sys_sem_valid(sys_sem_t *sem)
{
	if (sem->NamePtr)
		return (strcmp(sem->NamePtr, "?SEM")) ? 1 : 0;
	else
		return 0;
}
// ����һ���ź�����Ч
void sys_sem_set_invalid(sys_sem_t *sem)
{
	if (sys_sem_valid(sem))
		sys_sem_free(sem);
}
// arch ��ʼ��
void sys_init(void)
{
	//�����ر���
}
extern CPU_STK *TCPIP_THREAD_TASK_STK; // TCP IP �ں������ջ
OS_TCB TcpipthreadTaskTCB;

// LWIPʹ�õĴ���һ��������
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	if (strcmp(name, TCPIP_THREAD_NAME) == 0) 
	{
		OS_CRITICAL_ENTER(); 

		OSTaskCreate((OS_TCB *)&TcpipthreadTaskTCB,	 
					 (CPU_CHAR *)"TCPIPThread task", 
					 (OS_TASK_PTR)thread,			 
					 (void *)0,						
					 (OS_PRIO)prio,					
					 (CPU_STK *)&TCPIP_THREAD_TASK_STK[0],
					 (CPU_STK_SIZE)stacksize / 10, 
					 (CPU_STK_SIZE)stacksize,	   
					 (OS_MSG_QTY)0,
					 (OS_TICK)0,
					 (void *)0, 
					 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
					 (OS_ERR *)&err); 
		OS_CRITICAL_EXIT();			  
	}
	return 0;
}
// lwip ��ʱ����
void sys_msleep(u32_t ms)
{
	delay_ms(ms);
}
// ��ȡϵͳʱ��
u32_t sys_now(void)
{
	OS_ERR err;
	u32_t ucos_time, lwip_time;
	ucos_time = OSTimeGet(&err);							 
	lwip_time = (ucos_time * 1000 / OS_CFG_TICK_RATE_HZ + 1); 
	return lwip_time;										  
}