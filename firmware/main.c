#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>

#define GET_BIT(VAR,BITNO)	((VAR>>BITNO)&1)

// A0 -> OUTPUT LED
// A1 -> INPUT SWITCH

int main(void)
{
	DDRA = 0b00000001;
	PORTA = 0b00000010;

	while (1){
	    if(!GET_BIT(PINA,1)){
	        PORTA = 0b00000011;
	        _delay_ms(200);
            PORTA = 0b00000010;
            _delay_ms(200);
        }
				
	}

    return 0;
}
