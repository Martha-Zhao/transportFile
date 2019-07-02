/*
 * --------------TSL1401示例工程-----------------
 *
 * 测试硬件平台:  S32K
 *
 * 说明:    本工程移植于"拉普兰德K60底层库"开发的基于S32K的CCD测试程序，
 *
 *
 * 文件名:		TSL1401.c
 * 用途:		拉普兰德TSL1401线性CCD模块使用例程(单次曝光采集)
 *                      
 * 引脚连接:
 *                      PTE8       --SI
 *                      PTE9       --CLK
 *                      PTA3       --AO
 * 参数设置:
 *                      PRINT_AD    --设置串口打印数据的类型，二值量或模拟量   
 *                      THRESHOLD   --设置模拟量转二值量的阈值   
 *                      WINDOW_WIDTH--设置串口打印采集数据的宽度   
 */

#include "include_define.h"
#define STARTPOINT 0     //检测起始点
#define ENDPOINT 127      //检测终结点
#define MID_POINT 88
#define LINE_WIDTH_MIN 20 //白线最小线宽
#define LINE_WIDTH_MAX 40 //白线最大线宽
//ADC1
#define TSL1401_SI_High		PTA->PSOR |= 1<<0   //SI 设置SI引脚高电平
#define TSL1401_SI_Low		PTA->PCOR |= 1<<0   //SI 设置低电平
#define TSL1401_CLK_High	PTB->PSOR |= 1<<10  //CLK设置高电平
#define TSL1401_CLK_Low		PTB->PCOR |= 1<<10  //CLK设置低电平
//ADC0
#define CCD4_SI_High   PTD->PSOR |= 1<<12
#define CCD4_SI_Low    PTD->PCOR |= 1<<12
#define CCD4_CLK_High  PTD->PSOR |= 1<<10
#define CCD4_CLK_Low   PTD->PCOR |= 1<<10

//全局变量声明
extern char Mid_line_1;
extern char Mid_line_last_1;                   //记录上一刻的中线，用于解决全白情况（全白情况由于车身遮挡并不是全白，线宽增大，Mid_line右移）
extern char Mid_line_0;
extern char Mid_line_last_0;                   //
extern char pix_threshold_1;                   //CCD动态阈值，在阈值函数里进行更新，可通过直接赋值设置成固定阈值
extern char pix_threshold_0;
extern uint8_t gPixel_1[128];
extern uint8_t gPixel_0[128];
extern uint8_t Binary_Pixel_1[128];
extern uint8_t Binary_Pixel_0[128];
extern int Line_Width;
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

/*------------数据归一化--------------
 * 将ADC采集到的数值归一化到0-255范围内*/
uint8_t Normalized_U8(float data)
{
  return (uint8_t)(data*255/4096);
}
/*------------获取动态阈值------------
 * 备注：计算像素值的最大值和最小值来确定阈值*/
void Get_dynamic_threshold_1(void){
	char value_max;
	char value_min;
	int i;
	value_max = gPixel_1[0];
	value_min = gPixel_1[0];
	for(i = 0;i < 127; i++){
		if(value_max < gPixel_1[i])value_max = gPixel_1[i];
		if(value_min > gPixel_1[i])value_min = gPixel_1[i];
	}
	pix_threshold_1 = (value_max + value_min)/2;
}

void Get_dynamic_threshold_0(void){
	char value_max;
	char value_min;
	int i;
	value_max = gPixel_0[0];
	value_min = gPixel_0[0];
	for(i = 0;i < 127; i++){
		if(value_max < gPixel_0[i])value_max = gPixel_0[i];
		if(value_min > gPixel_0[i])value_min = gPixel_0[i];
	}
	pix_threshold_0 = (value_max + value_min)/2;
}

void Binary_Process_1(void){
	int i;
	/*-*------将两端舍弃的点置零------*-*/
//	for(i = 0;i < STARTPOINT;i++){Binary_Pixel_1[i] = 0;}
//	for(i = ENDPOINT;i < 127;i++){Binary_Pixel_1[i] = 0;}
	/*-*二值化对原始像素值不进行更改，只是将二值化后的结果保留到新的数组中*-*/
	for(i = STARTPOINT;i < ENDPOINT;i++){
		if(gPixel_1[i] > pix_threshold_1)Binary_Pixel_1[i] = 254;
		else Binary_Pixel_1[i] = 0;
	}
}
void Binary_Process_0(void){
	int i;
	/*-*------将两端舍弃的点置零------*-*/
//	for(i = 0;i < STARTPOINT;i++){Binary_Pixel_0[i] = 0;}
//	for(i = ENDPOINT;i < 127;i++){Binary_Pixel_0[i] = 0;}
	/*-*二值化对原始像素值不进行更改，只是将二值化后的结果保留到新的数组中*-*/
	for(i = STARTPOINT;i < ENDPOINT; i++){
		if(gPixel_0[i] > pix_threshold_0)Binary_Pixel_0[i] = 254;
		else Binary_Pixel_0[i] = 0;
	}
}
/*---------------滤波函数-----------------
 * 功能：1、过滤掉跳变点
 *     2、可以的话过滤掉不满足线宽要求的点
 *     3、均值法滤波，针对缓变信号，求前n-1次采集值的均值后，与最近一次采集值比较，如果差值超过一定范围，则丢弃，否则保留
 *     4、滤波在二值化后进行*/

void Filter_Mean_1(void){
	int i;
	/*-------------过滤单个跳变点-------------*/
	for(i = STARTPOINT+1;i < ENDPOINT-1;i++){
		if(Binary_Pixel_1[i] == 255 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+1])Binary_Pixel_1[i] = 0;
		else if(Binary_Pixel_1[i] == 0 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+1])Binary_Pixel_1[i] = 255;
	}
	/*-----------过滤连续的两个点的影响----------*/
	for(i = STARTPOINT+1;i < ENDPOINT-2;i++){
		if(Binary_Pixel_1[i] == 255 && Binary_Pixel_1[i+1] == 255 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+2])Binary_Pixel_1[i] = 0,Binary_Pixel_1[i+1] = 0;
		else if(Binary_Pixel_1[i] == 0 && Binary_Pixel_1[i+1] == 0 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+2])Binary_Pixel_1[i] = 255,Binary_Pixel_1[i+1] = 255;
	}
}

void Filter_Mean_0(void){
	int i;
	/*-------------过滤单个跳变点-------------*/
	for(i = STARTPOINT+1;i < ENDPOINT-1;i++){
		if(Binary_Pixel_0[i] == 255 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+1])Binary_Pixel_0[i] = 0;
		else if(Binary_Pixel_0[i] == 0 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+1])Binary_Pixel_0[i] = 255;
	}
	/*-----------过滤连续的两个点的影响----------*/
	for(i = STARTPOINT+1;i < ENDPOINT-2;i++){
		if(Binary_Pixel_0[i] == 255 && Binary_Pixel_0[i+1] == 255 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+2])Binary_Pixel_0[i] = 0,Binary_Pixel_0[i+1] = 0;
		else if(Binary_Pixel_0[i] == 0 && Binary_Pixel_0[i+1] == 0 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+2])Binary_Pixel_0[i] = 255,Binary_Pixel_0[i+1] = 255;
	}
}

/*--------------白线提取函数---------------
 * 通过检测跳变点检测白线,对二值化后的图像进行检测
 * 该函数自带有过滤不满足线宽要求的功能，但如果干扰太多，尤其是有多种跳变点时
 * 先滤波在提线比较好
 *
 * 检测到全黑会死*/
void Get_WhiteLine_1(void){
	int i;
//	int Line_Width;
	int Left_point[3]={0},Right_point[3]={127};
	/*-----从初始端开始遍历找到左跳变点-----*/
	for(i = STARTPOINT;i < ENDPOINT;i++){//上升沿寻找
		if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
			Left_point[0] = i;break;//上升沿边界点值为0
		}
	}
	/*-----从结束端开始遍历找到右跳变点-----*/
	for(i = ENDPOINT;i > STARTPOINT;i--){//下降沿寻找
		if(((Binary_Pixel_1[i+1] | Binary_Pixel_1[i+2] | Binary_Pixel_1[i+3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i-1] && Binary_Pixel_1[i-2]) == 1)){
			Right_point[0] = i;break;//下降沿边界点值为0
		}
	}
	Line_Width = Right_point[0] - Left_point[0];
	if(Line_Width < 0)Mid_line_1 = 0;
	if(Line_Width == 0)Mid_line_1 =Left_point[0];//只有一个跳变点，白线在边缘，以跳变点作为中线
	//线宽满足要求说明跳变点正确，计算中线
	else if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[0] + Right_point[0])/2;//确定中线位置
	else{
		/*如果程序执行到此，说明边界点不符合要求，有干扰的白线，或者是遇到全白线宽剧增，在这两个边界点范围内继续寻找合适宽度的白线*/
		for(i = Left_point[0] + 3;i < Right_point[0] - 2;i++){//检测下降沿
			//中间检测到下降沿
			if(((Binary_Pixel_1[i+1] | Binary_Pixel_1[i+2] | Binary_Pixel_1[i+3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i-1] && Binary_Pixel_1[i-2]) == 1)){
				Right_point[1] = i;break;
			}
			//中间没有检测到下降沿，即全白的实际情况
			else Right_point[1] = Right_point[0];//对应全白情况，中间下降沿与右边界点对应
			//else Mid_line_1 = Mid_line_last_1;
		}
		Line_Width = Right_point[1] - Left_point[0];
		if(Line_Width < 0)Mid_line_1 = (Left_point[0] + Right_point[0])/2;         //之前的线宽不满足要求，但是不存在两条白线，以上一次确定的中线为准
		if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[0] + Right_point[1])/2;//确定中线位置
		else{
			/*运行至此说明至少出现了两条线，且第一条线不满足要求或者全白*/
			if(Right_point[0] - Right_point[1] < 4) Mid_line_1 = Mid_line_last_1;  //全白情况
			else{
				for(i = Right_point[1] + 4;i < Right_point[0];i++){
				    if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
				        Left_point[1] = i;break;//上升沿边界点值为0
				    }
			    }
				Line_Width = Right_point[0] - Left_point[1];
				if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[1] + Right_point[0])/2;//确定中线位置
				else Mid_line_1 = Right_point[0];
			}
//			for(i = Right_point[1] + 4;i < Right_point[0];i++){
//				if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
//					Left_point[1] = i;break;//上升沿边界点值为0
//				}
//			}
//			Line_Width = Right_point[0] - Left_point[1];
//			if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[1] + Right_point[0])/2;//确定中线位置
//			else Mid_line_1 = Right_point[0];
			//----------程序先写至此，以上是最多出现两条白线的情况分析或者全白情况分析
		}
	}
	Mid_line_last_1 = Mid_line_1;
}

void Get_WhiteLine_0(void){
	int i;
//	int Line_Width;
	int Left_point[3]={10},Right_point[3]={118};
	/*-----从初始端开始遍历找到左跳变点-----*/
	for(i = STARTPOINT;i < ENDPOINT;i++){//上升沿寻找
		if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
			Left_point[0] = i;break;//上升沿边界点值为0
		}
	}
	/*-----从结束端开始遍历找到右跳变点-----*/
	for(i = ENDPOINT;i > STARTPOINT;i--){//下降沿寻找
		if(((Binary_Pixel_0[i+1] | Binary_Pixel_0[i+2] | Binary_Pixel_0[i+3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i-1] && Binary_Pixel_0[i-2]) == 1)){
			Right_point[0] = i;break;//下降沿边界点值为0
		}
	}
	Line_Width = Right_point[0] - Left_point[0];
	if(Line_Width < 0)Mid_line_0 = 0;
	if(Line_Width == 0)Mid_line_0 =Left_point[0];//只有一个跳变点，白线在边缘，以跳变点作为中线
	//线宽满足要求说明跳变点正确，计算中线
	else if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[0] + Right_point[0])/2;//确定中线位置
	else{
		/*如果程序执行到此，说明边界点不符合要求，有干扰的白线，在这两个边界点范围内继续寻找合适宽度的白线*/
		for(i = Left_point[0] + 3;i < Right_point[0] - 2;i++){//检测下降沿
			if(((Binary_Pixel_0[i+1] | Binary_Pixel_0[i+2] | Binary_Pixel_0[i+3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i-1] && Binary_Pixel_0[i-2]) == 1)){
				Right_point[1] = i;break;
			}
			else Right_point[1] = Right_point[0];
		}
		Line_Width = Right_point[1] - Left_point[0];
		if(Line_Width < 0)Mid_line_0 = (Left_point[0] + Right_point[0])/2;//之前的线宽不满足要求，但是不存在两条白线，以上一次确定的中线为准
		if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[0] + Right_point[1])/2;//确定中线位置
		else{
			/*运行至此说明至少出现了两条线，且第一条线不满足要求*/
			if(Right_point[1] - Right_point[0] < 4)Mid_line_0 = Mid_line_last_0;
			else{
				for(i = Right_point[1] + 4;i < Right_point[0];i++){
				    if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
						Left_point[1] = i;break;//上升沿边界点值为0
					}
				}
				Line_Width = Right_point[0] - Left_point[1];
				if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[1] + Right_point[0])/2;//确定中线位置
				else Mid_line_0 = Right_point[0];
			    //----------程序先写至此，以上是最多出现两条白线的情况分析
			}
//			for(i = Right_point[1] + 4;i < Right_point[0];i++){
//				if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
//					Left_point[1] = i;break;//上升沿边界点值为0
//				}
//			}
//			Line_Width = Right_point[0] - Left_point[1];
//			if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[1] + Right_point[0])/2;//确定中线位置
//			else Mid_line_0 = Right_point[0];
			//----------程序先写至此，以上是最多出现两条白线的情况分析
		}
	}
}
