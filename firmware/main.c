#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"
#include "lcd_interface.h"
#include "adc_interface.h"
#include "input.h"
#include "tone.h"


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
static const char* range_to_unit_string_table[8] = {"m", "", "", "", "K", "K", "K", "M"};
static const s8 range_point_location[8] = {0, 2, 1, 0, 2, 1, 0, 2};

// helper functions
static u8 digit_to_char(u8 digit);
static s32 power10(u8 exponent);
void render_number(s32 num);


// keys frequencies (4th octave)
static u16 keys_octave8_frequencies_table[12] = {
	4186, // C8
	4434, // C#8 
	4699, // D8	
	4978, // D#8
	5274, // E8 
	5588, // F8  
	5920, // F#8
	6272, // G8 
	6645, // G#8
	7040, // A8 
	7459, // A#8
	7902  // B8
};
static const char* keys_names_table[12] = {"C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4"};

static void music_mode(void)
{
	tone_init();
	input_init();
	adc_init(ADC_PRESCALER_2, ADC_REF_AVCC);

	// welcome text
	lcd_clear();
	lcd_send_string("Music Mode");

	// welcome tone
	tone_start(keys_octave8_frequencies_table[0]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[2]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[4]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[5]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[7]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[9]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[10]); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[11]); _delay_ms(250);
	tone_stop();

	u8 octave = 4;
	lcd_clear(); lcd_send_string("Octave: "); render_number(octave);
	Key last_active_key = KEY_NONE;
	while (1)
	{
		// check for exit, if so, stop the tone, reset adc reference to internal 2.56 V
		if (!GET_BIT(PORT_PIN(PORT_A), 6))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 6)); // wait for the button to release
			tone_stop();
			adc_init(ADC_PRESCALER_2, ADC_REF_INTERNAL_2V56);
			break;
		}

		// check for octave up button
		if (!GET_BIT(PORT_PIN(PORT_A), 4))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 4)); // wait for the button to release
			if (octave > 0)
			{
				octave--;
				lcd_clear(); lcd_send_string("Octave: "); render_number(octave);
			}
		}
		// check for octave down button
		if (!GET_BIT(PORT_PIN(PORT_A), 5))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 5)); // wait for the button to release
			if (octave < 8)
			{
				octave++;
				lcd_clear(); lcd_send_string("Octave: "); render_number(octave);
			}
		}

		input_update();

		// play the active key
		Key active_key = input_get_active_key();
		if (active_key != last_active_key)
		{
			if (active_key != KEY_NONE)
			{
				u16 frequency = keys_octave8_frequencies_table[(active_key-1)%12] >> (8-octave);
				//lcd_clear();
				//lcd_send_string(keys_names_table[(active_key-1)]);
				tone_start(frequency);
			}
			else
			{
				//lcd_clear();
				tone_stop();
			}
			last_active_key = active_key;
		}

		// delay a bit
		_delay_ms(1);
	}
}

int main(void)
{
    lcd_init();
    adc_init(ADC_PRESCALER_2, ADC_REF_INTERNAL_2V56);
	PORT_DDR(I_SELECT_PORT) = 0xFF; // set I_SELECT_PORT to output

	// set A4 to input pullup (Octave Up)
	CLR_BIT(PORT_DDR(PORT_A), 4);
	SET_BIT(PORT_PORT(PORT_A), 4);
	// set A5 to input pullup (Octave Down) or Differential Mode
	CLR_BIT(PORT_DDR(PORT_A), 5);
	SET_BIT(PORT_PORT(PORT_A), 5);
	// set A6 to input pullup (Music Mode)
	CLR_BIT(PORT_DDR(PORT_A), 6);
	SET_BIT(PORT_PORT(PORT_A), 6);

	u8 range = 7; // 0..7
	u8 i;
    u32 reading;
	while (1)
    {
		// check for music mode button
		if (!GET_BIT(PORT_PIN(PORT_A), 6))
		{
			while(!GET_BIT(PORT_PIN(PORT_A), 6)); // wait for the button to release
			music_mode();
		}

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
