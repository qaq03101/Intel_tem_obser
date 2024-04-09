#include "usmart.h"
#include "usmart_str.h"	   

#include "delay.h"	
#include "usart.h"		
#include "sys.h"
#include "lcd.h"
#include "sram.h"
#include "malloc.h"
#include "dm9000.h"
 
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//如果使能了读写操作
	(void*)read_addr,"u32 read_addr(u32 addr)",
	(void*)write_addr,"void write_addr(u32 addr,u32 val)",	 
#endif
	(void*)delay_ms,"void delay_ms(u16 nms)",
	(void*)delay_us,"void delay_us(u32 nus)",	 
	(void*)DM9000_ReadReg,"u16  DM9000_ReadReg(u16 reg)",
	(void*)DM9000_WriteReg,"void DM9000_WriteReg(u16 reg,u16 data)",
	(void*)DM9000_PHY_ReadReg,"u16  DM9000_PHY_ReadReg(u16 reg)",
	(void*)DM9000_PHY_WriteReg,"void DM9000_PHY_WriteReg(u16 reg,u16 data)",
};						  

//函数控制管理器初始化
//得到各个受控函数的名字
//得到函数总数量
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//函数数量
	0,	  	//参数数量
	0,	 	//函数ID
	1,		//参数显示类型,0,10进制;1,16进制
	0,		//参数类型.bitx:,0,数字;1,字符串	    
	0,	  	//每个参数的长度暂存表,需要MAX_PARM个0初始化
	0,		//函数的参数,需要PARM_LEN个0初始化
};   



















