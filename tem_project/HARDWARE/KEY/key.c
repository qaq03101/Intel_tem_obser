#include "key.h"
#include "includes.h"

								    
//������ʼ������
void KEY_Init(void)
{
	RCC->APB2ENR|=1<<2;     //ʹ��PORTAʱ��

	GPIOA->CRL&=0XFFFFFFF0;	//PA0���ó����룬Ĭ������	  
	GPIOA->CRL|=0X00000008; 
	  

} 





















