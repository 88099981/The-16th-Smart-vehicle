/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		main
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.24
 * @Target core		NXP RT1064DVL6A
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-04-30
 ********************************************************************************************************************/


//整套推荐IO查看Projecct文件夹下的TXT文本



//打开新的工程或者工程移动了位置务必执行以下操作
//第一步 关闭上面所有打开的文件
//第二步 project  clean  等待下方进度条走完



#include "headfile.h"


uint16 adc_value;
int main(void)
{
    DisableGlobalIRQ();
    board_init();   //务必保留，本函数用于初始化MPU 时钟 调试串口
    
    //初始化ADC_1模块 3通道 引脚为B14， 精度为12位
	//每一个ADC模块 所包含的通道，分辨率应设置为一样
	//如果设置不一样则最后一个初始化的分辨率生效
    adc_init(ADC_1,ADC1_CH3_B14,ADC_8BIT);
    
    //通过在线调试直接查看adc_value的值
	//将ADC引脚接到GND，adc_value的值在0附近
	//将ADC引脚接到3.3V，adc_value的值在255附近
    EnableGlobalIRQ(0);
    
    
    
    while(1)
    {
        //采集一次ADC
        adc_value = adc_convert(ADC_1,ADC1_CH3_B14);
        systick_delay_ms(100);
        
    }

    
}





