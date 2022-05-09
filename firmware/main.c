#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"
#include "lcd_interface.h"


int main(void)
{
    // test LCD
    lcd_init();
    lcd_set_cursor(0, 0);
    lcd_send_string("Hello from");
    lcd_set_cursor(1, 0);
    lcd_send_string("ATmega16");

	PORT_DDR(PORT_C) = 0b00000001;
	PORT_PORT(PORT_C) = 0b00000010;

	while (1)
    {
	    if(!GET_BIT(PORT_PIN(PORT_C), 1)){
	        PORT_PORT(PORT_C) = 0b00000011;
	        _delay_ms(200);
            PORT_PORT(PORT_C) = 0b00000010;
            _delay_ms(200);
        }
	}

    return 0;
}
