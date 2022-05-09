#ifndef __UTILS_H__
#define __UTILS_H__

#include <avr/io.h>


#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3

#define PORT_PORT(port) (*port_port(port))
#define PORT_DDR(port)  (*port_ddr(port))
#define PORT_PIN(port)  (*port_pin(port))

static inline volatile u8* port_port(const u8 port)
{
    switch (port)
    {
        case PORT_A: return &PORTA;
        case PORT_B: return &PORTB;
        case PORT_C: return &PORTC;
        case PORT_D: return &PORTD;
        default: return (volatile u8*)0;
    }
}

static inline volatile u8* port_ddr(const u8 port)
{
    switch (port)
    {
        case PORT_A: return &DDRA;
        case PORT_B: return &DDRB;
        case PORT_C: return &DDRC;
        case PORT_D: return &DDRD;
        default: return (volatile u8*)0;
    }
}

static inline volatile u8* port_pin(const u8 port)
{
    switch (port)
    {
        case PORT_A: return &PINA;
        case PORT_B: return &PINB;
        case PORT_C: return &PINC;
        case PORT_D: return &PIND;
        default: return (volatile u8*)0;
    }
}

// Example:
//   PORT_PORT(PORT_A) === PORTA
//   PORT_DDR(PORT_A)  === DDRA
//   PORT_PIN(PORT_A)  === PINA

#endif /*__UTILS_H__*/

