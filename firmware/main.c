#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"
#include "lcd_interface.h"
#include "adc_interface.h"


// Ohmmeter configurations
#define MULTIPLE_READINGS_COUNT 10
#define MULTIPLE_READINGS_DELAY 10
#define I_SELECT_PORT PORT_D
#define LOOP_DELAY 100
#define READING_MAX_RANGE 1000
#define RANGE_SWITCH_MIN 90
#define RANGE_SWITCH_MAX 900


// Ranges:
//   0 >>> 1 = 1 m Ohm
//   1 >>> 1 = 10 m Ohm
//   2 >>> 1 = 100 m Ohm
//   3 >>> 1 = 1 Ohm
//   4 >>> 1 = 10 Ohm
//   5 >>> 1 = 100 Ohm
//   6 >>> 1 = 1 K Ohm
//   7 >>> 1 = 10 K Ohm

// ranges table
const char* range_to_unit_string_table[8] = {" m", "", "", "", " K", " K", " K", "M"};
const s8 range_point_location[8] = {0, 2, 1, 0, 2, 1, 0, 2};

// helper functions
static u8 digit_to_char(u8 digit);
static s32 power10(u8 exponent);
static void render_number(s32 num);


int main(void)
{
    lcd_init();
    adc_init(ADC_PRESCALER_2, ADC_REF_INTERNAL_2V56);
	PORT_DDR(I_SELECT_PORT) = 0xFF; // set I_SELECT_PORT to output

	u8 range = 7; // 0..7
	u8 i;
    u32 reading;
	while (1)
    {
		// set the range (multiplex current)
		PORT_PORT(I_SELECT_PORT) = ~(1<<range); // active low

		// take multiple readings then average them
		reading = 0;
		for (i = 0; i < MULTIPLE_READINGS_COUNT; i++)
		{
			reading += adc_convert(0);
			_delay_ms(MULTIPLE_READINGS_DELAY);
		}
		reading /= MULTIPLE_READINGS_COUNT;

		// show reading on the screen
        lcd_clear();
		if (reading < READING_MAX_RANGE)
		{
			// render number left to the point
			if (range_point_location[range] == 0)
			{
				render_number(reading);
			}
			else if (range_point_location[range] == 1)
			{
				render_number(reading/10);
				lcd_send_data('.');
				render_number(reading%10);
			}
			else if (range_point_location[range] == 2)
			{
				render_number(reading/100);
				lcd_send_data('.');
				render_number(reading%100);
			}

			// render unit
			//lcd_send_data(' ');
			lcd_send_string(range_to_unit_string_table[range]);
			lcd_send_string(" Ohm");
		}
		else
		{
			lcd_send_string("OL");
		}

		// update the range
		if (range > 0 && reading < RANGE_SWITCH_MIN)
		{
			range--;
		}
		if (range < 7 && reading > RANGE_SWITCH_MAX)
		{
			range++;
		}
		// clamp range value to a valid range (0..7)
		if (range > 7)
		{
			range = 7;
		}

		// delay for a bit
		_delay_ms(LOOP_DELAY);
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
