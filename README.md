

# Simple x86 Kernel with Keyboard Input

This is a basic hobbyist kernel written in C and Assembly, designed to run on 32-bit x86 systems. It includes keyboard input handling, interrupt descriptor table setup, screen output using VGA text mode, and a simple stack for carriage returns. This kernel is bootable with GRUB and tested using QEMU and VirtualBox.

## Features

- Multiboot-compliant bootloader header
- Custom interrupt descriptor table (IDT)
- Keyboard input handling (with Shift and Caps Lock support)
- Backspace and carriage return handling
- VGA text-mode output with cursor tracking
- Basic kernel stack implementation
- Written in low-level C and x86 Assembly

## Requirements

- `nasm` (for assembling kernel entry)
- `gcc` with cross-compiler support for `i386-elf` target
- `ld` (GNU linker)
- `qemu` or `VirtualBox` for testing
- `grub-mkrescue` to create bootable ISO

## Building the Kernel

```bash
make
```

This will produce an `os.iso` file that can be booted in QEMU or VirtualBox.

## Running with QEMU

```bash
qemu-system-i386 -cdrom os.iso
```

If you're getting dropped into GRUB command line, ensure:
- Your `grub.cfg` is correctly named and located in `iso/boot/grub/`
- You have `multiboot` specified in your GRUB config

## Running with VirtualBox

1. Create a new VM (Type: `Other`, Version: `Other/Unknown (32-bit)`)
2. Mount the `os.iso` as a virtual CD
3. Boot the VM

## Notes

- `keyboard_handler_main()` processes raw scan codes from the PS/2 keyboard.
- Caret (cursor) position is updated via port-mapped I/O using ports `0x3D4` and `0x3D5`.
- Keyboard handling supports Shift, Caps Lock, Backspace, and Enter keys.
- The kernel sets up its own IDT and directly interacts with PIC (Programmable Interrupt Controller).

##  Known Issues

- Some physical/virtual machines may crash on boot if the I/O Privilege Level (IOPL) or PIC is not configured correctly.
- Make sure your `keyboard_handler` address is properly loaded into the IDT and aligned correctly.
- Ensure interrupts are unmasked (especially IRQ1) or keyboard input will not be received.

##  References

- [OSDev Wiki](https://wiki.osdev.org)
- Intel x86 architecture manuals
- GRUB Multiboot Specification

---

This is a learning project. Contributions, suggestions, and pull requests are welcome!

```
