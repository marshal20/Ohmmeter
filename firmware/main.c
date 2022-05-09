#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "std_types.h"
#include "bit_math.h"
#include "utils.h"


int main(void)
{
	PORT_DDR(PORT_A) = 0b00000001;
	PORT_PORT(PORT_A) = 0b00000010;

	while (1)
    {
	    if(!GET_BIT(PINA,1)){
	        PORT_PORT(PORT_A) = 0b00000011;
	        _delay_ms(200);
            PORT_PORT(PORT_A) = 0b00000010;
            _delay_ms(200);
        }
	}

    return 0;
}
