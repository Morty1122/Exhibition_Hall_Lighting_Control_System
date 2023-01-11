#include <msp430f5529.h>
unsigned int icnt;

void AD_Init()
{
    // ��ADC12ENC==0ʱ��Ĭ�ϣ�,��ʼ�����Ĵ��������ADCת��ʹ�ܣ�ADC12ENC==1��
    //��·����ת�����״���ҪSHI�ź������ش���������ʱ�����Զ�ѭ������ת����
    ADC12CTL0 |=ADC12MSC ;
    //�������򿪣�ADC12ģ�飬ADC12ON
    ADC12CTL0 |=ADC12ON ;
    //ѡ��ͨ��ѭ������ת����10B,��ͨ���ظ�ת��ģʽ
    ADC12CTL1 |=ADC12CONSEQ_3+ ADC12SSEL_1;
    //��������ģʽѡ��ѡ������źţ�SAMPCON������Դ�ǲ�����ʱ��,ѡ��ACLK
    ADC12CTL1 |= ADC12SHP;
    //ѡ��ͨ��A0,A2,P6.0�ɼ����ź�,P6.2�ɼ���˷��ź�
    ADC12MCTL0|=ADC12INCH_0;
    ADC12MCTL1 |= ADC12INCH_2+ADC12EOS;
    //��·������ӦP6.0��P6.2
    ADC12CTL1|=ADC12CSTARTADD_0;
    //ADCת��ʹ�� ADC12ENC
    ADC12CTL0 |=ADC12ENC;
}

void initClock()
{
    //����Ч����MCLK:16MHZ,SMCLK:8MHZ,ACLK:32KHZ
    P5SEL|=BIT4+BIT5;
    UCSCTL6&=~(XT1OFF);//����XT1
    P5SEL|=BIT2+BIT3; //XT2���Ź���ѡ��
    UCSCTL6&=~(XT2OFF); //����XT2
    //����ϵͳʱ��������1,FLL control loop�ر�SCG0=1,�ر���Ƶ�����û��Զ���
    //UCSCTL0~1����ģʽ
    __bis_SR_register(SCG0);
    //�ֶ�ѡ��DCOƵ�ʽ��ݣ�8�ֽ��ݣ���ȷ��DCOƵ�ʴ��·�Χ��
    UCSCTL0 = DCO0+DCO1+DCO2+DCO3+DCO4; //DCOƵ�ʷ�Χ��28.2MHz���£�DCOƵ�ʷ�Χѡ������bitλ���ı�ֱ����������
    //ѹ�������ı�DCO���Ƶ�ʣ�
    UCSCTL1 = DCORSEL_4;
    //fDCOCLK/32����Ƶ����Ƶ��
    UCSCTL2 = FLLD_5;
    //n=8,FLLREFCLKʱ��ԴΪXT2CLK
    //DCOCLK=D*(N+1)*(FLLREFCLK/n)
    //DCOCLKDIV=(N+1)*(FLLREFCLK/n)
    UCSCTL3 = SELREF_5 + FLLREFDIV_3;
    //ACLK��ʱ��ԴΪDCOCLKDIV,MCLK\SMCLK��ʱ��ԴΪDCOCLK
    UCSCTL4 = SELA_4 + SELS_3 +SELM_3;
    //ACLK��DCOCLKDIV��32��Ƶ�õ���SMCLK��DCOCLK��2��Ƶ�õ�
    UCSCTL5 = DIVA_5 +DIVS_1;
    // __bic_SR_register(SCG0); //Enable the FLL control loop
}

void ioinit(void)
{
    P8DIR |= BIT1;       // ����P8.1��Ϊ���ģʽ  ����LED��
    P8OUT &= ~BIT1;      // ѡ��P8.1Ϊ�����ʽ
    P3DIR |= BIT7;       // ����P3.7��Ϊ���ģʽ  ����LED��
    P3OUT &= ~BIT7;      // ѡ��P3.7Ϊ�����ʽ
    P7DIR |= BIT4;       // ����P7.4��Ϊ���ģʽ  ����LED��
    P7OUT &= ~BIT4;      // ѡ��P7.4Ϊ�����ʽ
    P6DIR |= BIT3;       // ����P6.3��Ϊ���ģʽ  ����LED��
    P6OUT &= ~BIT3;      // ѡ��P6.3Ϊ�����ʽ
    P6DIR |= BIT4;       // ����P6.4��Ϊ���ģʽ  ����LED��
    P6OUT &= ~BIT4;      // ѡ��P6.4Ϊ�����ʽ
    P3DIR |= BIT5;       // ����P3.5��Ϊ���ģʽ  ����LED��
    P3OUT &= ~BIT5;      // ѡ��P3.5Ϊ�����ʽ
    //���ô���LED��ʼ��
    P1DIR |= BIT5;
    P1OUT |= BIT5;
    P2DIR |= BIT4;
    P2OUT &= ~BIT4;
    P2DIR |= BIT5;
    P2SEL |= BIT5;
}

int main(void)
{
    int value=0;//�����
    int sw=0;//��˷����
    int i=0;
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    AD_Init();
    initClock();
    ioinit();

    TA2CTL = TASSEL__SMCLK+TACLR+MC_1;
    TA2CCR0 = 10000;   // PWM����,����һ��
    TA2CCTL2 = OUTMOD_7;// ���ģʽ7

    TA0CTL |= MC_1 + TASSEL__ACLK + TACLR;       //ʱ��ΪACLK, �Ƚ�ģʽ����ʼʱ���������
    TA0CCTL0 = CCIE;                             //CCR0�Ƚ����ж�ʹ��
    TA0CCR0  = 32768;                            //ÿ�μ��ʱ��Ϊ1s

    __enable_interrupt();                     //ȫ���ж�ʹ��

    while (1)
    {
        value=ADC12MEM0;
        sw=ADC12MEM1;
        ADC12CTL0 |= ADC12SC;
        if(sw>=4000)//��������LED����
        {
            P2OUT &= ~BIT4;
            TA0CTL |=  TACLR;                       // ʱ��ΪACLK,�����������
            icnt =0;
        }
        if(value<=900)
        {
            if(i>0)
            {
                i--;
                TA2CCR2 = i;
            }
        }
        else if(value>=600)
        {
            if(i<10000)
            {
                i++;
                TA2CCR2 = i;
            }
        }
        __delay_cycles(16000);
        //���ݹ��ź�ǿ��������LED1~LED6�Ŀ���
        if(value<=2500)
        {
            P8OUT|= BIT1;       //LED1��
        }
        else
        {
            P8OUT&= ~BIT1;       //LED1��
        }
        if(value<=2000)
        {
            P3OUT|= BIT7;       //LED2��
        }
        else
        {
            P3OUT&= ~BIT7;       //LED2��
        }
        if(value<=1500)
        {
            P7OUT|= BIT4;       //LED3��
        }
        else
        {
            P7OUT&= ~BIT4;       //LED3��
        }
        if(value<=1000)
        {
            P6OUT|= BIT3;       //LED4��
        }
        else
        {
            P6OUT&= ~BIT3;       //LED4��
        }
        if(value<=500)
        {
            P6OUT|= BIT4;       //LED5��
        }
        else
        {
            P6OUT&= ~BIT4;       //LED5��
        }
        if(value<=500)
        {
            P3OUT|= BIT5;       //LED6��
        }
        else
        {
            P3OUT&= ~BIT5;       //LED6��
        }

    }
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    icnt++;

    if(icnt==15)     // ���жϣ��ۻ���15s�Źص�
    {
       P2OUT |= BIT4;              //�ص�
       icnt=0;
    }

}
