#include "std_types.h"
#include "bit_math.h"
#include "utils.h"

#include <avr/io.h>
#include "adc_interface.h"

void adc_init(u8 prescaler, u8 reference)
{
	if (prescaler > ADC_PRESCALER_128)
	{
		return;
	}

	// enable ADC
	SET_BIT(ADCSRA, ADEN);

	// set mode to manual trigger (disable auto trigger)
	CLR_BIT(ADCSRA, ADATE);

	// disable ADC interrupt
	CLR_BIT(ADCSRA, ADIE);

    // set prescaler (the first 3 bits in ADCSRA)
	WRITE_MASK(ADCSRA, 0b00000111, prescaler);

	// set reference type
	SET_BIT_VAL(ADMUX, REFS0, (reference == ADC_REF_AVCC || reference == ADC_REF_INTERNAL_2V56));
	SET_BIT_VAL(ADMUX, REFS1, (reference == ADC_REF_INTERNAL_2V56));

	// set ADLAR to right adjusted
	CLR_BIT(ADMUX, ADLAR);
}

u16 adc_convert(u8 channel)
{
	if (channel > 7)
	{
		return 0;
	}

	// set pin to input (high impedance mode)
	CLR_BIT(PORT_DDR(PORT_A), channel);
	CLR_BIT(PORT_PORT(PORT_A), channel);

	// set channel mux the first 3 bits in ADMUX (ADMUX[0:2])
	WRITE_MASK(ADMUX, 0b00000111, channel);
	// bits 3 and 4 in ADMUX (ADMUX[3:4]) controls the mode, in this case we choose single ended input
	CLR_BIT(ADMUX, MUX4);
	CLR_BIT(ADMUX, MUX3);

	// start conversion
	SET_BIT(ADCSRA, ADSC);

    // wait for the conversion to finish (ADIF is set)
    while (GET_BIT(ADCSRA, ADIF) == 0);

    // clear ADIF
    SET_BIT(ADCSRA, ADIF);

    // we can return ((ADCH << 8) | ADCL) but this causes proteus to complain:
    //  PC=0x0986. [AVR AD CONVERTER] Result is not written to the ADC register because it has been locked. [U1]
    // so we define ADC as the 16 bit result (ADLAR must be cleared) and return it
	return ADC;
}


