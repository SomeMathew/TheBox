###### User settings ######

TOOLROOT = /home/matt/opt/Arduino/arduino-1.8.7/hardware/tools/avr
PRG = $(notdir $(CURDIR))
USRSRCS = main.c spi_command.c
LIBSSRCS = command.c uart.c spi.c i2c.c servo.c box_control.c bbb_commands.c

# Directory locations
OBJDIR = bin
SRCDIR = src
INCDIR = inc

#Target mcu and config
MCU_TARGET = atmega328p
CLOCK = 16000000
ISPTOOL	?= arduino
ISPPORT	?= /dev/ttyUSB0
ISPSPEED ?= 57600
DUDECONF = ../libs/avrdude.conf

###### END User Settings ######

LIBS = libs

LIBSSRCDIR = $(LIBS)/$(SRCDIR)
LIBSINCDIR = $(LIBS)/$(INCDIR)


#Tools
TOOLBINROOT = $(TOOLROOT)/bin
CC = $(TOOLBINROOT)/avr-gcc
OC = $(TOOLBINROOT)/avr-objcopy
AD = $(TOOLBINROOT)/avrdude


#Compile settings
INCLUDES = $(INCDIR) $(LIBSINCDIR)
OBJS = $(addprefix $(OBJDIR)/, $(USRSRCS:.c=.o) $(LIBSSRCS:.c=.o))
override CFLAGS = -std=gnu11 -Wall -Os -mmcu=$(MCU_TARGET) -ffunction-sections -fdata-sections -DF_CPU=$(CLOCK) $(addprefix -I, $(INCLUDES))

# Search path for standard files
vpath %.c $(SRCDIR)
vpath %.c $(LIBSSRCDIR)
vpath %.h $(INCDIR)
vpath %.h $(LIBSINCDIR)

# Binaries to create
ELF = $(OBJDIR)/$(PRG).elf
HEX = $(OBJDIR)/$(PRG).hex

# Targets
all: $(ELF) text
text: $(HEX)

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -MM $(CFLAGS) $< > $(@D)/$*.d
	
$(HEX): $(ELF)
	$(OC) -j .data -j .text -O ihex $< $@

clean: 
	rm -f $(OBJS) $(OBJS:.o=.d) $(ELF) $(HEX)


upload: $(HEX) all
	$(AD) -C $(DUDECONF) -v -c $(ISPTOOL) \
	-p $(MCU_TARGET) -P $(ISPPORT) -b $(ISPSPEED) \
	-U flash:w:$(HEX):i
