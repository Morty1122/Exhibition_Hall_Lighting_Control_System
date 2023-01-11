#include <msp430f5529.h>
unsigned int icnt;

void AD_Init()
{
    // 在ADC12ENC==0时（默认）,初始化各寄存器，后打开ADC转换使能（ADC12ENC==1）
    //多路采样转换（首次需要SHI信号上升沿触发采样定时器）自动循环采样转换，
    ADC12CTL0 |=ADC12MSC ;
    //启动（打开）ADC12模块，ADC12ON
    ADC12CTL0 |=ADC12ON ;
    //选择单通道循环采样转换，10B,单通道重复转换模式
    ADC12CTL1 |=ADC12CONSEQ_3+ ADC12SSEL_1;
    //采样保持模式选择，选择采样信号（SAMPCON）的来源是采样定时器,选择ACLK
    ADC12CTL1 |= ADC12SHP;
    //选择通道A0,A2,P6.0采集光信号,P6.2采集麦克风信号
    ADC12MCTL0|=ADC12INCH_0;
    ADC12MCTL1 |= ADC12INCH_2+ADC12EOS;
    //两路采样对应P6.0和P6.2
    ADC12CTL1|=ADC12CSTARTADD_0;
    //ADC转换使能 ADC12ENC
    ADC12CTL0 |=ADC12ENC;
}

void initClock()
{
    //最终效果：MCLK:16MHZ,SMCLK:8MHZ,ACLK:32KHZ
    P5SEL|=BIT4+BIT5;
    UCSCTL6&=~(XT1OFF);//启动XT1
    P5SEL|=BIT2+BIT3; //XT2引脚功能选择
    UCSCTL6&=~(XT2OFF); //启动XT2
    //设置系统时钟生成器1,FLL control loop关闭SCG0=1,关闭锁频环，用户自定义
    //UCSCTL0~1工作模式
    __bis_SR_register(SCG0);
    //手动选择DCO频率阶梯（8种阶梯），确定DCO频率大致范围。
    UCSCTL0 = DCO0+DCO1+DCO2+DCO3+DCO4; //DCO频率范围在28.2MHz以下，DCO频率范围选择（三个bit位，改变直流发生器电
    //压，进而改变DCO输出频率）
    UCSCTL1 = DCORSEL_4;
    //fDCOCLK/32，锁频环分频器
    UCSCTL2 = FLLD_5;
    //n=8,FLLREFCLK时钟源为XT2CLK
    //DCOCLK=D*(N+1)*(FLLREFCLK/n)
    //DCOCLKDIV=(N+1)*(FLLREFCLK/n)
    UCSCTL3 = SELREF_5 + FLLREFDIV_3;
    //ACLK的时钟源为DCOCLKDIV,MCLK\SMCLK的时钟源为DCOCLK
    UCSCTL4 = SELA_4 + SELS_3 +SELM_3;
    //ACLK由DCOCLKDIV的32分频得到，SMCLK由DCOCLK的2分频得到
    UCSCTL5 = DIVA_5 +DIVS_1;
    // __bic_SR_register(SCG0); //Enable the FLL control loop
}

void ioinit(void)
{
    P8DIR |= BIT1;       // 设置P8.1口为输出模式  控制LED灯
    P8OUT &= ~BIT1;      // 选中P8.1为输出方式
    P3DIR |= BIT7;       // 设置P3.7口为输出模式  控制LED灯
    P3OUT &= ~BIT7;      // 选中P3.7为输出方式
    P7DIR |= BIT4;       // 设置P7.4口为输出模式  控制LED灯
    P7OUT &= ~BIT4;      // 选中P7.4为输出方式
    P6DIR |= BIT3;       // 设置P6.3口为输出模式  控制LED灯
    P6OUT &= ~BIT3;      // 选中P6.3为输出方式
    P6DIR |= BIT4;       // 设置P6.4口为输出模式  控制LED灯
    P6OUT &= ~BIT4;      // 选中P6.4为输出方式
    P3DIR |= BIT5;       // 设置P3.5口为输出模式  控制LED灯
    P3OUT &= ~BIT5;      // 选中P3.5为输出方式
    //配置大功率LED初始化
    P1DIR |= BIT5;
    P1OUT |= BIT5;
    P2DIR |= BIT4;
    P2OUT &= ~BIT4;
    P2DIR |= BIT5;
    P2SEL |= BIT5;
}

int main(void)
{
    int value=0;//光采样
    int sw=0;//麦克风采样
    int i=0;
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    AD_Init();
    initClock();
    ioinit();

    TA2CTL = TASSEL__SMCLK+TACLR+MC_1;
    TA2CCR0 = 10000;   // PWM周期,计算一下
    TA2CCTL2 = OUTMOD_7;// 输出模式7

    TA0CTL |= MC_1 + TASSEL__ACLK + TACLR;       //时钟为ACLK, 比较模式，开始时清零计数器
    TA0CCTL0 = CCIE;                             //CCR0比较器中断使能
    TA0CCR0  = 32768;                            //每次间隔时间为1s

    __enable_interrupt();                     //全局中断使能

    while (1)
    {
        value=ADC12MEM0;
        sw=ADC12MEM1;
        ADC12CTL0 |= ADC12SC;
        if(sw>=4000)//决定大功率LED亮灭
        {
            P2OUT &= ~BIT4;
            TA0CTL |=  TACLR;                       // 时钟为ACLK,清零计数器，
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
        //根据光信号强弱来控制LED1~LED6的开关
        if(value<=2500)
        {
            P8OUT|= BIT1;       //LED1亮
        }
        else
        {
            P8OUT&= ~BIT1;       //LED1灭
        }
        if(value<=2000)
        {
            P3OUT|= BIT7;       //LED2亮
        }
        else
        {
            P3OUT&= ~BIT7;       //LED2灭
        }
        if(value<=1500)
        {
            P7OUT|= BIT4;       //LED3亮
        }
        else
        {
            P7OUT&= ~BIT4;       //LED3灭
        }
        if(value<=1000)
        {
            P6OUT|= BIT3;       //LED4亮
        }
        else
        {
            P6OUT&= ~BIT3;       //LED4灭
        }
        if(value<=500)
        {
            P6OUT|= BIT4;       //LED5亮
        }
        else
        {
            P6OUT&= ~BIT4;       //LED5灭
        }
        if(value<=500)
        {
            P3OUT|= BIT5;       //LED6亮
        }
        else
        {
            P3OUT&= ~BIT5;       //LED6灭
        }

    }
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    icnt++;

    if(icnt==15)     // 进中断，累积到15s才关灯
    {
       P2OUT |= BIT4;              //关灯
       icnt=0;
    }

}
