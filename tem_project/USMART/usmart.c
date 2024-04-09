#include "usmart.h"
#include "usart.h"
#include "sys.h"

//系统命令
u8 *sys_cmd_tab[]=
{
	"?",
	"help",
	"list",
	"id",
	"hex",
	"dec",
	"runtime",	   
};	    
//处理系统指令
//0,成功处理;其他,错误代码;
u8 usmart_sys_cmd_exe(u8 *str)
{
	u8 i;
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	u8 pnum;
	u8 rval;
	u32 res;  
	res=usmart_get_cmdname(str,sfname,&i,MAX_FNAME_LEN);//得到指令及指令长度
	if(res)return USMART_FUNCERR;//错误的指令 
	str+=i;	 	 			    
	for(i=0;i<sizeof(sys_cmd_tab)/4;i++)//支持的系统指令
	{
		if(usmart_strcmp(sfname,sys_cmd_tab[i])==0)break;
	}
	switch(i)
	{					   
		case 0:
		case 1:
			printf("\r\n");

			break;
		case 2://查询指令
			printf("\r\n");
			printf("-------------------------函数清单--------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)printf("%s\r\n",usmart_dev.funs[i].name);
			printf("\r\n");
			break;	 
		case 3://查询ID
			printf("\r\n");
			printf("-------------------------函数 ID --------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)
			{
				usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&pnum,&rval);//得到本地函数名 
				printf("%s id is:\r\n0X%08X\r\n",sfname,usmart_dev.funs[i].func); //显示ID
			}
			printf("\r\n");
			break;
		case 4://hex指令
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//参数正常
			{
				i=usmart_str2num(sfname,&res);	   	//记录该参数	
				if(i==0)						  	//进制转换功能
				{
					printf("HEX:0X%X\r\n",res);	   	//转为16进制
				}else if(i!=4)return USMART_PARMERR;//参数错误.
				else 				   				//参数显示设定功能
				{
					printf("16进制参数显示!\r\n");
					usmart_dev.sptype=SP_TYPE_HEX;  
				}

			}else return USMART_PARMERR;			//参数错误.
			printf("\r\n"); 
			break;
		case 5://dec指令
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//参数正常
			{
				i=usmart_str2num(sfname,&res);	   	//记录该参数	
				if(i==0)						   	//进制转换功能
				{
					printf("DEC:%lu\r\n",res);	   	//转为10进制
				}else if(i!=4)return USMART_PARMERR;//参数错误.
				else 				   				//参数显示设定功能
				{
					printf("10进制参数显示!\r\n");
					usmart_dev.sptype=SP_TYPE_DEC;  
				}

			}else return USMART_PARMERR;			//参数错误. 
			printf("\r\n"); 
			break;	 
		case 6://runtime指令,设置是否显示函数执行时间
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//参数正常
			{
				i=usmart_str2num(sfname,&res);	   		//记录该参数	
				if(i==0)						   		//读取指定地址数据功能
				{
					if(USMART_ENTIMX_SCAN==0)printf("\r\nError! \r\nTo EN RunTime function,Please set USMART_ENTIMX_SCAN = 1 first!\r\n");//报错
					else
					{
						usmart_dev.runtimeflag=res;
						if(usmart_dev.runtimeflag)printf("Run Time Calculation ON\r\n");
						else printf("Run Time Calculation OFF\r\n"); 
					}
				}else return USMART_PARMERR;   			//未带参数,或者参数错误	 
 			}else return USMART_PARMERR;				//参数错误. 
			printf("\r\n"); 
			break;	    
		default://非法指令
			return USMART_FUNCERR;
	}
	return 0;
}

#if USMART_ENTIMX_SCAN==1
//复位runtime
//需要根据所移植到的MCU的定时器参数进行修改
void usmart_reset_runtime(void)
{
	TIM2->SR&=~(1<<0);	//清除中断标志位 
	TIM2->ARR=0XFFFF;	//将重装载值设置到最大
	TIM2->CNT=0;		//清空定时器的CNT
	usmart_dev.runtime=0;	
}
//获得runtime时间
//返回值:执行时间,单位:0.1ms,最大延时时间为定时器CNT值的2倍*0.1ms
//需要根据所移植到的MCU的定时器参数进行修改
u32 usmart_get_runtime(void)
{
	if(TIM2->SR&0X0001)//在运行期间,产生了定时器溢出
	{
		usmart_dev.runtime+=0XFFFF;
	}
	usmart_dev.runtime+=TIM2->CNT;
	return usmart_dev.runtime;		//返回计数值
}
//下面这两个函数,非USMART函数,放到这里,仅仅方便移植. 
//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{ 		    		  			    
	if(TIM2->SR&0X0001)//溢出中断
	{ 
		usmart_dev.scan();	//执行usmart扫描	
		TIM2->CNT=0;		//清空定时器的CNT
		TIM2->ARR=1000;		//恢复原来的设置
	}				   
	TIM2->SR&=~(1<<0);//清除中断标志位 	    
}
//使能定时器2,使能中断.
void Timer2_Init(u16 arr,u16 psc)
{
    //TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;

	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //TIM2时钟使能 
	RCC->APB1ENR|=1;	//TIM2时钟使能
	
	//TIM2初始化设置
 	TIM2->ARR=arr;  	//设定计数器自动重装值//刚好1ms    
	TIM2->PSC=psc;  	//预分频器7200,得到10Khz的计数时钟		
// 	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
//	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
// 
	//TIM_ITConfig( TIM2, TIM_IT_Update|TIM_IT_Trigger, ENABLE );//TIM2 允许更新，触发中断
	TIM2->DIER|=1<<0;   //允许更新中断	 
	//TIM2中断分组配置
//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //先占优先级03级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
  	MY_NVIC_Init(3,3,TIM2_IRQn,2);//抢占1，子优先级3，组2
	//TIM_Cmd(TIM2, ENABLE);  //使能TIM2 
	TIM2->CR1|=0x01;    //使能定时器3	
}
#endif
////////////////////////////////////////////////////////////////////////////////////////
//初始化串口控制器
//sysclk:系统时钟（Mhz）
void usmart_init(u8 sysclk)
{
#if USMART_ENTIMX_SCAN==1
	Timer2_Init(1000,(u32)sysclk*100-1);//分频,时钟为10K ,100ms中断一次,注意,计数频率必须为10Khz,以和runtime单位(0.1ms)同步.
#endif
	usmart_dev.sptype=1;	//十六进制显示参数
}		
//从str中获取函数名,id,及参数信息
//*str:字符串指针.
//返回值:0,识别成功;其他,错误代码.
u8 usmart_cmd_rec(u8*str) 
{
	u8 sta,i,rval;//状态	 
	u8 rpnum,spnum;
	u8 rfname[MAX_FNAME_LEN];//暂存空间,用于存放接收到的函数名  
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	sta=usmart_get_fname(str,rfname,&rpnum,&rval);//得到接收到的数据的函数名及参数个数	  
	if(sta)return sta;//错误
	for(i=0;i<usmart_dev.fnum;i++)
	{
		sta=usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&spnum,&rval);//得到本地函数名及参数个数
		if(sta)return sta;//本地解析有误	  
		if(usmart_strcmp(sfname,rfname)==0)//相等
		{
			if(spnum>rpnum)return USMART_PARMERR;//参数错误(输入参数比源函数参数少)
			usmart_dev.id=i;//记录函数ID.
			break;//跳出.
		}	
	}
	if(i==usmart_dev.fnum)return USMART_NOFUNCFIND;	//未找到匹配的函数
 	sta=usmart_get_fparam(str,&i);					//得到函数参数个数	
	if(sta)return sta;								//返回错误
	usmart_dev.pnum=i;								//参数个数记录
    return USMART_OK;
}
//usamrt执行函数
void usmart_exe(void)
{
	u8 id,i;
	u32 res;		   
	u32 temp[MAX_PARM];//参数转换,使之支持了字符串 
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	u8 pnum,rval;
	id=usmart_dev.id;
	if(id>=usmart_dev.fnum)return;//不执行.
	usmart_get_fname((u8*)usmart_dev.funs[id].name,sfname,&pnum,&rval);//得到本地函数名,及参数个数 
	printf("\r\n%s(",sfname);//输出正要执行的函数名
	for(i=0;i<pnum;i++)//输出参数
	{
		if(usmart_dev.parmtype&(1<<i))//参数是字符串
		{
			printf("%c",'"');			 
			printf("%s",usmart_dev.parm+usmart_get_parmpos(i));
			printf("%c",'"');
			temp[i]=(u32)&(usmart_dev.parm[usmart_get_parmpos(i)]);
		}else						  //参数是数字
		{
			temp[i]=*(u32*)(usmart_dev.parm+usmart_get_parmpos(i));
			if(usmart_dev.sptype==SP_TYPE_DEC)printf("%lu",temp[i]);//10进制参数显示
			else printf("0X%X",temp[i]);//16进制参数显示 	   
		}
		if(i!=pnum-1)printf(",");
	}
	printf(")");
	usmart_reset_runtime();	//计时器清零,开始计时
	switch(usmart_dev.pnum)
	{
		case 0://无参数(void类型)											  
			res=(*(u32(*)())usmart_dev.funs[id].func)();
			break;
	    case 1://有1个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0]);
			break;
	    case 2://有2个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1]);
			break;
	    case 3://有3个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2]);
			break;
	    case 4://有4个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3]);
			break;
	    case 5://有5个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4]);
			break;
	    case 6://有6个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5]);
			break;
	    case 7://有7个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6]);
			break;
	    case 8://有8个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7]);
			break;
	    case 9://有9个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8]);
			break;
	    case 10://有10个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8],temp[9]);
			break;
	}
	usmart_get_runtime();//获取函数执行时间
	if(rval==1)//需要返回值.
	{
		if(usmart_dev.sptype==SP_TYPE_DEC)printf("=%lu;\r\n",res);//输出执行结果(10进制参数显示)
		else printf("=0X%X;\r\n",res);//输出执行结果(16进制参数显示)	   
	}else printf(";\r\n");		//不需要返回值,直接输出结束
	if(usmart_dev.runtimeflag)	//需要显示函数执行时间
	{ 
		printf("Function Run Time:%d.%1dms\r\n",usmart_dev.runtime/10,usmart_dev.runtime%10);//打印函数执行时间 
	}	
}
//usmart扫描函数
void usmart_scan(void)
{
	u8 sta,len;  
	if(USART_RX_STA&0x8000)//串口接收完成？
	{					   
		len=USART_RX_STA&0x3fff;	//得到此次接收到的数据长度
		USART_RX_BUF[len]='\0';	//在末尾加入结束符. 
		sta=usmart_dev.cmd_rec(USART_RX_BUF);//得到函数各个信息
		if(sta==0)usmart_dev.exe();	//执行函数 
		else 
		{  
			len=usmart_sys_cmd_exe(USART_RX_BUF);
			if(len!=USMART_FUNCERR)sta=len;
			if(sta)
			{
				switch(sta)
				{
					case USMART_FUNCERR:
						printf("函数错误!\r\n");   			
						break;	
					case USMART_PARMERR:
						printf("参数错误!\r\n");   			
						break;				
					case USMART_PARMOVER:
						printf("参数太多!\r\n");   			
						break;		
					case USMART_NOFUNCFIND:
						printf("未找到匹配的函数!\r\n");   			
						break;		
				}
			}
		}
		USART_RX_STA=0;//状态寄存器清空	    
	}
}

#if USMART_USE_WRFUNS==1 	//如果使能了读写操作
//读取指定地址的值		 
u32 read_addr(u32 addr)
{
	return *(u32*)addr;//	
}
//在指定地址写入指定的值		 
void write_addr(u32 addr,u32 val)
{
	*(u32*)addr=val; 	
}
#endif













