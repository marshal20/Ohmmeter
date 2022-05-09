#ifndef __ADC_INTERFACE_H__
#define __ADC_INTERFACE_H__


#define ADC_PRESCALER_2   1
#define ADC_PRESCALER_4   2
#define ADC_PRESCALER_8   3
#define ADC_PRESCALER_16  4
#define ADC_PRESCALER_32  5
#define ADC_PRESCALER_64  6
#define ADC_PRESCALER_128 7


#define ADC_REF_AREF          0
#define ADC_REF_AVCC          1
#define ADC_REF_INTERNAL_2V56 2


void adc_init(u8 prescaler, u8 reference);

u16 adc_convert(u8 channel); /*channel from 0 to 7*/


#endif /* __ADC_INTERFACE_H__ */
