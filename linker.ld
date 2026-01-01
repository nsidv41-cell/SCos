/* ============================================================================
 * SCos 1.3.5 - Linker Script
 * ============================================================================ */

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(kernel_main)

SECTIONS
{
    /* Kernel loaded at 1MB (0x100000) but we load to 0x10000 initially */
    . = 0x10000;

    .text : ALIGN(4K)
    {
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* End of kernel */
    _kernel_end = .;
}
