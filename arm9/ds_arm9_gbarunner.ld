OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

__vectors_start	=	ORIGIN(vectors);
__itcm_start	=	ORIGIN(itcm);
__vram_start	=	ORIGIN(vram);
__ewram_end	=	ORIGIN(ewram) + LENGTH(ewram);
__eheap_end	=	ORIGIN(ewram) + LENGTH(ewram);
__dtcm_start	=	ORIGIN(dtcm);
__dtcm_top	=	ORIGIN(dtcm) + LENGTH(dtcm);
__irq_flags	=	__dtcm_top - 0x08;
__irq_vector	=	__dtcm_top - 0x04;

__sp_svc	=	__dtcm_top - 0x100;
__sp_irq	=	__sp_svc - 0x100;
__sp_usr	=	__sp_irq - 0x100;

SECTIONS
{
	.init	:
	{
		__text_start = . ;
__secure_area__ = .;
	. = . + 0x800;
		KEEP (*(.init))
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.plt : { *(.plt) } >ewram = 0xff

	.text :   /* ALIGN (4): */
	{
		*(EXCLUDE_FILE (*.itcm*) .text)

		*(.text.*)
		*(.stub)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.fini           :
	{
		KEEP (*(.fini))
	} >ewram =0xff

	__text_end = . ;

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ewram
 	__exidx_start = .;
	ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >ewram
 	__exidx_end = .;

	/*	Ensure the __preinit_array_start label is properly aligned.  We
		could instead move the label definition inside the section, but
		the linker would then create the section even if it turns out to
		be empty, which isn't pretty.  */

	. = ALIGN(32 / 8);

	PROVIDE (__preinit_array_start = .);
	.preinit_array     : { KEEP (*(.preinit_array)) } >ewram = 0xff
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array     :
	{
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
	} >ewram = 0xff
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     :
	{
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
	} >ewram = 0xff

	PROVIDE (__fini_array_end = .);

	.ctors :
	{
	/*	gcc uses crtbegin.o to find the start of the constructors, so
		we make sure it is first.  Because this is a wildcard, it
		doesn't matter if the user does not actually link against
		crtbegin.o; the linker won't look for a file to match a
		wildcard.  The wildcard also means that it doesn't matter which
		directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff
	.jcr            : { KEEP (*(.jcr)) } >ewram = 0
	.got            : { *(.got.plt) *(.got) *(.rel.got) } >ewram = 0

	.ewram ALIGN(4) : 
	{
		__ewram_start = ABSOLUTE(.) ;
		*(.ewram)
		*ewram.*(.text)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff


	.data ALIGN(4) :
	{
		__data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >ewram = 0xff


	__dtcm_lma = . ;

	.dtcm __dtcm_start : AT (__dtcm_lma)
	{
		*(.dtcm)
		*(.dtcm.*)
		. = ALIGN(4);
		__dtcm_end = ABSOLUTE(.);
	} >dtcm  = 0xff


	__itcm_lma = __dtcm_lma + SIZEOF(.dtcm);

	.itcm __itcm_start : AT (__itcm_lma)
	{
		*(.itcm)
		*itcm.*(.text)
		. = ALIGN(4);
		__itcm_end = ABSOLUTE(.);
	} >itcm = 0xff
	
	__vectors_lma = __itcm_lma + SIZEOF(.itcm);

	.vectors __vectors_start : AT (__vectors_lma)
	{
		*(.vectors)
		*vectors.*(.text)
		. = ALIGN(4);
		__vectors_end = ABSOLUTE(.);
	} >vectors = 0xff

	__vram_lma = __vectors_lma + SIZEOF(.vectors);

	.vram __vram_start : AT (__vram_lma)
	{
		*(.vram)
		*vram.*(.text)
		. = ALIGN(32);
		__vram_end = ABSOLUTE(.);
	} >vram = 0xff

	__bss_vma = __vram_lma + (__vram_end - __vram_start);
	
	.sbss __dtcm_end (NOLOAD): 
	{
		__sbss_start = ABSOLUTE(.);
		__sbss_start__ = ABSOLUTE(.);
		*(.sbss)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__sbss_end = ABSOLUTE(.);
	} >dtcm 

	.bss __bss_vma (NOLOAD): 
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__bss_end__ = ABSOLUTE(.) ;
		__end__ = ABSOLUTE(.) ;
	} AT>ewram 



	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000 : { _stack = .; *(.stack) }
	/* These must appear regardless of  .  */
}
