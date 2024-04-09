#include "sram.h"	  
#include "usart.h"

					
//使用NOR/SRAM的 Bank1.sector3,地址位HADDR[27,26]=10 
#define Bank1_SRAM3_ADDR    ((u32)(0x68000000))		
  						   
//初始化外部SRAM
void FSMC_SRAM_Init(void)
{	
	RCC->AHBENR|=1<<8; //使能 FSMC 时钟 
	RCC->APB2ENR|=1<<5; //使能 PORTD 时钟
	RCC->APB2ENR|=1<<6; //使能 PORTE 时钟
	RCC->APB2ENR|=1<<7; //使能 PORTF 时钟
	RCC->APB2ENR|=1<<8; //使能 PORTG 时钟
	//PORTD 复用推挽输出
	GPIOD->CRH&=0X00000000;
	GPIOD->CRH|=0XBBBBBBBB; 
	GPIOD->CRL&=0XFF00FF00;
	GPIOD->CRL|=0X00BB00BB; 
	//PORTE 复用推挽输出
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB; 
	GPIOE->CRL&=0X0FFFFF00;
	GPIOE->CRL|=0XB00000BB; 
	//PORTF 复用推挽输出
	GPIOF->CRH&=0X0000FFFF;
	GPIOF->CRH|=0XBBBB0000; 
	GPIOF->CRL&=0XFF000000;
	GPIOF->CRL|=0X00BBBBBB; 
	//PORTG 复用推挽输出 PG10->NE3 
	GPIOG->CRH&=0XFFFFF0FF;
	GPIOG->CRH|=0X00000B00; 
	GPIOG->CRL&=0XFF000000;
	GPIOG->CRL|=0X00BBBBBB; 
	//寄存器清零
	//bank1 有 NE1~4,每一个有一个 BCR+TCR，所以总共八个寄存器。	
	FSMC_Bank1->BTCR[4]=0X00000000;
	FSMC_Bank1->BTCR[5]=0X00000000;
	FSMC_Bank1E->BWTR[4]=0X00000000;
	//操作 BCR 寄存器 使用异步模式,模式 A(
	//BTCR[偶数]:BCR 寄存器;BTCR[奇数]:BTR 寄存器
	FSMC_Bank1->BTCR[4]|=1<<12;//存储器写使能
	FSMC_Bank1->BTCR[4]|=1<<4; //存储器数据宽度为 16bit 
	//操作 BTR 寄存器 
	FSMC_Bank1->BTCR[5]|=1<<8; 
	//数据保持时间（DATAST）为 1 个 HCLK 1/72M=14ns(对 EM 的 SRAM 芯片)
	FSMC_Bank1->BTCR[5]|=0<<4; //地址保持时间（ADDHLD）未用到 
	FSMC_Bank1->BTCR[5]|=0<<0; //地址建立时间（ADDSET）为 0 个 HCLK
	//闪存写时序寄存器 
	FSMC_Bank1E->BWTR[4]=0x0FFFFFFF;//默认值

	FSMC_Bank1->BTCR[4]|=1<<0; 								  
											
}
	  														  
//在指定地址开始,连续写入n个字节.
//pBuffer:字节指针
//WriteAddr:要写入的地址
//n:要写入的字节数
void FSMC_SRAM_WriteBuffer(u8* pBuffer,u32 WriteAddr,u32 n)
{
	for(;n!=0;n--)  
	{										    
		*(vu8*)(Bank1_SRAM3_ADDR+WriteAddr)=*pBuffer;	  
		WriteAddr++; 
		pBuffer++;
	}   
}																			    
//在指定地址开始,连续读出n个字节.
//pBuffer:字节指针
//ReadAddr:要读出的起始地址
//n:要写入的字节数
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
	FSMC_SRAM_WriteBuffer(&data,addr,1);//写入1个字节
}
//读取1个字节
//addr:要读取的地址
//返回值:读取到的数据
u8 fsmc_sram_test_read(u32 addr)
{
	u8 data;
	FSMC_SRAM_ReadBuffer(&data,addr,1);
	return data;
}	











