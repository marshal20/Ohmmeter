#ifndef __LCD_INTERFACE_H__
#define __LCD_INTERFACE_H__

#include "std_types.h"


void lcd_send_command(u8 command);
void lcd_send_data(u8 data);
void lcd_init(void);

void lcd_clear(void);
void lcd_set_cursor(u8 row, u8 column);
void lcd_write_special_char(u8 block_number, const u8* arr);

void lcd_send_string(const char* str);

#endif /* __LCD_INTERFACE_H__ */
