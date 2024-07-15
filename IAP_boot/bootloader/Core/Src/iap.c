/*
 * iap.c
 *
 *  Created on: Apr 21, 2024
 *      Author: ECA1WX
 */
#include "iap.h"
#include "usart.h"
#include "stmflash.h"

iapfun jump2app;

u32 iapbuf[512]; 	//2K字节缓存  

//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(uint32_t addr){
    asm("MSR MSP, r0");			//set Main Stack value
    asm("BX r14");
}

void iap_load_app(u32 appxaddr){
	if(((*(u32*)appxaddr)&0x2FF00000)==0x20000000)	//检查主堆栈指针指向位置是否合法
	{ 
		// SysTick->CTRL = 0X00;//禁止SysTick
		// SysTick->LOAD = 0;
		// SysTick->VAL = 0;

		// HAL_SuspendTick();

		//__disable_irq();

		portDISABLE_INTERRUPTS();	//如果开启rtos

		jump2app=(iapfun)*(__IO u32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		

		MSR_MSP(*(u32*)appxaddr);					//设置APP的主堆栈指针

		//__set_MSP(*(__IO uint32_t*) appxaddr);
		jump2app();									//��ת��APP.

	}
}

void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{

	u32 t;

	u16 i=0;

	u32 temp;

	u32 fwaddr=appxaddr;//当前写入地址

	u8 *dfu=appbuf;

	for(t=0;t<appsize;t+=4)

	{						   

		temp=(u32)dfu[3]<<24;   

		temp|=(u32)dfu[2]<<16;    

		temp|=(u32)dfu[1]<<8;

		temp|=(u32)dfu[0];	  

		dfu+=4;//偏移四个字节

		iapbuf[i++]=temp;	    

		if(i==512)

		{

			i=0; 

			STMFLASH_Write(fwaddr,iapbuf,512);

			fwaddr+=2048;//偏移2048  512*4=2048 一页为2048字节，2k

		}

	} 

	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将剩余字节写入 

}



