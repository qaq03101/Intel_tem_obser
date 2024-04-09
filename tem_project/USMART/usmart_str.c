#include "usmart_str.h"
#include "usmart.h"		   

  
//对比字符串str1和str2
//*str1:字符串1指针
//*str2:字符串2指针
//返回值:0，相等;1，不相等;
u8 usmart_strcmp(u8 *str1,u8 *str2)
{
	while(1)
	{
		if(*str1!=*str2)return 1;
		if(*str1=='\0')break;
		str1++;
		str2++;
	}
	return 0;
}
//把str1的内容copy到str2
//*str1:字符串1指针
//*str2:字符串2指针			   
void usmart_strcopy(u8*str1,u8 *str2)
{
	while(1)
	{										   
		*str2=*str1;	
		if(*str1=='\0')break;
		str1++;
		str2++;
	}
}
//得到字符串的长度(字节)
//*str:字符串指针
//返回值:字符串的长度		   
u8 usmart_strlen(u8*str)
{
	u8 len=0;
	while(1)
	{							 
		if(*str=='\0')break;
		len++;
		str++;
	}
	return len;
}
//m^n函数
//返回值:m^n次方
u32 usmart_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}	    
//把字符串转为数字
//*str:数字字符串指针
//*res:转换完的结果存放地址.
//返回值:0，成功转换完成.其他,错误代码.
//1,数据格式错误.2,16进制位数为0.3,起始格式错误.4,十进制位数为0.
u8 usmart_str2num(u8*str,u32 *res)
{
	u32 t;
	u8 bnum=0;	
	u8 *p;		  
	u8 hexdec=10;
	p=str;
	*res=0;
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//参数合法
		{
			if(*p>='A')hexdec=16;	
			bnum++;					
		}else if(*p=='\0')break;	
		else return 1;				
		p++; 
	} 
	p=str;			    
	if(hexdec==16)		
	{
		if(bnum<3)return 2;			
		if(*p=='0' && (*(p+1)=='X'))//必须以'0X'开头.
		{
			p+=2;	//偏移到数据起始地址.
			bnum-=2;
		}else return 3;
	}else if(bnum==0)return 4;//位数为0，直接退出.	  
	while(1)
	{
		if(bnum)bnum--;
		if(*p<='9'&&*p>='0')t=*p-'0';	//得到数字的值
		else t=*p-'A'+10;				//得到A~F对应的值	    
		*res+=t*usmart_pow(hexdec,bnum);		   
		p++;
		if(*p=='\0')break;	
	}
	return 0;
}
//得到指令名
//*str:源字符串
//*cmdname:指令名
//*nlen:指令名长度		
//maxlen:最大长度(做限制,指令不可能太长的)	
//返回值:0,成功;其他,失败.	  
u8 usmart_get_cmdname(u8*str,u8*cmdname,u8 *nlen,u8 maxlen)
{
	*nlen=0;
 	while(*str!=' '&&*str!='\0') 
	{
		*cmdname=*str;
		str++;
		cmdname++;
		(*nlen)++;
		if(*nlen>=maxlen)return 1;
	}
	*cmdname='\0';
	return 0;
}
//获取下一个字符
//str:字符串指针	
//返回值:下一个字符
u8 usmart_search_nextc(u8* str)
{		   	 	
	str++;
	while(*str==' '&&str!='\0')str++;
	return *str;
} 
//从str中得到函数名
//*str:源字符串指针
//*fname:获取到的函数名字指针
//*pnum:函数的参数个数
//*rval:是否需要显示返回值(0,不需要;1,需要)
//返回值:0,成功;其他,错误代码.
u8 usmart_get_fname(u8*str,u8*fname,u8 *pnum,u8 *rval)
{
	u8 res;
	u8 fover=0;	  
	u8 *strtemp;
	u8 offset=0;  
	u8 parmnum=0;
	u8 temp=1;
	u8 fpname[6];
	u8 fplcnt=0; //第一个参数的长度计数器
	u8 pcnt=0;	 //参数计数器
	u8 nchar;
	//判断函数是否有返回值
	strtemp=str;
	while(*strtemp!='\0')
	{
		if(*strtemp!=' '&&(pcnt&0X7F)<5)//最多记录5个字符
		{	
			if(pcnt==0)pcnt|=0X80;
			if(((pcnt&0x7f)==4)&&(*strtemp!='*'))break;//最后一个字符,必须是*
			fpname[pcnt&0x7f]=*strtemp;
			pcnt++;
		}else if(pcnt==0X85)break;
		strtemp++; 
	} 
	if(pcnt)//接收完了
	{
		fpname[pcnt&0x7f]='\0';
		if(usmart_strcmp(fpname,"void")==0)*rval=0;
		else *rval=1;							   
		pcnt=0;
	} 
	res=0;
	strtemp=str;
	while(*strtemp!='('&&*strtemp!='\0') 
	{  
		strtemp++;
		res++;
		if(*strtemp==' '||*strtemp=='*')
		{
			nchar=usmart_search_nextc(strtemp);		//获取下一个字符
			if(nchar!='('&&nchar!='*')offset=res;	//跳过空格和*号
		}
	}	 
	strtemp=str;
	if(offset)strtemp+=offset+1;//跳到函数名开始的地方	   
	res=0;
	nchar=0;//是否正在字符串里面的标志,0，不在字符串;1，在字符串;
	while(1)
	{
		if(*strtemp==0)
		{
			res=USMART_FUNCERR;//函数错误
			break;
		}else if(*strtemp=='('&&nchar==0)fover++; 
		else if(*strtemp==')'&&nchar==0)
		{
			if(fover)fover--;
			else res=USMART_FUNCERR;
			if(fover==0)break;	    
		}else if(*strtemp=='"')nchar=!nchar;

		if(fover==0)//函数名还没接收完
		{
			if(*strtemp!=' ')
			{
				*fname=*strtemp;//得到函数名
				fname++;
			}
		}else 
		{
			if(*strtemp==',')
			{
				temp=1;		//使能增加一个参数
				pcnt++;	
			}else if(*strtemp!=' '&&*strtemp!='(')
			{
				if(pcnt==0&&fplcnt<5)		
				{
					fpname[fplcnt]=*strtemp;
					fplcnt++;
				}
				temp++;	
			}
			if(fover==1&&temp==2)
			{
				temp++;		
				parmnum++; 	
			}
		}
		strtemp++; 			
	}   
	if(parmnum==1)//只有1个参数.
	{
		fpname[fplcnt]='\0';//加入结束符
		if(usmart_strcmp(fpname,"void")==0)parmnum=0;//参数为void,表示没有参数.
	}
	*pnum=parmnum;	//记录参数个数
	*fname='\0';	//加入结束符
	return res;		//返回执行结果
}


//从str中得到一个函数的参数
//*str:源字符串指针
//*fparm:参数字符串指针
//*ptype:参数类型 0，数字;1，字符串;0XFF，参数错误
//返回值:0,已经无参数了;其他,下一个参数的偏移量.
u8 usmart_get_aparm(u8 *str,u8 *fparm,u8 *ptype)
{
	u8 i=0;
	u8 enout=0;
	u8 type=0;
	u8 string=0; 
	while(1)
	{		    
		if(*str==','&& string==0)enout=1;			
		if((*str==')'||*str=='\0')&&string==0)break;//立即退出标识符
		if(type==0)
		{
			if((*str>='0' && *str<='9')||(*str>='a' && *str<='f')||(*str>='A' && *str<='F')||*str=='X'||*str=='x')//数字串检测
			{
				if(enout)break;					
				if(*str>='a')*fparm=*str-0X20;	
				else *fparm=*str;		   		
				fparm++;
			}else if(*str=='"')//找到字符串的开始标志
			{
				if(enout)break;//找到,后才找到",认为结束了.
				type=1;
				string=1;//登记STRING 正在读了
			}else if(*str!=' '&&*str!=',')//发现非法字符,参数错误
			{
				type=0XFF;
				break;
			}
		}else//string类
		{ 
			if(*str=='"')string=0;
			if(enout)break;			
			if(string)				//字符串正在读
			{	
				if(*str=='\\')		
				{ 
					str++;			
					i++;
				}					
				*fparm=*str;		
				fparm++;
			}	
		}
		i++;//偏移量增加
		str++;
	}
	*fparm='\0';	//加入结束符
	*ptype=type;	//返回参数类型
	return i;		
}
//得到指定参数的起始地址
//num:第num个参数,范围0~9.
//返回值:该参数的起始地址
u8 usmart_get_parmpos(u8 num)
{
	u8 temp=0;
	u8 i;
	for(i=0;i<num;i++)temp+=usmart_dev.plentbl[i];
	return temp;
}
//从str中得到函数参数
//str:源字符串;
//parn:参数的多少.0表示无参数 void类型
//返回值:0,成功;其他,错误代码.
u8 usmart_get_fparam(u8*str,u8 *parn)
{	
	u8 i,type;  
	u32 res;
	u8 n=0;
	u8 len;
	u8 tstr[PARM_LEN+1];
	for(i=0;i<MAX_PARM;i++)usmart_dev.plentbl[i]=0;//清空参数长度表
	while(*str!='(')
	{
		str++;											    
		if(*str=='\0')return USMART_FUNCERR;//遇到结束符了
	}
	str++;
	while(1)
	{
		i=usmart_get_aparm(str,tstr,&type);
		str+=i;								
		switch(type)
		{
			case 0:	//数字
				if(tstr[0]!='\0')				
				{					    
					i=usmart_str2num(tstr,&res);	
					if(i)return USMART_PARMERR;		
					*(u32*)(usmart_dev.parm+usmart_get_parmpos(n))=res;
					usmart_dev.parmtype&=~(1<<n);	
					usmart_dev.plentbl[n]=4;		
					n++;							
					if(n>MAX_PARM)return USMART_PARMOVER;
				}
				break;
			case 1://字符串	 	
				len=usmart_strlen(tstr)+1;	
				usmart_strcopy(tstr,&usmart_dev.parm[usmart_get_parmpos(n)]);
				usmart_dev.parmtype|=1<<n;	
				usmart_dev.plentbl[n]=len;	
				n++;
				if(n>MAX_PARM)return USMART_PARMOVER;
				break;
			case 0XFF:
				return USMART_PARMERR;//参数错误	  
		}
		if(*str==')'||*str=='\0')break;
	}
	*parn=n;	
	return USMART_OK;
}














