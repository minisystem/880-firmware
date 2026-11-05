#!/usr/bin/env python3
"""Extract the production bootloader portion from an Intel HEX image."""

from pathlib import Path
import argparse


def read_hex(path: Path):
    memory = {}
    upper = 0
    for line_number, line in enumerate(path.read_text().splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        if not line.startswith(":"):
            raise ValueError(f"{path}:{line_number}: invalid Intel HEX record")
        raw = bytes.fromhex(line[1:])
        count = raw[0]
        if len(raw) != count + 5 or sum(raw) & 0xFF:
            raise ValueError(f"{path}:{line_number}: invalid Intel HEX record")
        address = (raw[1] << 8) | raw[2]
        record_type = raw[3]
        data = raw[4:4 + count]
        if record_type == 0:
            for offset, value in enumerate(data):
                memory[upper + address + offset] = value
        elif record_type == 1:
            break
        elif record_type == 4:
            upper = int.from_bytes(data, "big") << 16
        elif record_type == 2:
            upper = int.from_bytes(data, "big") << 4
    return memory


def write_hex(path: Path, memory):
    with path.open("w") as output:
        addresses = sorted(memory)
        index = 0
        while index < len(addresses):
            end = index + 1
            while end < len(addresses) and addresses[end] == addresses[end - 1] + 1:
                end += 1
            for offset in range(index, end, 16):
                chunk = addresses[offset:min(offset + 16, end)]
                address = chunk[0]
                data = bytes(memory[a] for a in chunk)
                record = bytes([len(data), address >> 8, address & 0xFF, 0]) + data
                output.write(":" + (record + bytes([-sum(record) & 0xFF])).hex().upper() + "\n")
            index = end
        output.write(":00000001FF\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--start", type=lambda value: int(value, 0), default=0x7800)
    args = parser.parse_args()

    source = read_hex(args.input)
    bootloader = {address: value for address, value in source.items() if address >= args.start}
    if not bootloader:
        raise SystemExit(f"no bootloader data found at or above 0x{args.start:04X}")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_hex(args.output, bootloader)
    print(f"bootloader: 0x{min(bootloader):04X}..0x{max(bootloader) + 1:04X}")
    print(f"wrote: {args.output}")


if __name__ == "__main__":
    main()
