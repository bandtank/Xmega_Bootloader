###############################################################################
# Makefile for the project Xmega_Bootloader
###############################################################################

## General Flags
PROJECT = Xmega_Bootloader

###############################################################################
# User modification section
###############################################################################
#  Choose one of the following MCUs:
#    If you have a different MCU, you will have to define the
#    BOOT_SECTION_START_IN_BYTES and FLASH_PAGE_SIZE_IN_BYTES for
#    -section-start and the number of bytes to program at a time respectively.
#    This information can be found in the iox....h file as BOOT_SECTION_START
#    and PROGMEM_PAGE_SIZE.
# MCU = atxmega128a1
# MCU = atxmega64a3
# MCU = atxmega32a4
# MCU = atxmega16a4
  MCU = atxmega16d4
  
# Choose a baud rate for the UART.
#    If you need a baud rate that is not listed in this makefile, you must add
#    new configuration statements in config.macros.h. Remember, Xmegas start-up
#    with a 2MHz clock.
# BAUD_RATE = 9600
# BAUD_RATE = 38400
# BAUD_RATE = 57600
  BAUD_RATE = 115200
  
# Specify a pin to check for entry into the bootloader. The notation is
# PORT,PIN. For example, if you wanted to use PIN 3 on PORTC, you would set
# the option as C,3. Then specifiy the logic value required to enable the
# bootloader code (1 = enable the bootloader if the pin is VCC, 0 = enable 
# the bootloader if the pin is GND).
  BOOTLOADER_PIN    = B,2
  BOOTLOADER_PIN_ON = 1
  
# Specify a pin to control an LED. The notation is PORT,PIN. For example, if
# you wanted to use PIN 6 on PORTA, you would set the option as A,6. Then
# specifiy the logic value required to enable the LED (1 = output VCC to turn
# on the LED, 0 = output GND to turn on the LED).
  LED_PIN = D,2
  LED_ON  = 0

# Specify which UART to use with PORT,NUM notation. For example, UART1 on
# PORTD would be D,1.
  UART = C,0

###############################################################################
# End user modification section
###############################################################################
  
## Set configuration options based on MCU model  
ifeq ($(MCU), atxmega128a1)
   BOOT_SECTION_START_IN_BYTES = 0x20000
   FLASH_PAGE_SIZE = 512
endif

ifeq ($(MCU), atxmega64a3)
   BOOT_SECTION_START_IN_BYTES = 0x10000
   FLASH_PAGE_SIZE = 256
endif

ifeq ($(MCU), atxmega32a4)
   BOOT_SECTION_START_IN_BYTES = 0x8000
   FLASH_PAGE_SIZE = 256
endif

ifeq ($(MCU), atxmega16a4)
   BOOT_SECTION_START_IN_BYTES = 0x4000
   FLASH_PAGE_SIZE = 256
endif

ifeq ($(MCU), atxmega16d4)
   BOOT_SECTION_START_IN_BYTES = 0x4000
   FLASH_PAGE_SIZE = 256
endif

TARGET = $(PROJECT).elf
TARGET_BASE = $(PROJECT)

CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=2000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -DFLASH_PAGE_SIZE=$(FLASH_PAGE_SIZE) -DMCU=$(MCU) -DBAUD_RATE=$(BAUD_RATE) -DMY_UART=$(UART) -DENTER_BOOTLOADER_PIN=$(BOOTLOADER_PIN) -DLED_PIN=$(LED_PIN) -DLED_ON=$(LED_ON) -DBOOTLOADER_PIN_EN=$(BOOTLOADER_PIN_ON)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(PROJECT).map
LDFLAGS += -Wl,-section-start=.text=$(BOOT_SECTION_START_IN_BYTES)

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJECTS = eeprom_driver.o $(PROJECT).o serial.o sp_driver.o CCP_Write.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: sizebefore $(TARGET) $(PROJECT).hex $(PROJECT).eep sizeafter  $(PROJECT).lss## Compile
eeprom_driver.o: eeprom_driver.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

$(PROJECT).o: $(PROJECT).c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

serial.o: serial.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sp_driver.o: sp_driver.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

CCP_Write.o: CCP_Write.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).map

## Other dependencies


# Display size of file.
FORMAT = ihex
SIZE = avr-size
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:

HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET_BASE).hex
ELFSIZE = $(SIZE) -A $(TARGET_BASE).elf
AVRMEM = avr-mem.sh $(TARGET_BASE).elf $(MCU)

sizebefore:
	@if test -f $(TARGET); then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); \
	$(AVRMEM) 2>/dev/null; echo; fi

sizeafter:
	@if test -f $(TARGET); then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); \
	$(AVRMEM) 2>/dev/null; echo; fi