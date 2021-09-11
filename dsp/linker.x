OUTPUT_FORMAT("elf32-teak", "elf32-teak", "elf32-teak")
OUTPUT_ARCH(teak)

ENTRY(_start)
SECTIONS
{
    . = 0x0;

    .text       : ALIGN(4) { *(.text.start) *(.text*); . = ALIGN(4); }

    . = 0x10000000;

    /* stack from 0x0000-0x0500 */
    . += 0x500 * 2;

    .rodata      : AT(SIZEOF(.text)) ALIGN(4) { *(.rodata*); . = ALIGN(4); }
    .data       : AT(SIZEOF(.text) + SIZEOF(.rodata)) ALIGN(4) { *(.data.ipc) *(.data.sndregs) *(.data*); . = ALIGN(4); }
    .bss        : AT(SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data)) ALIGN(8) { __bss_start = .; *(.bss* COMMON); . = ALIGN(8); __bss_end = .; }

    . = ALIGN(4);
}