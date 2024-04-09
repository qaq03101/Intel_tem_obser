#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

	


#define KEY_UP 	PAin(0)	//PA0  WK_UP即KEY_UP

#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)


void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数					    
#endif
