#include "tone.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "bit_math.h"
#include "lcd_interface.h" // TEMP


static void tone_output_set_value(u8 value)
{
    SET_BIT_VAL(PORT_PORT(TONE_OUTPUT_PORT), TONE_OUTPUT_PIN, value);
}

static u8 tone_output_get_value(void)
{
    return GET_BIT(PORT_PORT(TONE_OUTPUT_PORT), TONE_OUTPUT_PIN);
}

ISR(TIMER2_COMP_vect)
{
    // toggle tone pin
    static u8 tone_output_value = 0;
    tone_output_value = !tone_output_value;
    tone_output_set_value(tone_output_value);
    TCNT2 = 0;
}

void tone_init(void)
{
    SET_BIT(PORT_DDR(TONE_OUTPUT_PORT), TONE_OUTPUT_PIN);
}

void render_number(s32 num);

void tone_start(uint16_t frequency)
{
    // using the 8-bit Timer/Counter2

    //cli(); // disable global interrupt

    // calculate the OCR and Prescaler for the timer
    u32 ocr = 0;
    static const uint16_t prescales[7] = {1, 8, 32, 64, 128, 256, 1024};
    u8 prescaler = 1; // prescale 0 is timer off
    for(; prescaler < 8; prescaler++)
    {
        ocr = (u32)F_CPU / prescales[prescaler-1] / 2 / frequency - 1;
        if(ocr <= 255) // try all prescales to get ocr < 255
        {
            break;
        }
    }
    if(ocr > 255)
    {
        ocr = 255;
    }

    // lcd_set_cursor(1, 0);
    // render_number(prescaler);
    // lcd_send_data(' ');
    // render_number(ocr);

    // Clear Timer on Compare match (CTC) mode /*(1<WGM21) | (0<WGM20) |*/ 
    TCCR2 = (prescaler<<CS20);
    OCR2 = (u8)ocr;
    TCNT2 = 0; // counter value
    TIMSK |= (1<<OCIE2); // Timer/Counter2 Output Compare Match Interrupt Enable
    sei(); // enable global interrupts
}

void tone_stop(void)
{
    TIMSK &= ~(1<<OCIE2); // stop the interrupt
    cli(); // disable global interrupt
    tone_output_set_value(0);
}


