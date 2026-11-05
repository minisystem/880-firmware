#!/usr/bin/env python3
"""Overlay the application HEX onto a known-good full-flash backup.

The resulting image can be written after a chip erase.  The backup supplies
the bootloader and every other byte; the new application supplies addresses
starting at 0x0000.  This is intentionally conservative and refuses to place
application data at or beyond the configured bootloader boundary.
"""

from pathlib import Path
import argparse


def read_hex(path: Path):
    memory = {}
    for line_number, line in enumerate(path.read_text().splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        if not line.startswith(":"):
            raise ValueError(f"{path}:{line_number}: invalid Intel HEX record")
        raw = bytes.fromhex(line[1:])
        count = raw[0]
        if len(raw) != count + 5:
            raise ValueError(f"{path}:{line_number}: invalid record length")
        if sum(raw) & 0xff:
            raise ValueError(f"{path}:{line_number}: checksum error")
        address = (raw[1] << 8) | raw[2]
        record_type = raw[3]
        data = raw[4:4 + count]
        if record_type == 0:
            # This project emits a 16-bit address space HEX file.  Reject
            # extended addressing rather than silently making a bad image.
            for offset, value in enumerate(data):
                memory[address + offset] = value
        elif record_type == 1:
            break
        elif record_type in (2, 4):
            raise ValueError(f"{path}:{line_number}: extended addressing is unsupported")
    return memory


def write_hex(path: Path, memory):
    with path.open("w") as output:
        addresses = sorted(memory)
        for start in range(0, len(addresses), 16):
            chunk_addresses = addresses[start:start + 16]
            if chunk_addresses != list(range(chunk_addresses[0], chunk_addresses[0] + len(chunk_addresses))):
                raise ValueError("internal error: non-contiguous output chunk")
            address = chunk_addresses[0]
            data = bytes(memory[address + i] for i in range(len(chunk_addresses)))
            record = bytes([len(data), (address >> 8) & 0xff, address & 0xff, 0]) + data
            checksum = (-sum(record)) & 0xff
            output.write(":" + (record + bytes([checksum])).hex().upper() + "\n")
        output.write(":00000001FF\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--backup", type=Path, required=True)
    parser.add_argument("--application", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--bootloader-start", type=lambda value: int(value, 0), default=0x7800)
    args = parser.parse_args()

    backup = read_hex(args.backup)
    application = read_hex(args.application)
    if not application:
        raise SystemExit("application HEX is empty")
    app_end = max(application) + 1
    if app_end > args.bootloader_start:
        raise SystemExit(
            f"application reaches 0x{app_end:04X}, at/after bootloader boundary "
            f"0x{args.bootloader_start:04X}; refusing to create image"
        )

    combined = dict(backup)
    combined.update(application)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_hex(args.output, combined)
    print(f"application: 0x{app_end:04X} exclusive")
    print(f"bootloader boundary: 0x{args.bootloader_start:04X}")
    print(f"wrote: {args.output}")


if __name__ == "__main__":
    main()
