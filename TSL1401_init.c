/*-------------曝光延时-------------*/
void Integration_Delay(uint16_t t)
{
	uint16_t x,y;                  //按外部晶振8MHZ算的延时
	for(x = 0;x < t;x++){
		for(y = 0; y < 8000; y++);
	}
//    LPLD_LPTMR_DelayMs(t);
}

void Delay_ms(uint16_t time){
	uint16_t x,y;                  //按外部晶振8MHZ算的延时
	for(x = 0;x < time;x++){
		for(y = 0; y < 8000; y++);
	}
}

float ADC1_SE_Get(void){        //ADC1采集数据
	float val;
	ADC1->SC1[0] &= ~ADC_SC1_ADCH_MASK; //清除ADCH
	ADC1->SC1[0] |= ADC_SC1_ADCH(10);    //开始转换，涉及通道ADC1_SE10
	while(ADC1_Complete()==0){}           //等待转换完成
	val = (float)(ADC1->R[0]);              //读取数据
	return val;
}
float ADC0_SE_Get(void){        //ADC1采集数据
	float val;
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK; //清除ADCH
	ADC0->SC1[0] |= ADC_SC1_ADCH(8);    //开始转换，涉及通道ADC0_SE8
	while(ADC0_Complete()==0){}           //等待转换完成
	val = (float)(ADC0->R[0]);              //读取数据
	return val;
}
/*-------------单次采集-------------*/
void TSL1401_GetLine_Oneshot_1(uint8_t *pixel)
{
  uint8_t i;
  //-----------单次采集曝光处理------------
  TSL1401_SI_High;
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");//delay();
  TSL1401_SI_Low;
  //发送第1个CLK
  asm("nop");
  TSL1401_CLK_Low;
  
  //发送第2~129个CLK
  for(i=1; i<129; i++)
  {
	asm("nop");
    TSL1401_CLK_High;
    asm("nop");
    TSL1401_CLK_Low;
  }
  //曝光延时
  Integration_Delay(20);//固定曝光时间 10ms
  //采集上次曝光得到的图像
  //开始SI
  TSL1401_SI_High;
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");//delay();
  TSL1401_SI_Low;
  asm("nop");//delay();
  //采集第1个点
  pixel[0] = Normalized_U8(ADC1_SE_Get());
  TSL1401_CLK_Low;
  
  //采集第2~128个点
  for(i=1; i<128; i++)
  {
	asm("nop");
    TSL1401_CLK_High;
    asm("nop");//delay();
    pixel[i] = Normalized_U8(ADC1_SE_Get());
    TSL1401_CLK_Low;
  }
  
  //发送第129个clk
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");
  TSL1401_CLK_Low;
  asm("nop");
  
}

void TSL1401_GetLine_Oneshot_0(uint8_t *pixel)
{
  uint8_t i;
  //-----------单次采集曝光处理------------
  CCD4_SI_High;
  asm("nop");
  CCD4_CLK_High;
  asm("nop");//delay();
  CCD4_SI_Low;
  //发送第1个CLK
  asm("nop");
  CCD4_CLK_Low;

  //发送第2~129个CLK
  for(i=1; i<129; i++)
  {
	asm("nop");
	CCD4_CLK_High;
    asm("nop");
    CCD4_CLK_Low;
  }
  //曝光延时
  Integration_Delay(10);//固定曝光时间 10ms
  //采集上次曝光得到的图像
  //开始SI
  CCD4_SI_High;
  asm("nop");
  CCD4_CLK_High;
  asm("nop");//delay();
  CCD4_SI_Low;
  asm("nop");//delay();
  //采集第1个点
  pixel[0] = Normalized_U8(ADC0_SE_Get());
  CCD4_CLK_Low;

  //采集第2~128个点
  for(i=1; i<128; i++)
  {
	asm("nop");
	CCD4_CLK_High;
    asm("nop");//delay();
    pixel[i] = Normalized_U8(ADC0_SE_Get());
    CCD4_CLK_Low;
  }

  //发送第129个clk
  asm("nop");
  CCD4_CLK_High;
  asm("nop");
  CCD4_CLK_Low;
  asm("nop");

}