#!/usr/bin/env python3
"""
apply_bmg.py - Swap the BMG (text) section of a Pulsar config.pul without
touching anything else in the file.

Background
----------
A Pulsar config.pul is laid out as:

    [Header: 36 bytes]
      magic          u32   "PULS"
      version        i32
      offsetToInfo   i32
      offsetToCups   i32
      offsetToBMG    i32    <- position of the BMG blob, never changes
      modFolderName  char[16]
    [Info section]
    [Cups + Tracks section]
    [BMG blob]     -- self-describing: 8-byte magic "MESGbmg1" followed
                       by a big-endian u32 giving its own total size
    [FILE text blob]  -- no length field of its own, just runs to EOF

Because the BMG blob declares its own size, and the FILE blob after it has
no length field of its own, replacing the BMG blob with a different one
(any size, bigger or smaller) never requires touching the header or
recomputing any other offset. We just:

    1. Find where the old BMG blob starts  (header.offsetToBMG)
    2. Find where it ends                  (its own embedded size field)
    3. Encode the new bmg.txt to binary with wbmgt
    4. Splice: [everything before old BMG] + [new BMG] + [old FILE blob]

Requirements
------------
Wiimm's wbmgt (part of Wiimm's SZS Tools) is used to convert the plain-text
bmg.txt into the binary BMG format. By default this script looks for it at
    ./tools/wbmgt.exe
(override with --wbmgt if yours lives elsewhere).

Usage
-----
    python apply_bmg.py --config Config.pul --bmg new_bmg.txt --output Config_new.pul
"""

import argparse
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

BMG_MAGIC = b"MESGbmg1"
PUL_MAGIC = b"PULS"


def read_config_header(data: bytes):
    magic = data[0:4]
    if magic != PUL_MAGIC:
        sys.exit(f"Not a Pulsar config.pul (bad magic: {magic!r})")
    version = struct.unpack(">i", data[4:8])[0]
    offset_to_bmg = struct.unpack(">i", data[16:20])[0]
    return version, offset_to_bmg


def read_bmg_blob_size(data: bytes, offset_to_bmg: int) -> int:
    magic = data[offset_to_bmg: offset_to_bmg + 8]
    if magic != BMG_MAGIC:
        sys.exit(f"No BMG blob found at offset {offset_to_bmg:#x} (found {magic!r} instead of {BMG_MAGIC!r})")
    size = struct.unpack(">i", data[offset_to_bmg + 8: offset_to_bmg + 12])[0]
    return size


def encode_bmg(wbmgt_path: Path, bmg_txt: Path, dest_bin: Path) -> bytes:
    cmd = [str(wbmgt_path), "encode", str(bmg_txt), "--dest", str(dest_bin), "-o"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0 or not dest_bin.exists():
        sys.exit(
            "wbmgt failed to encode the BMG text.\n"
            f"Command: {' '.join(cmd)}\n"
            f"stdout: {result.stdout}\n"
            f"stderr: {result.stderr}\n\n"
            "If this flag layout doesn't match your wbmgt build, run "
            "'wbmgt encode --help' to check the correct arguments for "
            "--dest / overwrite."
        )
    return dest_bin.read_bytes()


def main():
    parser = argparse.ArgumentParser(description="Swap the BMG section of a Pulsar config.pul")
    parser.add_argument("--config", required=True, type=Path, help="Existing config.pul to modify")
    parser.add_argument("--bmg", required=True, type=Path, help="New bmg.txt to apply (wbmgt text format)")
    parser.add_argument("--output", required=True, type=Path, help="Path to write the new config.pul")
    parser.add_argument(
        "--wbmgt",
        default=Path('..\\tools\\wiims\\bin\\wbmgt.exe'),
        type=Path,
        help="Path to the wbmgt executable (default: tools/wbmgt.exe)",
    )
    args = parser.parse_args()

    if not args.config.exists():
        sys.exit(f"config.pul not found: {args.config}")
    if not args.bmg.exists():
        sys.exit(f"bmg.txt not found: {args.bmg}")
    if not args.wbmgt.exists():
        sys.exit(f"wbmgt not found: {args.wbmgt} (pass --wbmgt to point at it)")

    data = args.config.read_bytes()
    version, offset_to_bmg = read_config_header(data)
    old_bmg_size = read_bmg_blob_size(data, offset_to_bmg)

    print(f"config version:      {version}")
    print(f"old BMG blob:        offset {offset_to_bmg:#x}, size {old_bmg_size} bytes")

    with tempfile.TemporaryDirectory() as tmp:
        new_bmg_path = Path(tmp) / "new_bmg.bin"
        new_bmg_bytes = encode_bmg(args.wbmgt, args.bmg, new_bmg_path)

    if new_bmg_bytes[0:8] != BMG_MAGIC:
        sys.exit("wbmgt produced output that doesn't look like a valid BMG file (bad magic).")

    print(f"new BMG blob:        size {len(new_bmg_bytes)} bytes")

    head = data[:offset_to_bmg]                        # header + info + cups/tracks, untouched
    tail = data[offset_to_bmg + old_bmg_size:]          # FILE section, untouched

    new_data = head + new_bmg_bytes + tail
    args.output.write_bytes(new_data)

    delta = len(new_bmg_bytes) - old_bmg_size
    print(f"\nWrote {args.output}  ({len(new_data)} bytes total, BMG size changed by {delta:+d} bytes)")
    print("Header offsets and FILE section left untouched — nothing else needed to change.")


if __name__ == "__main__":
    main()
