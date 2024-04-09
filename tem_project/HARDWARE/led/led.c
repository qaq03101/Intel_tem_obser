#include "led.h"
//初始化 PB5 和 PE5 为输出口.并使能这两个口的时钟 
//LED IO 初始化
void LED_Init(void)
{
RCC->APB2ENR|=1<<3; //使能 PORTB 时钟 
RCC->APB2ENR|=1<<6; //使能 PORTE 时钟 
GPIOB->CRL&=0XFF0FFFFF; 
GPIOB->CRL|=0X00300000;//PB.5 推挽输出 
 GPIOB->ODR|=1<<5; //PB.5 输出高 
GPIOE->CRL&=0XFF0FFFFF;
GPIOE->CRL|=0X00300000;//PE.5 推挽输出
GPIOE->ODR|=1<<5; //PE.5 输出高
}
