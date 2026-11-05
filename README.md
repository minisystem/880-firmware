# 880 firmware

Written for Atemga328PB in Microchip Studio

Version 1.1.4

See https://github.com/minisystem/880-firmware/tree/1.1.3 for earlier firmware versions

## Ubuntu / AVRISP mkII development

Build the application in VS Code with `make`. The output is the application-only
HEX at `TR-808-firmware/build/TR-808-firmware.hex`, matching the production
application layout. For a development board, program it with the AVRISP mkII:

```sh
avrdude -c avrispmkII -P usb -p m328pb -B 10 \
  -U flash:w:build/TR-808-firmware.hex:i \
  -U flash:v:build/TR-808-firmware.hex:i
```

The normal chip erase is intentional: it allows every application update to
replace the previous application correctly, including bits that need to change
from 0 back to 1. It also erases the bootloader on this development target.
The `program` target sets `HFUSE=0xd9` after programming so `BOOTRST` is
disabled and the application starts directly at `0x0000`. Production
programming remains a separate process: run `make init` to install the
bootloader and set `HFUSE=0xd8`, then send the application through the MIDI
SysEx bootloader.

To create the production MIDI SysEx update file from the current application:

```sh
make -C TR-808-firmware sysex PYTHON2=/usr/bin/python2
```

This writes `TR-808-firmware/build/TR-808-firmware.syx`. The target uses the
bundled Python 2 `hex2sysex` utility under
`TR-808-firmware/.bootloader-source/tools/hex2sysex/` and packages the
application-only HEX; do not use the combined ISP image for SysEx updates.
To choose a different output filename, override `SYSEX_FILE`:

```sh
make -C TR-808-firmware sysex \
  SYSEX_FILE=build/880-1.1.4-midi-accent.syx
```

To initialize a fresh board with the verified production bootloader and
recorded fuse/lock values, run `make init`. The command asks for confirmation,
erases flash, writes the bootloader, and does not write EEPROM:

```sh
make init
```

The configured values are `LFUSE=0xff`, `HFUSE=0xd8`, `EFUSE=0xf4`, and
`LOCK=0xef`. The lock value protects the bootloader from SPM writes while
leaving application self-programming available to the MIDI bootloader.
