#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

							  
#define ADC_CH1 		1  			//ͨ��1(������PA1)	 	    
#define ADC_CH_TEMP  	16 			//�¶ȴ�����ͨ��
	   									   
void Adc_Init(void); 				//ADCͨ����ʼ��
u16  Get_Adc(u8 ch); 				//���ĳ��ͨ��ֵ 
u16 Get_Adc_Average(u8 ch,u8 times);//�õ�ĳ��ͨ��10�β�����ƽ��ֵ
short Get_Temprate(void);			//��ȡ�ڲ��¶ȴ������¶�ֵ
#endif 














