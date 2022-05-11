#include "input.h"
#include "adc_interface.h"

#define KEYS_COUNT 12
#define KEY_SPACE 1023/KEYS_COUNT

// Decode the analog keypad
static u8 decode_key(u16 analog_value)
{
    for(u8 i = 0; i < KEYS_COUNT+1; i++)
    {
        u16 threshold = (u16)i*KEY_SPACE + KEY_SPACE/2;
        if(analog_value <= threshold)
            return i;
    }
    return 0;
}

// Get the active key
static u8 get_active_key(void)
{
        u8 row1 = decode_key(adc_convert(INPUT_ADC_CHANNEL));
        
        if(row1 != 0U)
            return row1;

        return 0U;
}

// Input state
static Key last_key;
static Key active_key;
static u8 press_duration;

void input_init(void)
{
    last_key = KEY_NONE;
    active_key = KEY_NONE;
    press_duration = 0;
}

void input_update(void)
{
    last_key = active_key;
    active_key = get_active_key();

    if(active_key == last_key && press_duration < 255)
    {
        press_duration += 1;
    }
    else if(active_key != last_key)
    {
        press_duration = 0;
    }
}

Key input_get_active_key(void)
{
    return active_key;
}

Key input_get_pressed_key(void)
{
    if(active_key != last_key)
        return active_key;

    return KEY_NONE;
}

Key input_get_released_key(void)
{
    if(active_key != last_key)
        return last_key;

    return KEY_NONE;
}

u8 input_get_press_duration(void)
{
    return press_duration;
}

