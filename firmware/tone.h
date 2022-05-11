#ifndef __TONE_H__
#define __TONE_H__

#include "std_types.h"
#include "utils.h"

#define TONE_OUTPUT_PORT PORT_B
#define TONE_OUTPUT_PIN 3

void tone_init(void);
void tone_start(u16 frequency);
void tone_stop(void);

#endif /*__TONE_H__*/
