#!/usr/bin/env python3
"""
build_pack.py

Automates the full pack build pipeline:
  1. Copy ./packTemplate -> ./packBuild (recursive)
  2. Run SettingsBuilder scripts (id_codec.py, apply_bmg.py, gen_settings_param.py)
  3. Run ./BuildPulsar.bat
  4. Copy build outputs into packBuild / PulsarEngine
  5. Run SZSPatcher/patch_szs.py
  6. Copy SZSPatcher output into packBuild

Run this script from the project root (the directory that contains
packTemplate, SettingsBuilder, SZSPatcher, BuildPulsar.bat, etc.).
"""

import shutil
import subprocess
import sys
from pathlib import Path

# Root = directory this script lives in. Change to ROOT.parent-relative logic
# if you want to invoke it from elsewhere; as written it assumes CWD-independent
# behavior by anchoring everything to the script's own location.
ROOT = Path(__file__).resolve().parent


def run(cmd, cwd):
    """Run a command, streaming output, and abort the build on failure."""
    print(f"\n>>> Running {cmd} in {cwd}")
    result = subprocess.run(cmd, cwd=str(cwd), shell=False)
    if result.returncode != 0:
        print(f"!!! Command failed ({result.returncode}): {cmd}")
        sys.exit(result.returncode)


def copy_tree_merge(src: Path, dst: Path):
    """Recursively copy src into dst, overwriting existing files."""
    if not src.exists():
        print(f"!!! Source path does not exist: {src}")
        sys.exit(1)
    dst.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src, dst, dirs_exist_ok=True)


def copy_file(src: Path, dst_dir: Path):
    if not src.exists():
        print(f"!!! Source file does not exist: {src}")
        sys.exit(1)
    dst_dir.mkdir(parents=True, exist_ok=True)
    dst_path = dst_dir / src.name
    shutil.copy2(src, dst_path)
    print(f"Copied {src} -> {dst_path}")


def main():
    py = sys.executable  # use the same Python interpreter running this script

    pack_template = ROOT / "packTemplate"
    pack_build = ROOT / "packBuild"
    settings_builder = ROOT / "SettingsBuilder"
    szs_patcher = ROOT / "SZSBuilder"

    # 1. Copy packTemplate -> packBuild recursively
    print(f"\n=== Copying {pack_template} -> {pack_build} ===")
    copy_tree_merge(pack_template, pack_build)

    # 2. Go into SettingsBuilder and run the scripts in order
    print("\n=== Running SettingsBuilder scripts ===")
    run([py, "id_codec.py"], cwd=settings_builder)
    run(
        [
            py,
            "apply_bmg.py",
            "--config", "config.pul",
            "--bmg", "output_ids.txt",
            "--output", "Config.pul",
        ],
        cwd=settings_builder,
    )
    run([py, "gen_settings_param.py"], cwd=settings_builder)

    # 3. Back to parent (ROOT) and run BuildPulsar.bat
    print("\n=== Running BuildPulsar.bat ===")
    run(["BuildPulsar.bat"], cwd=ROOT)

    # 4. Copy build outputs
    print("\n=== Copying build outputs ===")
    copy_file(ROOT / "build" / "Code.pul", pack_build / "sigma" / "Binaries")
    copy_file(settings_builder / "Config.pul", pack_build / "sigma" / "Binaries")
    copy_file(
        settings_builder / "SettingsParam.cpp",
        ROOT / "PulsarEngine" / "Settings",
    )

    # 5. Go into SZSPatcher and run patch_szs.py
    print("\n=== Running SZSPatcher ===")
    run([py, "patch_szs.py"], cwd=szs_patcher)

    # 6. Back to parent, copy SZSPatcher/output into packBuild/sigma/My Stuff/
    print("\n=== Copying SZSPatcher output ===")
    szs_output = szs_patcher / "output"
    my_stuff = pack_build / "sigma" / "My Stuff"
    copy_tree_merge(szs_output, my_stuff)

    print("\n=== Build complete! ===")


if __name__ == "__main__":
    main()
