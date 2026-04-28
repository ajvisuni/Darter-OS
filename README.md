# Darter OS
A custom x86 32-bit " Sand Boxed " Operating System.

## Features
- Custom VGA terminal with "Sci-Fi" styling. 
- Kernel telemetry via `sysinfo` command.
- Interactive shell with command recognition. 

## How to Build and Run
Run these commands in order to create the kernel binary:

1. **Assemble:** `nasm -f elf32 boot.asm -o boot.o` 
2. **Compile:** `i686-linux-gnu-gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra` 
3. **Link:** `ld -m elf_i386 -T linker.ld -o darter.bin boot.o kernel.o` 
4. **Run:** `qemu-system-i386 -kernel darter.bin`

Once again please keep your expectations low and you will never be Disapointed. 

YOURS AJ