/*
 * --------------TSL1401ʾ������-----------------
 *
 * ����Ӳ��ƽ̨:  S32K
 *
 * ˵��:    ��������ֲ��"��������K60�ײ��"�����Ļ���S32K��CCD���Գ���
 *
 *
 * �ļ���:		TSL1401.c
 * ��;:		��������TSL1401����CCDģ��ʹ������(�����ع�ɼ�)
 *                      
 * ��������:
 *                      PTE8       --SI
 *                      PTE9       --CLK
 *                      PTA3       --AO
 * ��������:
 *                      PRINT_AD    --���ô��ڴ�ӡ���ݵ����ͣ���ֵ����ģ����   
 *                      THRESHOLD   --����ģ����ת��ֵ������ֵ   
 *                      WINDOW_WIDTH--���ô��ڴ�ӡ�ɼ����ݵĿ��   
 */

#include "include_define.h"
#define STARTPOINT 0     //�����ʼ��
#define ENDPOINT 127      //����ս��
#define MID_POINT 88
#define LINE_WIDTH_MIN 20 //������С�߿�
#define LINE_WIDTH_MAX 40 //��������߿�
//ADC1
#define TSL1401_SI_High		PTA->PSOR |= 1<<0   //SI ����SI���Ÿߵ�ƽ
#define TSL1401_SI_Low		PTA->PCOR |= 1<<0   //SI ���õ͵�ƽ
#define TSL1401_CLK_High	PTB->PSOR |= 1<<10  //CLK���øߵ�ƽ
#define TSL1401_CLK_Low		PTB->PCOR |= 1<<10  //CLK���õ͵�ƽ
//ADC0
#define CCD4_SI_High   PTD->PSOR |= 1<<12
#define CCD4_SI_Low    PTD->PCOR |= 1<<12
#define CCD4_CLK_High  PTD->PSOR |= 1<<10
#define CCD4_CLK_Low   PTD->PCOR |= 1<<10

//ȫ�ֱ�������
extern char Mid_line_1;
extern char Mid_line_last_1;                   //��¼��һ�̵����ߣ����ڽ��ȫ�������ȫ��������ڳ����ڵ�������ȫ�ף��߿�����Mid_line���ƣ�
extern char Mid_line_0;
extern char Mid_line_last_0;                   //
extern char pix_threshold_1;                   //CCD��̬��ֵ������ֵ��������и��£���ͨ��ֱ�Ӹ�ֵ���óɹ̶���ֵ
extern char pix_threshold_0;
extern uint8_t gPixel_1[128];
extern uint8_t gPixel_0[128];
extern uint8_t Binary_Pixel_1[128];
extern uint8_t Binary_Pixel_0[128];
extern int Line_Width;
/*-------------�ع���ʱ-------------*/
void Integration_Delay(uint16_t t)
{
	uint16_t x,y;                  //���ⲿ����8MHZ�����ʱ
	for(x = 0;x < t;x++){
		for(y = 0; y < 8000; y++);
	}
//    LPLD_LPTMR_DelayMs(t);
}

void Delay_ms(uint16_t time){
	uint16_t x,y;                  //���ⲿ����8MHZ�����ʱ
	for(x = 0;x < time;x++){
		for(y = 0; y < 8000; y++);
	}
}

float ADC1_SE_Get(void){        //ADC1�ɼ�����
	float val;
	ADC1->SC1[0] &= ~ADC_SC1_ADCH_MASK; //���ADCH
	ADC1->SC1[0] |= ADC_SC1_ADCH(10);    //��ʼת�����漰ͨ��ADC1_SE10
	while(ADC1_Complete()==0){}           //�ȴ�ת�����
	val = (float)(ADC1->R[0]);              //��ȡ����
	return val;
}
float ADC0_SE_Get(void){        //ADC1�ɼ�����
	float val;
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK; //���ADCH
	ADC0->SC1[0] |= ADC_SC1_ADCH(8);    //��ʼת�����漰ͨ��ADC0_SE8
	while(ADC0_Complete()==0){}           //�ȴ�ת�����
	val = (float)(ADC0->R[0]);              //��ȡ����
	return val;
}
/*-------------���βɼ�-------------*/
void TSL1401_GetLine_Oneshot_1(uint8_t *pixel)
{
  uint8_t i;
  //-----------���βɼ��ع⴦��------------
  TSL1401_SI_High;
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");//delay();
  TSL1401_SI_Low;
  //���͵�1��CLK
  asm("nop");
  TSL1401_CLK_Low;
  
  //���͵�2~129��CLK
  for(i=1; i<129; i++)
  {
	asm("nop");
    TSL1401_CLK_High;
    asm("nop");
    TSL1401_CLK_Low;
  }
  //�ع���ʱ
  Integration_Delay(20);//�̶��ع�ʱ�� 10ms
  //�ɼ��ϴ��ع�õ���ͼ��
  //��ʼSI
  TSL1401_SI_High;
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");//delay();
  TSL1401_SI_Low;
  asm("nop");//delay();
  //�ɼ���1����
  pixel[0] = Normalized_U8(ADC1_SE_Get());
  TSL1401_CLK_Low;
  
  //�ɼ���2~128����
  for(i=1; i<128; i++)
  {
	asm("nop");
    TSL1401_CLK_High;
    asm("nop");//delay();
    pixel[i] = Normalized_U8(ADC1_SE_Get());
    TSL1401_CLK_Low;
  }
  
  //���͵�129��clk
  asm("nop");
  TSL1401_CLK_High;
  asm("nop");
  TSL1401_CLK_Low;
  asm("nop");
  
}

void TSL1401_GetLine_Oneshot_0(uint8_t *pixel)
{
  uint8_t i;
  //-----------���βɼ��ع⴦��------------
  CCD4_SI_High;
  asm("nop");
  CCD4_CLK_High;
  asm("nop");//delay();
  CCD4_SI_Low;
  //���͵�1��CLK
  asm("nop");
  CCD4_CLK_Low;

  //���͵�2~129��CLK
  for(i=1; i<129; i++)
  {
	asm("nop");
	CCD4_CLK_High;
    asm("nop");
    CCD4_CLK_Low;
  }
  //�ع���ʱ
  Integration_Delay(10);//�̶��ع�ʱ�� 10ms
  //�ɼ��ϴ��ع�õ���ͼ��
  //��ʼSI
  CCD4_SI_High;
  asm("nop");
  CCD4_CLK_High;
  asm("nop");//delay();
  CCD4_SI_Low;
  asm("nop");//delay();
  //�ɼ���1����
  pixel[0] = Normalized_U8(ADC0_SE_Get());
  CCD4_CLK_Low;

  //�ɼ���2~128����
  for(i=1; i<128; i++)
  {
	asm("nop");
	CCD4_CLK_High;
    asm("nop");//delay();
    pixel[i] = Normalized_U8(ADC0_SE_Get());
    CCD4_CLK_Low;
  }

  //���͵�129��clk
  asm("nop");
  CCD4_CLK_High;
  asm("nop");
  CCD4_CLK_Low;
  asm("nop");

}

/*------------���ݹ�һ��--------------
 * ��ADC�ɼ�������ֵ��һ����0-255��Χ��*/
uint8_t Normalized_U8(float data)
{
  return (uint8_t)(data*255/4096);
}
/*------------��ȡ��̬��ֵ------------
 * ��ע����������ֵ�����ֵ����Сֵ��ȷ����ֵ*/
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
	/*-*------�����������ĵ�����------*-*/
//	for(i = 0;i < STARTPOINT;i++){Binary_Pixel_1[i] = 0;}
//	for(i = ENDPOINT;i < 127;i++){Binary_Pixel_1[i] = 0;}
	/*-*��ֵ����ԭʼ����ֵ�����и��ģ�ֻ�ǽ���ֵ����Ľ���������µ�������*-*/
	for(i = STARTPOINT;i < ENDPOINT;i++){
		if(gPixel_1[i] > pix_threshold_1)Binary_Pixel_1[i] = 254;
		else Binary_Pixel_1[i] = 0;
	}
}
void Binary_Process_0(void){
	int i;
	/*-*------�����������ĵ�����------*-*/
//	for(i = 0;i < STARTPOINT;i++){Binary_Pixel_0[i] = 0;}
//	for(i = ENDPOINT;i < 127;i++){Binary_Pixel_0[i] = 0;}
	/*-*��ֵ����ԭʼ����ֵ�����и��ģ�ֻ�ǽ���ֵ����Ľ���������µ�������*-*/
	for(i = STARTPOINT;i < ENDPOINT; i++){
		if(gPixel_0[i] > pix_threshold_0)Binary_Pixel_0[i] = 254;
		else Binary_Pixel_0[i] = 0;
	}
}
/*---------------�˲�����-----------------
 * ���ܣ�1�����˵������
 *     2�����ԵĻ����˵��������߿�Ҫ��ĵ�
 *     3����ֵ���˲�����Ի����źţ���ǰn-1�βɼ�ֵ�ľ�ֵ�������һ�βɼ�ֵ�Ƚϣ������ֵ����һ����Χ��������������
 *     4���˲��ڶ�ֵ�������*/

void Filter_Mean_1(void){
	int i;
	/*-------------���˵��������-------------*/
	for(i = STARTPOINT+1;i < ENDPOINT-1;i++){
		if(Binary_Pixel_1[i] == 255 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+1])Binary_Pixel_1[i] = 0;
		else if(Binary_Pixel_1[i] == 0 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+1])Binary_Pixel_1[i] = 255;
	}
	/*-----------�����������������Ӱ��----------*/
	for(i = STARTPOINT+1;i < ENDPOINT-2;i++){
		if(Binary_Pixel_1[i] == 255 && Binary_Pixel_1[i+1] == 255 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+2])Binary_Pixel_1[i] = 0,Binary_Pixel_1[i+1] = 0;
		else if(Binary_Pixel_1[i] == 0 && Binary_Pixel_1[i+1] == 0 && Binary_Pixel_1[i]!=Binary_Pixel_1[i-1] && Binary_Pixel_1[i]!=Binary_Pixel_1[i+2])Binary_Pixel_1[i] = 255,Binary_Pixel_1[i+1] = 255;
	}
}

void Filter_Mean_0(void){
	int i;
	/*-------------���˵��������-------------*/
	for(i = STARTPOINT+1;i < ENDPOINT-1;i++){
		if(Binary_Pixel_0[i] == 255 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+1])Binary_Pixel_0[i] = 0;
		else if(Binary_Pixel_0[i] == 0 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+1])Binary_Pixel_0[i] = 255;
	}
	/*-----------�����������������Ӱ��----------*/
	for(i = STARTPOINT+1;i < ENDPOINT-2;i++){
		if(Binary_Pixel_0[i] == 255 && Binary_Pixel_0[i+1] == 255 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+2])Binary_Pixel_0[i] = 0,Binary_Pixel_0[i+1] = 0;
		else if(Binary_Pixel_0[i] == 0 && Binary_Pixel_0[i+1] == 0 && Binary_Pixel_0[i]!=Binary_Pixel_0[i-1] && Binary_Pixel_0[i]!=Binary_Pixel_0[i+2])Binary_Pixel_0[i] = 255,Binary_Pixel_0[i+1] = 255;
	}
}

/*--------------������ȡ����---------------
 * ͨ���������������,�Զ�ֵ�����ͼ����м��
 * �ú����Դ��й��˲������߿�Ҫ��Ĺ��ܣ����������̫�࣬�������ж��������ʱ
 * ���˲������߱ȽϺ�
 *
 * ��⵽ȫ�ڻ���*/
void Get_WhiteLine_1(void){
	int i;
//	int Line_Width;
	int Left_point[3]={0},Right_point[3]={127};
	/*-----�ӳ�ʼ�˿�ʼ�����ҵ��������-----*/
	for(i = STARTPOINT;i < ENDPOINT;i++){//������Ѱ��
		if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
			Left_point[0] = i;break;//�����ر߽��ֵΪ0
		}
	}
	/*-----�ӽ����˿�ʼ�����ҵ��������-----*/
	for(i = ENDPOINT;i > STARTPOINT;i--){//�½���Ѱ��
		if(((Binary_Pixel_1[i+1] | Binary_Pixel_1[i+2] | Binary_Pixel_1[i+3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i-1] && Binary_Pixel_1[i-2]) == 1)){
			Right_point[0] = i;break;//�½��ر߽��ֵΪ0
		}
	}
	Line_Width = Right_point[0] - Left_point[0];
	if(Line_Width < 0)Mid_line_1 = 0;
	if(Line_Width == 0)Mid_line_1 =Left_point[0];//ֻ��һ������㣬�����ڱ�Ե�����������Ϊ����
	//�߿�����Ҫ��˵���������ȷ����������
	else if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[0] + Right_point[0])/2;//ȷ������λ��
	else{
		/*�������ִ�е��ˣ�˵���߽�㲻����Ҫ���и��ŵİ��ߣ�����������ȫ���߿���������������߽�㷶Χ�ڼ���Ѱ�Һ��ʿ�ȵİ���*/
		for(i = Left_point[0] + 3;i < Right_point[0] - 2;i++){//����½���
			//�м��⵽�½���
			if(((Binary_Pixel_1[i+1] | Binary_Pixel_1[i+2] | Binary_Pixel_1[i+3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i-1] && Binary_Pixel_1[i-2]) == 1)){
				Right_point[1] = i;break;
			}
			//�м�û�м�⵽�½��أ���ȫ�׵�ʵ�����
			else Right_point[1] = Right_point[0];//��Ӧȫ��������м��½������ұ߽���Ӧ
			//else Mid_line_1 = Mid_line_last_1;
		}
		Line_Width = Right_point[1] - Left_point[0];
		if(Line_Width < 0)Mid_line_1 = (Left_point[0] + Right_point[0])/2;         //֮ǰ���߿�����Ҫ�󣬵��ǲ������������ߣ�����һ��ȷ��������Ϊ׼
		if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[0] + Right_point[1])/2;//ȷ������λ��
		else{
			/*��������˵�����ٳ����������ߣ��ҵ�һ���߲�����Ҫ�����ȫ��*/
			if(Right_point[0] - Right_point[1] < 4) Mid_line_1 = Mid_line_last_1;  //ȫ�����
			else{
				for(i = Right_point[1] + 4;i < Right_point[0];i++){
				    if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
				        Left_point[1] = i;break;//�����ر߽��ֵΪ0
				    }
			    }
				Line_Width = Right_point[0] - Left_point[1];
				if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[1] + Right_point[0])/2;//ȷ������λ��
				else Mid_line_1 = Right_point[0];
			}
//			for(i = Right_point[1] + 4;i < Right_point[0];i++){
//				if(((Binary_Pixel_1[i-1] | Binary_Pixel_1[i-2] | Binary_Pixel_1[i-3]) == 0)&&((Binary_Pixel_1[i] && Binary_Pixel_1[i+1] && Binary_Pixel_1[i+2]) == 1)){
//					Left_point[1] = i;break;//�����ر߽��ֵΪ0
//				}
//			}
//			Line_Width = Right_point[0] - Left_point[1];
//			if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_1 = (Left_point[1] + Right_point[0])/2;//ȷ������λ��
//			else Mid_line_1 = Right_point[0];
			//----------������д���ˣ��������������������ߵ������������ȫ���������
		}
	}
	Mid_line_last_1 = Mid_line_1;
}

void Get_WhiteLine_0(void){
	int i;
//	int Line_Width;
	int Left_point[3]={10},Right_point[3]={118};
	/*-----�ӳ�ʼ�˿�ʼ�����ҵ��������-----*/
	for(i = STARTPOINT;i < ENDPOINT;i++){//������Ѱ��
		if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
			Left_point[0] = i;break;//�����ر߽��ֵΪ0
		}
	}
	/*-----�ӽ����˿�ʼ�����ҵ��������-----*/
	for(i = ENDPOINT;i > STARTPOINT;i--){//�½���Ѱ��
		if(((Binary_Pixel_0[i+1] | Binary_Pixel_0[i+2] | Binary_Pixel_0[i+3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i-1] && Binary_Pixel_0[i-2]) == 1)){
			Right_point[0] = i;break;//�½��ر߽��ֵΪ0
		}
	}
	Line_Width = Right_point[0] - Left_point[0];
	if(Line_Width < 0)Mid_line_0 = 0;
	if(Line_Width == 0)Mid_line_0 =Left_point[0];//ֻ��һ������㣬�����ڱ�Ե�����������Ϊ����
	//�߿�����Ҫ��˵���������ȷ����������
	else if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[0] + Right_point[0])/2;//ȷ������λ��
	else{
		/*�������ִ�е��ˣ�˵���߽�㲻����Ҫ���и��ŵİ��ߣ����������߽�㷶Χ�ڼ���Ѱ�Һ��ʿ�ȵİ���*/
		for(i = Left_point[0] + 3;i < Right_point[0] - 2;i++){//����½���
			if(((Binary_Pixel_0[i+1] | Binary_Pixel_0[i+2] | Binary_Pixel_0[i+3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i-1] && Binary_Pixel_0[i-2]) == 1)){
				Right_point[1] = i;break;
			}
			else Right_point[1] = Right_point[0];
		}
		Line_Width = Right_point[1] - Left_point[0];
		if(Line_Width < 0)Mid_line_0 = (Left_point[0] + Right_point[0])/2;//֮ǰ���߿�����Ҫ�󣬵��ǲ������������ߣ�����һ��ȷ��������Ϊ׼
		if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[0] + Right_point[1])/2;//ȷ������λ��
		else{
			/*��������˵�����ٳ����������ߣ��ҵ�һ���߲�����Ҫ��*/
			if(Right_point[1] - Right_point[0] < 4)Mid_line_0 = Mid_line_last_0;
			else{
				for(i = Right_point[1] + 4;i < Right_point[0];i++){
				    if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
						Left_point[1] = i;break;//�����ر߽��ֵΪ0
					}
				}
				Line_Width = Right_point[0] - Left_point[1];
				if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[1] + Right_point[0])/2;//ȷ������λ��
				else Mid_line_0 = Right_point[0];
			    //----------������д���ˣ��������������������ߵ��������
			}
//			for(i = Right_point[1] + 4;i < Right_point[0];i++){
//				if(((Binary_Pixel_0[i-1] | Binary_Pixel_0[i-2] | Binary_Pixel_0[i-3]) == 0)&&((Binary_Pixel_0[i] && Binary_Pixel_0[i+1] && Binary_Pixel_0[i+2]) == 1)){
//					Left_point[1] = i;break;//�����ر߽��ֵΪ0
//				}
//			}
//			Line_Width = Right_point[0] - Left_point[1];
//			if(Line_Width > LINE_WIDTH_MIN &&Line_Width < LINE_WIDTH_MAX) Mid_line_0 = (Left_point[1] + Right_point[0])/2;//ȷ������λ��
//			else Mid_line_0 = Right_point[0];
			//----------������д���ˣ��������������������ߵ��������
		}
	}
}
