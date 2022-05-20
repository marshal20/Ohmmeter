#include "music_mode.h"
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


void render_number(s32 num);


// keys
#define KEY_C  0
#define KEY_CS 1
#define KEY_D  2
#define KEY_DS 3
#define KEY_E  4
#define KEY_F  5
#define KEY_FS 6
#define KEY_G  7
#define KEY_GS 8
#define KEY_A  9
#define KEY_AS 10
#define KEY_B  11
// notes
typedef u8 Note;
#define NOTE(key, octave) (((octave)<<4)&0xF0) | ((key)&0x0F)
static void play_note(Note note);

// Song
#define SONG_NOTES_COUNT 40
#define SONG_OCTAVE 4
const Note SONG_NOTES[SONG_NOTES_COUNT] = {
	// 1st
	NOTE(KEY_C, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_B, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_B, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_B, SONG_OCTAVE),
	NOTE(KEY_C, SONG_OCTAVE+1),
	// 2nd
	NOTE(KEY_C, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_B, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_A, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	// 3rd
	NOTE(KEY_C, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_AS, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	// 4th
	NOTE(KEY_C, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_G, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	// 5th
	NOTE(KEY_C, SONG_OCTAVE),
	NOTE(KEY_D, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	NOTE(KEY_F, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	NOTE(KEY_D, SONG_OCTAVE),
	NOTE(KEY_E, SONG_OCTAVE),
	NOTE(KEY_D, SONG_OCTAVE),
};
#define SONG_NOTES_DELAY 450 //// 70 BPM: note delay = 70/60/4 = 583


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


void music_mode(void)
{
	tone_init();
	input_init();
	adc_init(ADC_PRESCALER_16, ADC_REF_AVCC);

	// welcome text
	lcd_clear();
	lcd_send_string("Music Mode");

	// welcome tone
	tone_start(keys_octave8_frequencies_table[0]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[2]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[4]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[5]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[7]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[9]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[10]>>4); _delay_ms(250);
	tone_start(keys_octave8_frequencies_table[11]>>4); _delay_ms(250);
	tone_stop();

	u8 octave = 4;
	lcd_clear(); lcd_send_string("Octave: "); render_number(octave);
	Key last_active_key = KEY_NONE;
	while (1)
	{
		// check for exit, if so, stop the tone, reset adc reference to internal 2.56 V
		if (!GET_BIT(PORT_PIN(PORT_A), 6))
		{
			u8 counter = 0;
			while(!GET_BIT(PORT_PIN(PORT_A), 6)) // wait for the button to release
			{
				if (counter < 255)
				{
					counter++;
				}
				_delay_ms(10);
			}
			if (counter > 50)
			{
				// pressed for more than 500 ms
				tone_stop();
				adc_init(ADC_PRESCALER_2, ADC_REF_AVCC);
				break;
			}
			else
			{
				for (u8 i = 0; i < SONG_NOTES_COUNT; i++)
				{
					play_note(SONG_NOTES[i]);
					_delay_ms(SONG_NOTES_DELAY);
				}
				tone_stop();
			}
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


static void play_note(Note note)
{
	// extract octave and key
	u8 octave = (note>>4)&0xF;
	u8 key = note&0xF;

	// play note
	u16 frequency = keys_octave8_frequencies_table[key%12] >> (8-octave);
	tone_start(frequency);
}
