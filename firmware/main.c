#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"
#include "lcd_interface.h"
#include "adc_interface.h"


static u8 digit_to_char(u8 digit);
static s32 power10(u8 exponent);
static void render_number(s32 num);


int main(void)
{
    lcd_init();
    adc_init(ADC_PRESCALER_2, ADC_REF_AVCC);

    // test LCD
    lcd_set_cursor(0, 0);
    lcd_send_string("Hello from");
    lcd_set_cursor(1, 0);
    lcd_send_string("ATmega16");

    _delay_ms(500);
    lcd_clear();

    u16 reading;
	while (1)
    {
        reading = adc_convert(0);
        render_number(reading);
        _delay_ms(200);
        lcd_clear();
	}

    return 0;
}


static u8 digit_to_char(u8 digit)
{
	return '0' + digit%10;
}

static s32 power10(u8 exponent)
{
	s32 result = 1;

	for (u8 i = 0; i < exponent; i++)
	{
		result *= 10;
	}

	return result;
}

static void render_number(s32 num)
{
	s32 temp;
	u8 digits_count;

	if (num < 0)
	{
		lcd_send_data('-');
		num = -num;
	}

	if (num == 0)
	{
		lcd_send_data('0');
		return;
	}

	// calculate digits count
	digits_count = 0;
	temp = num;
	while (temp != 0)
	{
		digits_count ++;
		temp /= 10;
	}

	// send digits from left to right
	for (s8 i = digits_count-1; i >= 0; i--)
	{
		temp = (num/(s32)power10(i))%10;
		lcd_send_data(digit_to_char((u8)temp));
	}
}
