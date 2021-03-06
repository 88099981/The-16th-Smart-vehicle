/**********************************************************
**0000000000000000000000000000000000000000000000000000000**
**00                                                   00**
**00               东秦ai视觉一队 编写                 00**
**00                  作者：刘坤龙                     00**
**00                                                   00**
**0000000000000000000000000000000000000000000000000000000**
**00            函数作用： adc                           00**
**0000000000000000000000000000000000000000000000000000000**
**********************************************************/
#include "aicar_adc.h"
#include "aicar_init.h"


uint16  ad_value1;
uint16  ad_value2;
uint16  ad_value3;
uint16  ad_value4;
uint16  ad_value5;
uint16  ad_value6;
uint16  ad_value_all;
int16 ad_speed=0;
float lasttime_ad=0.0;
//uint16  guiyi_ad1,guiyi_ad2,guiyi_ad3,guiyi_ad4;
float ad_error=0.0;
float kp_ad=0.0, kd_ad=0.0;
float ad_left=0.0, ad_right=0.0;
int16 servo_angle=0;

void aicar_adc_init()
{
    adc_init(POWER_ADC1_MOD,POWER_ADC1_PIN,ADC_8BIT);  //同一个ADC模块分辨率应该设置为一样的，如果设置不一样，则最后一个初始化时设置的分辨率生效
    adc_init(POWER_ADC2_MOD,POWER_ADC2_PIN,ADC_8BIT);
    adc_init(POWER_ADC3_MOD,POWER_ADC3_PIN,ADC_8BIT);
    adc_init(POWER_ADC4_MOD,POWER_ADC4_PIN,ADC_8BIT);
    adc_init(POWER_ADC5_MOD,POWER_ADC5_PIN,ADC_8BIT);
    adc_init(POWER_ADC6_MOD,POWER_ADC6_PIN,ADC_8BIT);
}

void aicar_adc_get()
{
    ad_value1 = adc_mean_filter(POWER_ADC1_MOD,POWER_ADC1_PIN,10);        
    ad_value2 = adc_mean_filter(POWER_ADC2_MOD,POWER_ADC2_PIN,10);        
    ad_value3 = adc_mean_filter(POWER_ADC3_MOD,POWER_ADC3_PIN,10);        
    ad_value4 = adc_mean_filter(POWER_ADC4_MOD,POWER_ADC4_PIN,10);
    ad_value5 = adc_mean_filter(POWER_ADC5_MOD,POWER_ADC5_PIN,10);        
    ad_value6 = adc_mean_filter(POWER_ADC6_MOD,POWER_ADC6_PIN,10);
    ad_value_all=ad_value1+ad_value2+ad_value3+ad_value4+ad_value5+ad_value6;
    aicar_stop();

}

void aicar_stop(void)
{
    if(ad_value_all <= 50)
    {
        stop_cnt++;
        if(stop_cnt>=20)
        {
            break_flag=1;
            stop_cnt=20;
        }
    }
    else
    {
        stop_cnt--;
        if(stop_cnt<=0)
        {
            stop_cnt=0;
            break_flag=0;
        }
    }
}


