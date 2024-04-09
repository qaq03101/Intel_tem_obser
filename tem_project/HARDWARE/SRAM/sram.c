#include "sram.h"	  
#include "usart.h"

					
//ʹ��NOR/SRAM�� Bank1.sector3,��ַλHADDR[27,26]=10 
#define Bank1_SRAM3_ADDR    ((u32)(0x68000000))		
  						   
//��ʼ���ⲿSRAM
void FSMC_SRAM_Init(void)
{	
	RCC->AHBENR|=1<<8; //ʹ�� FSMC ʱ�� 
	RCC->APB2ENR|=1<<5; //ʹ�� PORTD ʱ��
	RCC->APB2ENR|=1<<6; //ʹ�� PORTE ʱ��
	RCC->APB2ENR|=1<<7; //ʹ�� PORTF ʱ��
	RCC->APB2ENR|=1<<8; //ʹ�� PORTG ʱ��
	//PORTD �����������
	GPIOD->CRH&=0X00000000;
	GPIOD->CRH|=0XBBBBBBBB; 
	GPIOD->CRL&=0XFF00FF00;
	GPIOD->CRL|=0X00BB00BB; 
	//PORTE �����������
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB; 
	GPIOE->CRL&=0X0FFFFF00;
	GPIOE->CRL|=0XB00000BB; 
	//PORTF �����������
	GPIOF->CRH&=0X0000FFFF;
	GPIOF->CRH|=0XBBBB0000; 
	GPIOF->CRL&=0XFF000000;
	GPIOF->CRL|=0X00BBBBBB; 
	//PORTG ����������� PG10->NE3 
	GPIOG->CRH&=0XFFFFF0FF;
	GPIOG->CRH|=0X00000B00; 
	GPIOG->CRL&=0XFF000000;
	GPIOG->CRL|=0X00BBBBBB; 
	//�Ĵ�������
	//bank1 �� NE1~4,ÿһ����һ�� BCR+TCR�������ܹ��˸��Ĵ�����	
	FSMC_Bank1->BTCR[4]=0X00000000;
	FSMC_Bank1->BTCR[5]=0X00000000;
	FSMC_Bank1E->BWTR[4]=0X00000000;
	//���� BCR �Ĵ��� ʹ���첽ģʽ,ģʽ A(
	//BTCR[ż��]:BCR �Ĵ���;BTCR[����]:BTR �Ĵ���
	FSMC_Bank1->BTCR[4]|=1<<12;//�洢��дʹ��
	FSMC_Bank1->BTCR[4]|=1<<4; //�洢�����ݿ��Ϊ 16bit 
	//���� BTR �Ĵ��� 
	FSMC_Bank1->BTCR[5]|=1<<8; 
	//���ݱ���ʱ�䣨DATAST��Ϊ 1 �� HCLK 1/72M=14ns(�� EM �� SRAM оƬ)
	FSMC_Bank1->BTCR[5]|=0<<4; //��ַ����ʱ�䣨ADDHLD��δ�õ� 
	FSMC_Bank1->BTCR[5]|=0<<0; //��ַ����ʱ�䣨ADDSET��Ϊ 0 �� HCLK
	//����дʱ��Ĵ��� 
	FSMC_Bank1E->BWTR[4]=0x0FFFFFFF;//Ĭ��ֵ

	FSMC_Bank1->BTCR[4]|=1<<0; 								  
											
}
	  														  
//��ָ����ַ��ʼ,����д��n���ֽ�.
//pBuffer:�ֽ�ָ��
//WriteAddr:Ҫд��ĵ�ַ
//n:Ҫд����ֽ���
void FSMC_SRAM_WriteBuffer(u8* pBuffer,u32 WriteAddr,u32 n)
{
	for(;n!=0;n--)  
	{										    
		*(vu8*)(Bank1_SRAM3_ADDR+WriteAddr)=*pBuffer;	  
		WriteAddr++; 
		pBuffer++;
	}   
}																			    
//��ָ����ַ��ʼ,��������n���ֽ�.
//pBuffer:�ֽ�ָ��
//ReadAddr:Ҫ��������ʼ��ַ
//n:Ҫд����ֽ���
void FSMC_SRAM_ReadBuffer(u8* pBuffer,u32 ReadAddr,u32 n)
{
	for(;n!=0;n--)  
	{											    
		*pBuffer++=*(vu8*)(Bank1_SRAM3_ADDR+ReadAddr);    
		ReadAddr++; 
	}  
} 

void fsmc_sram_test_write(u8 data,u32 addr)
{			   
	FSMC_SRAM_WriteBuffer(&data,addr,1);//д��1���ֽ�
}
//��ȡ1���ֽ�
//addr:Ҫ��ȡ�ĵ�ַ
//����ֵ:��ȡ��������
u8 fsmc_sram_test_read(u32 addr)
{
	u8 data;
	FSMC_SRAM_ReadBuffer(&data,addr,1);
	return data;
}	











