#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"
#include "lcd_interface.h"
#include "adc_interface.h"
#include "music_mode.h"


// Ohmmeter configurations
#define MULTIPLE_READINGS_COUNT 10
#define MULTIPLE_READINGS_DELAY 10
#define I_SELECT_PORT PORT_D
#define LOOP_DELAY 100
#define READING_MAX_RANGE 1020
#define RANGE_SWITCH_MIN 256
#define RANGE_SWITCH_MAX 900
#define ADC_MAX 1023


// Ranges:
//   0 >>> 100 mA Constant Current Source
//   1 >>> 10 mA Constant Current Source
//   2 >>> 100 Ohm Reference
//   3 >>> 1K Ohm Reference
//   4 >>> 10K Ohm Reference
//   5 >>> 100K Ohm Reference
//   6 >>> 1M Ohm Reference
//   7 >>> 10M Ohm Reference

// ranges table
static const u32 range_differential_scale_xe6[4] = {88865UL, 447909UL, 447909UL, 447909UL};
static const u32 range_resistance_reference[8] = {1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL, 1000000UL, 10000000UL};
static const char exponent_multipliers_character[3] = {' ', 'K', 'M'};
static const char exponent_multipliers_character_milli[4] = {'m', ' ', 'K', 'M'};


// helper functions
static u8 digit_to_char(u8 digit);
static u8 number_digits_count_u32(u32 number);
static s32 power10(u8 exponent);
void render_number(s32 num);
static void render_number_u32(u32 num);


static void normal_mode(void)
{
	static u8 range = 7; // 2..7
	u8 i;
    u32 reading;
	u32 reading_10x;
	u32 resistance_value;
	u32 resistance_value_milli;

	// set the range (multiplex resistance)
	PORT_PORT(I_SELECT_PORT) = ~(1<<range); // active low

	// take multiple readings then average them
	reading = 0;
	for (i = 0; i < MULTIPLE_READINGS_COUNT; i++)
	{
		reading += adc_convert(0);
		_delay_ms(MULTIPLE_READINGS_DELAY);
	}
	reading_10x = 10*reading/MULTIPLE_READINGS_COUNT;
	reading /= MULTIPLE_READINGS_COUNT;

	// show reading on the screen
	lcd_clear();
	if (reading < READING_MAX_RANGE)
	{
		// calculate the resistance = R/(1/ratio - 1) = R/(ADC_MAX/reading - 1) = R*reading/(ADC_MAX - reading)
		resistance_value = range_resistance_reference[range]*100UL/(((u32)100UL*(u32)ADC_MAX*10UL/reading_10x) - (u32)100UL);
		resistance_value_milli = range_resistance_reference[range]*reading_10x*100UL/((u32)ADC_MAX - reading_10x/10UL);

		// render resistance to the LCD
		if (resistance_value > 100UL)
		{
			u8 digits_count = number_digits_count_u32(resistance_value);
			u8 multiplier_exponent = ((digits_count>0?digits_count:1)-1)/3 * 3;
			render_number_u32(resistance_value / power10(multiplier_exponent));
			lcd_send_data('.');
			for (s8 exponent = ((s8)multiplier_exponent-1); exponent >= 0 && exponent >= ((s8)multiplier_exponent-3); exponent--)
			{
				lcd_send_data(digit_to_char(resistance_value/power10(exponent)));
			}
			lcd_send_data(exponent_multipliers_character[multiplier_exponent/3]);
			lcd_send_string(" Ohm");

			/*
			lcd_clear();
			render_number_u32(resistance_value);
			lcd_send_string(" Ohm");
			*/
		}
		else
		{
			u8 digits_count = number_digits_count_u32(resistance_value_milli);
			u8 multiplier_exponent = ((digits_count>0?digits_count:1)-1)/3 * 3;
			render_number_u32(resistance_value_milli / power10(multiplier_exponent));
			lcd_send_data('.');
			for (s8 exponent = ((s8)multiplier_exponent-1); exponent >= 0 && exponent >= ((s8)multiplier_exponent-3); exponent--)
			{
				lcd_send_data(digit_to_char(resistance_value_milli/power10(exponent)));
			}
			lcd_send_data(exponent_multipliers_character_milli[multiplier_exponent/3]);
			lcd_send_string(" Ohm");

			/*
			render_number_u32(resistance_value_milli);
			lcd_send_string("m Ohm");
			*/
		}
	}
	else
	{
		lcd_send_string("Over Load");
	}

	// switch the range
	if (range > 2 && reading < RANGE_SWITCH_MIN)
	{
		range--;
	}
	if (range < 7 && reading > RANGE_SWITCH_MAX)
	{
		range++;
	}
	// clamp range value to a valid range (2..7)
	if (range < 2)
	{
		range = 2;
	}
	if (range > 7)
	{
		range = 7;
	}
}

static void differential_mode(void)
{
	static u8 range = 0; // 0..3
	u8 i;
    u32 reading;
	u32 reading_10x;
	u32 resistance_value;
	u32 resistance_value_milli;
	u32 resistance_value_micro;

	// set the range (multiplex current)
	if (range <= 1)
	{
		PORT_PORT(I_SELECT_PORT) = ~(1<<range); // active low
	}
	else
	{
		PORT_PORT(I_SELECT_PORT) = ~(1<<1); // active low
	}

	// take multiple readings then average them
	reading = 0;
	for (i = 0; i < MULTIPLE_READINGS_COUNT; i++)
	{
		if (range <= 1)
		{
			reading += adc_convert_P3_N2_200x();
		}
		else if (range == 2)
		{
			reading += adc_convert_P3_N2_10x();
		}
		else if (range == 3)
		{
			reading += adc_convert_P3_N2_1x();
		}
		_delay_ms(MULTIPLE_READINGS_DELAY);
	}
	reading_10x = 10*reading/MULTIPLE_READINGS_COUNT;
	reading /= MULTIPLE_READINGS_COUNT;

	// show reading on the screen
	lcd_clear();
	if (reading < READING_MAX_RANGE)
	{
		if (range <= 1)
		{
			resistance_value_micro = reading*range_differential_scale_xe6[range]/200UL;
		}
		else if (range == 2)
		{
			resistance_value_micro = reading*range_differential_scale_xe6[range]/10UL;
		}
		else if (range == 3)
		{
			resistance_value_micro = reading*range_differential_scale_xe6[range]/1UL;
		}

		// render the resistance value to the LCD
		render_number_u32(resistance_value_micro);
		lcd_send_string("u Ohm");
	}
	else
	{
		lcd_send_string("Over Load");
	}

	// switch the range
	if (range > 0 && reading < RANGE_SWITCH_MIN)
	{
		range--;
	}
	if (range < 3 && reading > RANGE_SWITCH_MAX)
	{
		range++;
	}
	// clamp range value to a valid range (0..3)
	if (range > 3)
	{
		range = 3;
	}
}

int main(void)
{
	/*
	PORT_DDR(PORT_A) = 0xFF;
	PORT_PORT(PORT_A) = 0xFF;

	while(1)
	{
	 	PORT_PORT(PORT_A) = ~PORT_PORT(PORT_A);
	 	_delay_ms(500);
	}
	
	return;
	*/

	/*
	lcd_init();
	lcd_send_string("Andrew");
	lcd_set_cursor(1, 0);
	lcd_send_string(":D :)");
	return;
	*/
	

    lcd_init();
    adc_init(ADC_PRESCALER_2, ADC_REF_AVCC);
	PORT_DDR(I_SELECT_PORT) = 0xFF; // set I_SELECT_PORT to output

	// input push buttons
	// set A4 to input pullup (Octave Up)
	CLR_BIT(PORT_DDR(PORT_A), 4);
	SET_BIT(PORT_PORT(PORT_A), 4);
	// set A5 to input pullup (Octave Down) or Differential Mode
	CLR_BIT(PORT_DDR(PORT_A), 5);
	SET_BIT(PORT_PORT(PORT_A), 5);
	// set A6 to input pullup (Music Mode)
	CLR_BIT(PORT_DDR(PORT_A), 6);
	SET_BIT(PORT_PORT(PORT_A), 6);

	u8 is_differential_mode = 0;
	while (1)
    {
		// check for music mode button
		if (!GET_BIT(PORT_PIN(PORT_A), 6))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 6)); // wait for the button to release
			music_mode();
		}

		// check for differential mode
		if (!GET_BIT(PORT_PIN(PORT_A), 5))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 5)); // wait for the button to release
			is_differential_mode = !is_differential_mode;
			if (is_differential_mode)
			{
				lcd_clear();
				lcd_send_string("DIFF MODE");
				_delay_ms(500);
			}
			else
			{
				lcd_clear();
				lcd_send_string("NORMAL MODE");
				_delay_ms(500);
			}
		}

		if (is_differential_mode)
		{
			differential_mode();
		}
		else
		{
			normal_mode();
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

static u8 number_digits_count_u32(u32 number)
{
	// calculate digits count
	u8 digits_count = 0;
	while (number != 0)
	{
		digits_count ++;
		number /= 10;
	}

	return digits_count;
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

void render_number(s32 num)
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

static void render_number_u32(u32 num)
{
	u32 temp;
	u8 digits_count;

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
		digits_count++;
		temp /= 10;
	}

	// send digits from left to right
	for (s8 i = digits_count-1; i >= 0; i--)
	{
		temp = (num/(s32)power10(i))%10;
		lcd_send_data(digit_to_char((u8)temp));
	}
}
