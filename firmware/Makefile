MCU = atmega16
CFLAGS = -g -Wall -mcall-prologues -mmcu=$(MCU) -Os -fshort-enums
LDFLAGS = -Wl,-gc-sections -Wl,-relax
CC = avr-gcc
BUILD_DIR = ./build/
SRC = main.c lcd_implementation.c adc_implementation.c input.c tone.c music_mode.c
TARGET = Ohmmeter

OBJ = $(SRC:.c=.o)
OBJ_PATH = $(addprefix $(BUILD_DIR), $(OBJ))

all: $(TARGET).hex

clean:
	rm -f $(OBJ_PATH) $(BUILD_DIR)$(TARGET).elf $(BUILD_DIR)$(TARGET).hex

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)$< $(BUILD_DIR)$@
	avr-size $(BUILD_DIR)$< $(BUILD_DIR)$@

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ_PATH) -o $(BUILD_DIR)$@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $(BUILD_DIR)$@

program: $(TARGET).hex
	avrdude -c usbasp -p $(MCU) -B12 -U flash:w:$(BUILD_DIR)$<:i

