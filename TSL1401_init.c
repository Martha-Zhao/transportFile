/*
* TSL1401 init file
* rewrite in 0702
*/

/*-------------曝光延时-------------*/
void Integration_Delay(uint16_t t)
{
	uint16_t x,y;                  //按外部晶振8MHZ算的延时
	for(x = 0;x < t;x++){
		for(y = 0; y < 8000; y++);
	}
}

/*-------------simple delay-------------*/
void Delay_ms(uint16_t time){
	uint16_t x,y;                  //按外部晶振8MHZ算的延时
	for(x = 0;x < time;x++){
		for(y = 0; y < 8000; y++);
	}
}

/*-------------ADC0 and ADC1-------------*/
float ADC1_SE_Get(void){        //ADC1采集数据
	float val;
	ADC1->SC1[0] &= ~ADC_SC1_ADCH_MASK; //清除ADCH
	ADC1->SC1[0] |= ADC_SC1_ADCH(10);    //开始转换，涉及通道ADC1_SE10
	while(ADC1_Complete()==0){}           //等待转换完成
	val = (float)(ADC1->R[0]);              //读取数据
	return val;
}


float ADC0_SE_Get(void){        //ADC0采集数据
	float val;
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK; //清除ADCH
	ADC0->SC1[0] |= ADC_SC1_ADCH(8);    //开始转换，涉及通道ADC0_SE8
	while(ADC0_Complete()==0){}           //等待转换完成
	val = (float)(ADC0->R[0]);              //读取数据
	return val;
}

/*-------------单次采集-------------*/
void TSL1401_GetLine_Oneshot(uint8_t *pixel， uint8_t ccd_select)
{
	if (ccd_select == 1){  //CCD1
		TSL1401_SI_High		PTA->PSOR |= 1<<0   //SI 设置SI引脚高电平
		TSL1401_SI_Low		PTA->PCOR |= 1<<0   //SI 设置低电平
		TSL1401_CLK_High	PTB->PSOR |= 1<<10  //CLK设置高电平
		TSL1401_CLK_Low	PTB->PCOR |= 1<<10 //CLK设置低电平
	}
	else if (ccd_select == 0){ //CCD0
		TSL1401_SI_High   PTD->PSOR |= 1<<12
		TSL1401_SI_Low    PTD->PCOR |= 1<<12
		TSL1401_CLK_High  PTD->PSOR |= 1<<10
		TSL1401_CLK_Low PTD->PCOR |= 1<<10
	}

	uint8_t i
//-----------单次采集曝光处理------------
	TSL14_SI_High;
	asm("nop");
	TSL1401_CLK_High;
	asm("nop");
	TSL1401_SI_Low;
	
	asm("nop");//发送第1个CLK
	TSL1401_CLK_Low;
	  
	for(i=1; i<129; i++){//发送第2~129个CLK
		asm("nop");
		TSL1401_CLK_High;
		asm("nop");
		TSL1401_CLK_Low;
	}
	Integration_Delay(20);//固定曝光时间 10ms
	  
	TSL1401_SI_High;//开始SI
	asm("nop");
	TSL1401_CLK_High;
	asm("nop");
	TSL1401_SI_Low;
	asm("nop");
	pixel[0] = Normalized_U8(ADC1_SE_Get());//采集第1个点
	TSL1401_CLK_Low;
	
	for(i=1; i<128; i++){//采集第2~128个点
		asm("nop");
		TSL1401_CLK_High;
		asm("nop");//delay();
		pixel[i] = Normalized_U8(ADC1_SE_Get());
		TSL1401_CLK_Low;
	}
	
	asm("nop");//发送第129个clk
	TSL1401_CLK_High;
	asm("nop");
	TSL1401_CLK_Low;
	asm("nop");
}
