#include "led.h"
//��ʼ�� PB5 �� PE5 Ϊ�����.��ʹ���������ڵ�ʱ�� 
//LED IO ��ʼ��
void LED_Init(void)
{
RCC->APB2ENR|=1<<3; //ʹ�� PORTB ʱ�� 
RCC->APB2ENR|=1<<6; //ʹ�� PORTE ʱ�� 
GPIOB->CRL&=0XFF0FFFFF; 
GPIOB->CRL|=0X00300000;//PB.5 ������� 
 GPIOB->ODR|=1<<5; //PB.5 ����� 
GPIOE->CRL&=0XFF0FFFFF;
GPIOE->CRL|=0X00300000;//PE.5 �������
GPIOE->ODR|=1<<5; //PE.5 �����
}
