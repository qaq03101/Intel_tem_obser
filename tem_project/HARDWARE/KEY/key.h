#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

	


#define KEY_UP 	PAin(0)	//PA0  WK_UP��KEY_UP

#define WKUP_PRES   4	//KEY_UP����(��WK_UP/KEY_UP)


void KEY_Init(void);//IO��ʼ��
u8 KEY_Scan(u8);  	//����ɨ�躯��					    
#endif
