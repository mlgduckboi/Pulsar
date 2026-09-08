#!/usr/bin/env python3
"""
patch_szs.py

For every "somefile.szs" in ./input, looks for a matching folder in
./patches (e.g. ./patches/somefile.szs/), extracts the .szs archive
using wszst, overlays/copies the contents of the patch folder into the
extracted archive folder, then repacks it (also with wszst) as
"somefile.szs" into ./output.

Requirements:
    - Windows
    - Wiimms SZS Tools installed and "wszst" available on PATH
      (or set WSZST_PATH below to the full path of wszst.exe)

Folder layout expected:
    ./input/course.szs
    ./patches/course.szs/<files to overlay, matching internal szs structure>
    ./output/course.szs   <- produced by this script

Usage:
    python patch_szs.py
"""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# If wszst is not on PATH, set the full path here, e.g.:
# WSZST_PATH = r"C:\Program Files (x86)\Wiimm\SZS\wszst.exe"
WSZST_PATH = "wszst"

BASE_DIR = Path(__file__).resolve().parent
INPUT_DIR = BASE_DIR / "input"
PATCHES_DIR = BASE_DIR / "patches"
OUTPUT_DIR = BASE_DIR / "output"


def run_wszst(args, description):
    """Run a wszst command, raising an exception with useful output on failure."""
    cmd = [WSZST_PATH] + args
    print(f"  -> {description}: {' '.join(str(a) for a in cmd)}")
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"wszst command failed ({description}):\n"
            f"  Command: {' '.join(str(a) for a in cmd)}\n"
            f"  Exit code: {result.returncode}\n"
            f"  Stdout: {result.stdout}\n"
            f"  Stderr: {result.stderr}"
        )
    return result


def copy_tree_overlay(src: Path, dst: Path):
    """Copy every file from src into dst, overwriting existing files and
    preserving the relative directory structure. Creates any needed
    subdirectories in dst."""
    for src_path in src.rglob("*"):
        rel_path = src_path.relative_to(src)
        dst_path = dst / rel_path
        if src_path.is_dir():
            dst_path.mkdir(parents=True, exist_ok=True)
        else:
            dst_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src_path, dst_path)


def check_wszst_available():
    try:
        subprocess.run(
            [WSZST_PATH, "--version"],
            capture_output=True,
            text=True,
            check=True,
        )
    except FileNotFoundError:
        print(
            f"ERROR: Could not find '{WSZST_PATH}'. Make sure Wiimms SZS "
            f"Tools are installed and wszst is on PATH, or set WSZST_PATH "
            f"in this script to the full path of wszst.exe.",
            file=sys.stderr,
        )
        sys.exit(1)
    except subprocess.CalledProcessError:
        # Some wszst builds may return non-zero on --version but still be
        # usable; don't hard-fail here, just warn.
        pass


def process_szs(szs_path: Path, patch_dir: Path, output_path: Path):
    """Extract szs_path, overlay patch_dir contents, repack to output_path."""
    with tempfile.TemporaryDirectory(prefix="szs_patch_") as tmp_str:
        tmp_dir = Path(tmp_str)
        extract_dir = tmp_dir / "extracted"

        # 1. Extract the szs archive
        run_wszst(
            [
                "extract",
                str(szs_path),
                "--DEST",
                str(extract_dir),
                "--overwrite",
            ],
            f"extracting {szs_path.name}",
        )

        # 2. Overlay the patch files on top of the extracted archive
        print(f"  -> copying patches from {patch_dir} into {extract_dir}")
        copy_tree_overlay(patch_dir, extract_dir)

        # 3. Repack into the output directory
        output_path.parent.mkdir(parents=True, exist_ok=True)
        run_wszst(
            [
                "create",
                str(extract_dir),
                "--dest",
                str(output_path),
                "--overwrite",
            ],
            f"repacking to {output_path.name}",
        )


def main():
    check_wszst_available()

    if not INPUT_DIR.is_dir():
        print(f"ERROR: input directory not found: {INPUT_DIR}", file=sys.stderr)
        sys.exit(1)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    szs_files = sorted(INPUT_DIR.glob("*.szs"))
    if not szs_files:
        print(f"No .szs files found in {INPUT_DIR}")
        return

    processed = 0
    skipped = 0
    failed = 0

    for szs_path in szs_files:
        name = szs_path.name  # e.g. "somefile.szs"
        patch_dir = PATCHES_DIR / name
        output_path = OUTPUT_DIR / name

        print(f"\n[{name}]")

        if not patch_dir.is_dir():
            print(f"  -> No matching patch folder found at {patch_dir}, skipping.")
            skipped += 1
            continue

        try:
            process_szs(szs_path, patch_dir, output_path)
            print(f"  -> Done: {output_path}")
            processed += 1
        except Exception as e:
            print(f"  -> FAILED: {e}", file=sys.stderr)
            failed += 1

    print(
        f"\nSummary: {processed} patched, {skipped} skipped (no patch folder), "
        f"{failed} failed."
    )
    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
