#include "config.h"
#include <avr/io.h>
#include <util/delay.h>

#include "std_types.h"
#include "bit_math.h"
#include "lcd_config.h"
#include "lcd_interface.h"
#include "utils.h"


#define CMD_FUNC 0b00110000
#define CMD_FUNC_ONELINE (0<<3)
#define CMD_FUNC_TWOLINE (1<<3)
#define CMD_FUNC_5X7     (0<<2)
#define CMD_FUNC_NOT5X7  (1<<2)

#define CMD_DISP 0b00001000
#define CMD_DISP_DISP_ON    (1<<2)
#define CMD_DISP_DISP_OFF   (0<<2)
#define CMD_DISP_CURSOR_ON  (1<<1)
#define CMD_DISP_CURSOR_OFF (0<<1)
#define CMD_DISP_BLINK_ON   (1<<0)
#define CMD_DISP_BLINK_OFF  (0<<0)

#define CMD_CLEAR 1

#define CMD_ENTRY 0b00000100
#define CMD_ENTRY_INCREMENT     (1<<1)
#define CMD_ENTRY_DECREMENT     (0<<1)
#define CMD_ENTRY_DISP_SHIFT    (1<<0)
#define CMD_ENTRY_NO_DISP_SHIFT (0<<0)


void lcd_send_command(u8 command)
{
	// RS = 1 (data)
	CLR_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_RS_PIN);
	// RW = 0 (write)
	CLR_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_RW_PIN);
	// set data port
	PORT_PORT(LCD_DATA_PORT) = command;

	// clock the enable pin
	SET_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_EN_PIN);
	_delay_ms(2);
	CLR_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_EN_PIN);
}

void lcd_send_data(u8 data)
{
	// RS = 1 (data)
	SET_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_RS_PIN);
	// RW = 0 (write)
	CLR_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_RW_PIN);
	// set data port
	PORT_PORT(LCD_DATA_PORT) = data;

	// clock the enable pin
	SET_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_EN_PIN);
	_delay_ms(2);
	CLR_BIT(PORT_PORT(LCD_CTRL_PORT), LCD_EN_PIN);
}

void lcd_init(void)
{
	// set data pin direction to output
	PORT_DDR(LCD_DATA_PORT) = 0xFF;
	
	// set ctrl pins direction to output
	SET_BIT(PORT_DDR(LCD_CTRL_PORT), LCD_RS_PIN);
	SET_BIT(PORT_DDR(LCD_CTRL_PORT), LCD_RW_PIN);
	SET_BIT(PORT_DDR(LCD_CTRL_PORT), LCD_EN_PIN);

	// wait for LCD to initialize
	_delay_ms(30);

	// function set: 2 lines, 5*8 font size
	lcd_send_command(CMD_FUNC | CMD_FUNC_TWOLINE | CMD_FUNC_5X7);
	// diplay ON OFF
	lcd_send_command(CMD_DISP | CMD_DISP_DISP_ON | CMD_DISP_CURSOR_OFF | CMD_DISP_BLINK_OFF);

	// clear command
	lcd_send_command(CMD_CLEAR);

	// entry mode
	lcd_send_command(CMD_ENTRY | CMD_ENTRY_INCREMENT | CMD_ENTRY_NO_DISP_SHIFT);
}


void lcd_clear(void)
{
	lcd_send_command(CMD_CLEAR);
}

void lcd_set_cursor(u8 row, u8 column)
{
	if (row > 1 || column > 16)
	{
		return;
	}

	u8 address = row*0x40 + column;
	lcd_send_command(0b10000000 | address);
}

void lcd_write_special_char(u8 block_number, const u8* arr)
{
	if (block_number >= 8)
	{
		return;
	}

	u8 address = block_number*8;

	// set CGRAM address
	lcd_send_command(0b01000000 | address);

	// send data to CGRAM
	for (u8 i = 0; i < 8; i++)
	{
		lcd_send_data(arr[i]);
	}
}

void lcd_send_string(const char* str)
{
	while (*str)
	{
		lcd_send_data(*str);
		str++;
	}
}


