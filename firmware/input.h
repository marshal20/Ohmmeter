#ifndef __INPUT_H__
#define __INPUT_H__

#include "std_types.h"

#define INPUT_ADC_CHANNEL 7
#define KEY_NONE 0

typedef u8 Key;

void input_init(void);
void input_update(void);
Key input_get_active_key(void);
Key input_get_pressed_key(void);
Key input_get_released_key(void);
u8 input_get_press_duration(void);

#endif // __INPUT_H__
