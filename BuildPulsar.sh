#!/bin/bash
set -e
shopt -s nullglob

# Use wine to call the Windows compiler and linker
WINE="wine"

# Compiler and tools
CC="./compiler/mwcceppc.exe"
LINKER="./KamekLinker/Kamek.exe"

# Project directories
ENGINE="./KamekInclude"
GAMESOURCE="./GameSource"
PULSAR="./PulsarEngine"
BUILD="./build"

# Create build directory if missing
mkdir -p "$BUILD"

# Compiler flags (note: -I- disables standard includes)
CFLAGS=(
  -I- 
  -i "$ENGINE"
  -i "$GAMESOURCE"
  -i "$PULSAR"
  -opt all 
  -inline auto 
  -enum int 
  -proc gekko 
  -fp hard 
  -sdata 0 
  -sdata2 0 
  -maxerrors 1 
  -func_align 4
)

# Clean previous .o files
# rm -f "$BUILD"/*.o

# Compile kamek.cpp
WINEDEBUG=-all wine  "$CC" "${CFLAGS[@]}" -c -o "$BUILD/kamek.o" "$ENGINE/kamek.cpp"

# Compile all .cpp files in PULSAR
OBJECTS=()
for file in $(find "$PULSAR" -type f -name "*.cpp"); do
  base=$(basename "$file" .cpp)
  echo "Compiling $file"
  obj="$BUILD/$base.o"
  if [ ! -f "$obj" ] || [ "$file" -nt "$obj" ]; then
    echo "Compiling $file"
    WINEDEBUG=-all wine "$CC" "${CFLAGS[@]}" -c -o "$obj" "$file"
  else
    echo "Skipping $file (up to date)"
  fi
  OBJECTS+=("$BUILD/$base.o")
done

# Link all object files
echo "Linking..."
WINEDEBUG=-all wine "$LINKER" "$BUILD/kamek.o" "${OBJECTS[@]}" \
  -dynamic \
  -externals="$GAMESOURCE/symbols.txt" \
  -versions="$GAMESOURCE/versions.txt" \
  -output-combined="$BUILD/Code.pul"
