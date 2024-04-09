#include "dm9000.h"
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "lwip_comm.h"
#include "os.h"


struct dm9000_config dm9000cfg;				//DM9000配置结构体

extern OS_SEM dm9000input;				//DM9000接收数据信号量
extern OS_MUTEX dm9000lock;				//DM9000读写互锁控制信号量

//初始化DM9000
//返回值:
//0,初始化成功
//1，DM9000A ID读取错误
u8 DM9000_Init(void)
{
		u32 temp; 
	//先初始化和DM9000连接的IO和FSMC
	RCC->AHBENR|=1<<8;     	 				
	RCC->APB2ENR|=1<<5;     				
	RCC->APB2ENR|=1<<6;     				
	RCC->APB2ENR|=1<<7;     				
 	RCC->APB2ENR|=1<<8;						
	//PORTD复用推挽输出 	
	GPIOD->CRH&=0X00FFF000;
	GPIOD->CRH|=0XBB000BBB; 
	GPIOD->CRL&=0X0F00FF00;					
	GPIOD->CRL|=0X30BB00BB;   	 
	//PORTE复用推挽输出 	
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB; 
	GPIOE->CRL&=0X0FFFFFFF;
	GPIOE->CRL|=0XB0000000;   
	//PORTF复用推挽输出 	PF13-->FSMC_A7
	GPIOF->CRH&=0XFF0FFFFF;
	GPIOF->CRH|=0X00B00000;   	
	//PORTG复用推挽输出 PG9->NE2     	 											 
	GPIOG->CRH&=0XFFFFFF0F;
	GPIOG->CRH|=0X000000B0;  
	
	GPIOG->CRL&=0XF0FFFFFF;					
	GPIOG->CRL|=0X08000000;   
	GPIOG->ODR|=1<<6;						
	Ex_NVIC_Config(GPIO_G,6,FTIR);			
	MY_NVIC_Init(0,0,EXTI9_5_IRQn,2);		
	//寄存器清零
			    
	FSMC_Bank1->BTCR[2]=0X00000000;
	FSMC_Bank1->BTCR[3]=0X00000000;
	FSMC_Bank1E->BWTR[2]=0X00000000;
	//操作BCR寄存器	使用异步模式,模式A(读写共用一个时序寄存器)
	//BTCR[偶数]:BCR寄存器;BTCR[奇数]:BTR寄存器
	FSMC_Bank1->BTCR[2]|=1<<12;				
	FSMC_Bank1->BTCR[2]|=1<<4; 				  
	//操作BTR寄存器								    
	FSMC_Bank1->BTCR[3]|=3<<8; 				 	 
	FSMC_Bank1->BTCR[3]|=0<<4; 				 	 
	FSMC_Bank1->BTCR[3]|=1<<0;				 
	//闪存写时序寄存器  
	FSMC_Bank1E->BWTR[2]=0x0FFFFFFF;		
	//使能BANK1区域2
	FSMC_Bank1->BTCR[2]|=1<<0; 
	
	temp=*(vu32*)(0x1FFFF7E8);				//获取STM32的唯一ID的前24位作为MAC地址后三字节
	dm9000cfg.mode=DM9000_AUTO;	
 	dm9000cfg.queue_packet_len=0;
	//DM9000的SRAM的发送和接收指针自动返回到开始地址，并且开启接收中断
	dm9000cfg.imr_all = IMR_PAR|IMR_PRI; 
	//初始化MAC地址
	dm9000cfg.mac_addr[0]=2;
	dm9000cfg.mac_addr[1]=0;
	dm9000cfg.mac_addr[2]=0;
	dm9000cfg.mac_addr[3]=(temp>>16)&0XFF;	//低三字节用STM32的唯一ID
	dm9000cfg.mac_addr[4]=(temp>>8)&0XFFF;
	dm9000cfg.mac_addr[5]=temp&0XFF;
	//初始化组播地址
	dm9000cfg.multicase_addr[0]=0Xff;
	dm9000cfg.multicase_addr[1]=0Xff;
	dm9000cfg.multicase_addr[2]=0Xff;
	dm9000cfg.multicase_addr[3]=0Xff;
	dm9000cfg.multicase_addr[4]=0Xff;
	dm9000cfg.multicase_addr[5]=0Xff;
	dm9000cfg.multicase_addr[6]=0Xff;
	dm9000cfg.multicase_addr[7]=0Xff; 
	
	DM9000_Reset();							//复位DM9000
	delay_ms(100);
	temp=DM9000_Get_DeiviceID();			//获取DM9000ID
	printf("DM9000 ID:%#x\r\n",temp);
	if(temp!=DM9000_ID) return 1; 			//读取ID错误
	DM9000_Set_PHYMode(dm9000cfg.mode);		//设置PHY工作模式
	
	DM9000_WriteReg(DM9000_NCR,0X00);
	DM9000_WriteReg(DM9000_TCR,0X00);		//发送控制寄存器清零
	DM9000_WriteReg(DM9000_BPTR,0X3F);	
	DM9000_WriteReg(DM9000_FCTR,0X38);
	DM9000_WriteReg(DM9000_FCR,0X00);
	DM9000_WriteReg(DM9000_SMCR,0X00);		//特殊模式
	DM9000_WriteReg(DM9000_NSR,NSR_WAKEST|NSR_TX2END|NSR_TX1END);//清除发送状态
	DM9000_WriteReg(DM9000_ISR,0X0F);		//清除中断状态
	DM9000_WriteReg(DM9000_TCR2,0X80);		//切换LED到mode1 	
	//设置MAC地址和组播地址
	DM9000_Set_MACAddress(dm9000cfg.mac_addr);		//设置MAC地址
	DM9000_Set_Multicast(dm9000cfg.multicase_addr);	//设置组播地址
	DM9000_WriteReg(DM9000_RCR,RCR_DIS_LONG|RCR_DIS_CRC|RCR_RXEN);
	DM9000_WriteReg(DM9000_IMR,IMR_PAR); 
	temp=DM9000_Get_SpeedAndDuplex();		//获取DM9000的连接速度和双工状态
	if(temp!=0XFF)							//连接成功，通过串口显示连接速度和双工状态
	{
		printf("DM9000 Speed:%dMbps,Duplex:%s duplex mode\r\n",(temp&0x02)?10:100,(temp&0x01)?"Full":"Half");
	}else printf("DM9000 Establish Link Failed!\r\n");
	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);	//设置中断
	return 0;	

} 
//外部中断线6的中断服务函数(放exti.c)
void EXTI9_5_IRQHandler(void)
{
	OSIntEnter();    
	EXTI->PR=1<<6;  			//清除LINE6上的中断标志位   
	while(DM9000_INT==0)
	{
		DMA9000_ISRHandler();
	}
	OSIntExit();  		
} 

//读取DM9000指定寄存器的值
//reg:寄存器地址
//返回值：DM9000指定寄存器的值
u16 DM9000_ReadReg(u16 reg)
{
	DM9000->REG=reg;
	return DM9000->DATA; 
}
//向DM9000指定寄存器中写入指定值
//reg:写入的寄存器
//data:写入的值
void DM9000_WriteReg(u16 reg,u16 data)
{
	DM9000->REG=reg;
	DM9000->DATA=data;
}
//读取DM9000的PHY的指定寄存器
//reg:要读的PHY寄存器
//返回值:读取到的PHY寄存器值
u16 DM9000_PHY_ReadReg(u16 reg)
{
	u16 temp;
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPCR,0X0C);				
	delay_ms(10);
	DM9000_WriteReg(DM9000_EPCR,0X00);				
	temp=(DM9000_ReadReg(DM9000_EPDRH)<<8)|(DM9000_ReadReg(DM9000_EPDRL));
	return temp;
}
//向DM9000的PHY寄存器写入指定值
//reg:PHY寄存器
//data:要写入的值
void DM9000_PHY_WriteReg(u16 reg,u16 data)
{
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPDRL,(data&0xff));		
	DM9000_WriteReg(DM9000_EPDRH,((data>>8)&0xff));	
	DM9000_WriteReg(DM9000_EPCR,0X0A);				
	delay_ms(50);
	DM9000_WriteReg(DM9000_EPCR,0X00);				
} 
//获取DM9000的芯片ID
//返回值：DM9000的芯片ID值
u32 DM9000_Get_DeiviceID(void)
{
	u32 value;
	value =DM9000_ReadReg(DM9000_VIDL);
	value|=DM9000_ReadReg(DM9000_VIDH) << 8;
	value|=DM9000_ReadReg(DM9000_PIDL) << 16;
	value|=DM9000_ReadReg(DM9000_PIDH) << 24;
	return value;
}
//获取DM9000的连接速度和双工模式
//返回值：	0,100M半双工
//			1,100M全双工
//			2,10M半双工
//			3,10M全双工
//			0XFF,连接失败！
u8 DM9000_Get_SpeedAndDuplex(void)
{
	u8 temp;
	u8 i=0;	
	if(dm9000cfg.mode==DM9000_AUTO)					//如果开启了自动协商模式一定要等待协商完成
	{
		while(!(DM9000_PHY_ReadReg(0X01)&0X0020))	//等待自动协商完成
		{
			delay_ms(100);					
			i++;
			if(i>100)return 0XFF;					//自动协商失败
		}	
	}else											//自定义模式,一定要等待连接成功
	{
		while(!(DM9000_ReadReg(DM9000_NSR)&0X40))	//等待连接成功
		{
			delay_ms(100);					
			i++;
			if(i>100)return 0XFF;					//连接失败			
		}
	}
	temp =((DM9000_ReadReg(DM9000_NSR)>>6)&0X02);	//获取DM9000的连接速度
	temp|=((DM9000_ReadReg(DM9000_NCR)>>3)&0X01);	//获取DM9000的双工状态
	return temp;
}

//设置DM900的PHY工作模式
//mode:PHY模式
void DM9000_Set_PHYMode(u8 mode)
{
	u16 BMCR_Value,ANAR_Value;	
	switch(mode)
	{
		case DM9000_10MHD:		//10M半双工
			BMCR_Value=0X0000;
			ANAR_Value=0X21;
			break;
		case DM9000_10MFD:		//10M全双工
			BMCR_Value=0X0100;
			ANAR_Value=0X41;
			break;
		case DM9000_100MHD:		//100M半双工
			BMCR_Value=0X2000;
			ANAR_Value=0X81;
			break;
		case DM9000_100MFD:		//100M全双工
			BMCR_Value=0X2100;
			ANAR_Value=0X101;
			break;
		case DM9000_AUTO:		//自动协商模式
			BMCR_Value=0X1000;
			ANAR_Value=0X01E1;
			break;		
	}
	DM9000_PHY_WriteReg(DM9000_PHY_BMCR,BMCR_Value);
	DM9000_PHY_WriteReg(DM9000_PHY_ANAR,ANAR_Value);
 	DM9000_WriteReg(DM9000_GPR,0X00);	//使能PHY
}
//设置DM9000的MAC地址
//macaddr:指向MAC地址
void DM9000_Set_MACAddress(u8 *macaddr)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		DM9000_WriteReg(DM9000_PAR+i,macaddr[i]);
	}
}
//设置DM9000的组播地址
//multicastaddr:指向多播地址
void DM9000_Set_Multicast(u8 *multicastaddr)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		DM9000_WriteReg(DM9000_MAR+i,multicastaddr[i]);
	}
}  
//复位DM9000
void DM9000_Reset(void)
{

	DM9000_RST = 0;								//DM9000硬件复位
	delay_ms(10);
	DM9000_RST = 1; 							//DM9000硬件复位结束
	delay_ms(100);								
 	DM9000_WriteReg(DM9000_GPCR,0x01);			
	DM9000_WriteReg(DM9000_GPR,0);				
 	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	
	do 
	{
		delay_ms(25); 	
	}while(DM9000_ReadReg(DM9000_NCR)&1);		
	DM9000_WriteReg(DM9000_NCR,0);
	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	
	do 
	{
		delay_ms(25);	
	}while (DM9000_ReadReg(DM9000_NCR)&1);
}  

//通过DM9000发送数据包
//p:pbuf结构体指针
void DM9000_SendPacket(struct pbuf *p)
{
	struct pbuf *q;
	u16 pbuf_index = 0;
	u8 word[2], word_index = 0;
	OS_ERR err;	
 	//printf("send len:%d\r\n",p->tot_len);
	
	OSMutexPend((OS_MUTEX *)&dm9000lock,(OS_TICK)0,OS_OPT_POST_NONE,0,&err); 			
	DM9000_WriteReg(DM9000_IMR,IMR_PAR);		//关闭网卡中断 
	DM9000->REG=DM9000_MWCMD;					
	q=p;

 	while(q)
	{
		if (pbuf_index < q->len)
		{
			word[word_index++] = ((u8_t*)q->payload)[pbuf_index++];
			if (word_index == 2)
			{
				DM9000->DATA=((u16)word[1]<<8)|word[0];
				word_index = 0;
			}
		}else
		{
			q=q->next;
			pbuf_index = 0;
		}
	}
	//还有一个字节未写入TX SRAM
	if(word_index==1)DM9000->DATA=word[0];
	//向DM9000写入发送长度
	DM9000_WriteReg(DM9000_TXPLL,p->tot_len&0XFF);
	DM9000_WriteReg(DM9000_TXPLH,(p->tot_len>>8)&0XFF);		
	DM9000_WriteReg(DM9000_TCR,0X01);						
	while((DM9000_ReadReg(DM9000_ISR)&0X02)==0);			
	DM9000_WriteReg(DM9000_ISR,0X02);						
 	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);			
	OSMutexPost((OS_MUTEX *)&dm9000lock,OS_OPT_POST_NONE,&err);								//发送互斥信号量,解锁DM9000
}
//DM9000接收数据包
//byte1:表明是否接收到数据，为0x00或者0X01，如果两个都不是的话一定要软件复位DM9000
//		0x01，接收到数据
//		0x00，未接收到数据
//byte2:第二个字节表示一些状态信息，和DM9000的RSR(0X06)寄存器一致的
//byte3:本帧数据长度的低字节
//byte4:本帧数据长度的高字节
//返回值：pbuf格式的接收到的数据包
struct pbuf *DM9000_Receive_Packet(void)
{
	struct pbuf* p;
	struct pbuf* q;
    u32 rxbyte;
	vu16 rx_status, rx_length;
    u16* data;
	u16 dummy; 
	int len;
	OS_ERR err;

	p=NULL; 
	OSMutexPend((OS_MUTEX*)&dm9000lock,0,OS_OPT_PEND_BLOCKING,0,&err); 				//请求互斥信号量,锁定DM9000
__error_retry:	
	DM9000_ReadReg(DM9000_MRCMDX);					//假读
	rxbyte=(u8)DM9000->DATA;						//进行第二次读取 
	if(rxbyte)										//接收到数据
	{
		if(rxbyte>1)								//rxbyte大于1，接收到的数据错误,挂了		
		{
            printf("dm9000 rx: rx error, stop device\r\n");
			DM9000_WriteReg(DM9000_RCR,0x00);
			DM9000_WriteReg(DM9000_ISR,0x80);		 
			return (struct pbuf*)p;
		}
		DM9000->REG=DM9000_MRCMD;
		rx_status=DM9000->DATA;
        rx_length=DM9000->DATA;  
		//if(rx_length>512)printf("rxlen:%d\r\n",rx_length);
        p=pbuf_alloc(PBUF_RAW,rx_length,PBUF_POOL);	//pbufs内存池分配pbuf
		if(p!=NULL)									//内存申请成功
        {
            for(q=p;q!=NULL;q=q->next)
            {
                data=(u16*)q->payload;
                len=q->len;
                while(len>0)
                {
					*data=DM9000->DATA;
                    data++;
                    len-= 2;
                }
            }
        }else										//内存申请失败
		{
			printf("pbuf内存申请失败:%d\r\n",rx_length);
            data=&dummy;
			len=rx_length;
			while(len)
			{
				*data=DM9000->DATA;
				len-=2;
			}
        }	
		//根据rx_status判断接收数据是否出现如下错误：FIFO溢出、CRC错误
		if((rx_status&0XBF00) || (rx_length < 0X40) || (rx_length > DM9000_PKT_MAX))
		{
			printf("rx_status:%#x\r\n",rx_status);
			if (rx_status & 0x100)printf("rx fifo error\r\n");
            if (rx_status & 0x200)printf("rx crc error\r\n");
            if (rx_status & 0x8000)printf("rx length error\r\n");
            if (rx_length>DM9000_PKT_MAX)
			{
				printf("rx length too big\r\n");
				DM9000_WriteReg(DM9000_NCR, NCR_RST); 	//复位DM9000
				delay_ms(5);
			}
			if(p!=NULL)pbuf_free((struct pbuf*)p);		//释放内存
			p=NULL;
			goto __error_retry;
		}
	}else
    {
        DM9000_WriteReg(DM9000_ISR,ISR_PTS);			//清除所有中断标志位
        dm9000cfg.imr_all=IMR_PAR|IMR_PRI;				//重新接收中断 
        DM9000_WriteReg(DM9000_IMR, dm9000cfg.imr_all);
    } 
	OSMutexPost((OS_MUTEX*)&dm9000lock,OS_OPT_POST_NONE,&err);							//发送互斥信号量,解锁DM9000
	return (struct pbuf*)p; 
}
//中断处理函数
void DMA9000_ISRHandler(void)
{
	u16 int_status;
	u16 last_io; 
	OS_ERR err;
	last_io = DM9000->REG;
	int_status=DM9000_ReadReg(DM9000_ISR); 
	DM9000_WriteReg(DM9000_ISR,int_status);				//清除中断标志位，DM9000的ISR寄存器的bit0~bit5写1清零
	if(int_status & ISR_ROS)printf("overflow \r\n");
    if(int_status & ISR_ROOS)printf("overflow counter overflow \r\n");	
	if(int_status & ISR_PRS)		//接收中断
	{  
 		OSSemPost((OS_SEM*)&dm9000input,OS_OPT_POST_ALL,&err);		//处理接收到数据帧 
	} 
	if(int_status&ISR_PTS)			//发送中断
	{ 
									//接收中断处理,这里没用到
	}
	DM9000->REG=last_io;	
}


































