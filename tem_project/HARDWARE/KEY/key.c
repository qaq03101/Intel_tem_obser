#include "key.h"
#include "includes.h"

								    
//按键初始化函数
void KEY_Init(void)
{
	RCC->APB2ENR|=1<<2;     //使能PORTA时钟

	GPIOA->CRL&=0XFFFFFFF0;	//PA0设置成输入，默认下拉	  
	GPIOA->CRL|=0X00000008; 
	  

} 





















