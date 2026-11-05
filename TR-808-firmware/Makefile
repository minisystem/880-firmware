MCU       := atmega328pb
TARGET    := TR-808-firmware
BUILD     := build
BOOTLOADER_HEX := $(BUILD)/production-bootloader.hex
BOOTLOADER_START := 0x7800
PRODUCTION_PROGRAM_HEX := $(BUILD)/$(TARGET)-production-test.hex
SYSEX_FILE ?= $(BUILD)/$(TARGET).syx
SYSEX_OUTPUT := $(abspath $(SYSEX_FILE))
READBACK_FILE ?= $(BUILD)/after-sysex-readback.hex
READBACK_OUTPUT := $(abspath $(READBACK_FILE))
PYTHON2 ?= python2
LFUSE := 0xff
# The bootloader is linked at byte address 0x7800.  HFUSE=0xda selects
# the 2 KiB boot section beginning there; 0xd8 would reserve 4 KiB starting
# at 0x7000 and would prevent the MIDI bootloader from writing the larger
# application image above that boundary.
HFUSE := 0xda
EFUSE := 0xf4
# Development programming erases the bootloader and boots directly at the
# application reset vector. This is HFUSE with BOOTRST disabled.
DEV_HFUSE := 0xdb
# BLB1 mode 2: SPM is prohibited in the boot section; application SPM remains
# allowed so the MIDI bootloader can update application flash.
LOCK  := 0xef

# Ubuntu's packaged AVR-GCC may not know ATmega328PB. Prefer the user-level
# installation when present, otherwise use the workspace-local toolchain.
# Override AVR_TOOLCHAIN when using another installation.
ifeq ($(wildcard $(HOME)/opt/avr8-gnu-toolchain-linux_x86_64/bin/avr-gcc),)
AVR_TOOLCHAIN ?= $(abspath ../../.toolchain/avr8-gnu-toolchain-linux_x86_64)
else
AVR_TOOLCHAIN ?= $(HOME)/opt/avr8-gnu-toolchain-linux_x86_64
endif
CC        := $(AVR_TOOLCHAIN)/bin/avr-gcc
OBJCOPY   := $(AVR_TOOLCHAIN)/bin/avr-objcopy
SIZE      := $(AVR_TOOLCHAIN)/bin/avr-size
AVRDUDE   ?= avrdude
PROGRAMMER ?= avrispmkII
AVRDUDE_PORT ?= usb
PROGRAMMER_SPEED ?= 10

# Use size optimization so the application remains below the legacy 0x7000
# boot-section boundary. Keep the original ABI/layout flags and NDEBUG setting.
CFLAGS    := -mmcu=$(MCU) -Os -Wall \
             -funsigned-char -funsigned-bitfields -fcommon \
             -fpack-struct -fshort-enums
CPPFLAGS  := -DNDEBUG
LDFLAGS   := -mmcu=$(MCU)
LDLIBS    := -lm

SOURCES   := $(wildcard *.c) \
             $(wildcard xnormidi-develop/*.c) \
             $(wildcard xnormidi-develop/bytequeue/*.c)
OBJECTS   := $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all clean size program production-image production-program sysex readback init bootloader-image

all: $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).eep size

$(BUILD)/$(TARGET).elf: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD)/$(TARGET).eep: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex -j .eeprom \
	  --set-section-flags=.eeprom=alloc,load \
	  --no-change-warnings $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

size: $(BUILD)/$(TARGET).elf
	$(SIZE) -A $<

$(BOOTLOADER_HEX): programmer-backup/production-flash.hex tools/extract_bootloader.py
	python3 tools/extract_bootloader.py \
	  --input programmer-backup/production-flash.hex \
	  --output $@ \
	  --start $(BOOTLOADER_START)

bootloader-image: $(BOOTLOADER_HEX)

# Build a complete image for production boards. The application-only HEX is
# unsafe with BOOTRST enabled because avrdude erases the bootloader first.
$(PRODUCTION_PROGRAM_HEX): $(BUILD)/$(TARGET).hex programmer-backup/production-flash.hex tools/make_program_image.py
	python3 tools/make_program_image.py \
		--backup programmer-backup/production-flash.hex \
		--application $(BUILD)/$(TARGET).hex \
		--output $@ \
		--bootloader-start $(BOOTLOADER_START)

production-image: $(PRODUCTION_PROGRAM_HEX)

production-program: $(PRODUCTION_PROGRAM_HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -P $(AVRDUDE_PORT) -p $(MCU) \
		-B $(PROGRAMMER_SPEED) \
		-U flash:w:$<:i \
		-U flash:v:$<:i

# Generate the MIDI SysEx update file used by production units. The bundled
# hex2sysex utility is Python 2 code and expects to be run from the bootloader
# source directory so its tools.midi and tools.hexfile imports resolve.
sysex: $(BUILD)/$(TARGET).hex
	@test -f .bootloader-source/tools/hex2sysex/hex2sysex.py || \
		{ echo "missing .bootloader-source/tools/hex2sysex/hex2sysex.py"; exit 1; }
	mkdir -p "$(dir $(SYSEX_OUTPUT))"
	cd .bootloader-source && $(PYTHON2) tools/hex2sysex/hex2sysex.py \
		--syx --page_size 64 \
		-o "$(SYSEX_OUTPUT)" \
		../$(BUILD)/$(TARGET).hex

# Read the complete device flash over ISP without erasing or programming it.
# This is useful for verifying a MIDI SysEx update after the fact.
readback:
	@mkdir -p "$(dir $(READBACK_OUTPUT))"
	$(AVRDUDE) -c $(PROGRAMMER) -P $(AVRDUDE_PORT) -p $(MCU) \
	  -B $(PROGRAMMER_SPEED) \
	  -U flash:r:"$(READBACK_OUTPUT)":i

# Initialize a fresh board with the verified production bootloader and
# recorded production fuse/lock values. This erases flash, writes the
# bootloader only, and does not write EEPROM.
init: bootloader-image
	@if [ "$(INIT_CONFIRM)" != "YES" ]; then \
	  printf 'This erases flash and installs the production bootloader. Type YES to continue: '; \
	  read -r confirm; \
	  [ "$$confirm" = "YES" ] || { echo 'init aborted'; exit 1; }; \
	fi
	$(AVRDUDE) -c $(PROGRAMMER) -P $(AVRDUDE_PORT) -p $(MCU) \
	  -B $(PROGRAMMER_SPEED) \
	  -e \
	  -U flash:w:$(BOOTLOADER_HEX):i \
	  -U flash:v:$(BOOTLOADER_HEX):i \
	  -U lfuse:w:$(LFUSE):m \
	  -U hfuse:w:$(HFUSE):m \
	  -U efuse:w:$(EFUSE):m \
	  -U lock:w:$(LOCK):m

# Development programming intentionally writes only the application HEX and
# disables BOOTRST. avrdude performs a chip erase first, so the bootloader is
# erased and the application must boot directly from address 0x0000.
program: $(BUILD)/$(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -P $(AVRDUDE_PORT) -p $(MCU) \
	  -B $(PROGRAMMER_SPEED) \
	  -U flash:w:$(BUILD)/$(TARGET).hex:i \
		-U flash:v:$(BUILD)/$(TARGET).hex:i \
		-U hfuse:w:$(DEV_HFUSE):m \
		-U hfuse:v:$(DEV_HFUSE):m

clean:
	rm -rf $(BUILD)
