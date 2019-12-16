#include "consts.s"

#ifdef ARM7_DLDI
.section .text
#else
.section .vram
#endif


@---------------------------------------------------------------------------------
	.align	4
	.arm
#ifndef ARM7_DLDI
	.global _io_dldi
#endif
@---------------------------------------------------------------------------------
.equ FEATURE_MEDIUM_CANREAD,		0x00000001
.equ FEATURE_MEDIUM_CANWRITE,		0x00000002
.equ FEATURE_SLOT_GBA,				0x00000010
.equ FEATURE_SLOT_NDS,				0x00000020

.equ FIX_ALL,						0x01
.equ FIX_GLUE,						0x02
.equ FIX_GOT,						0x04
.equ FIX_BSS,						0x08

.global _dldi_start
_dldi_start:

@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
	.word	0xBF8DA5ED		@ Magic number to identify this region
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	0x0E	@16KiB	@ Log [base-2] of the size of this driver in bytes.
	.byte	0x00			@ Sections to fix
	.byte 	0x0E	@16KiB	@ Log [base-2] of the allocated space in bytes.
	
@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 16 bytes
	.align	4
	.asciz "Default (No interface)"

@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
#ifdef ARM7_DLDI
	.word   0x0380A800 //_dldi_start		@ data start
	.word   0x0380E800 //_dldi_end		@ data end
#else
	.word   _dldi_start		@ data start
	.word   _dldi_end		@ data end
#endif
	.word	0x00000000		@ Interworking glue start	-- Needs address fixing
	.word	0x00000000		@ Interworking glue end
	.word   0x00000000		@ GOT start					-- Needs address fixing
	.word   0x00000000		@ GOT end
	.word   0x00000000		@ bss start					-- Needs setting to zero
	.word   0x00000000		@ bss end

@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
_io_dldi:
	.ascii	"DLDI"					@ ioType
	.word	0x00000000				@ Features
	.word	_DLDI_startup			@ 
	.word	_DLDI_isInserted		@ 
	.word	_DLDI_readSectors		@   Function pointers to standard device driver functions
	.word	_DLDI_writeSectors		@ 
	.word	_DLDI_clearStatus		@ 
	.word	_DLDI_shutdown			@ 
	
@---------------------------------------------------------------------------------

_DLDI_startup:
_DLDI_isInserted:
_DLDI_readSectors:
_DLDI_writeSectors:
_DLDI_clearStatus:
_DLDI_shutdown:
	mov		r0, #0x00				@ Return false for every function
	bx		lr



@---------------------------------------------------------------------------------
	.align
	.pool
dldi_data_end:
.space 16384 - (dldi_data_end - _dldi_start)		@ Fill to 16KiB

_dldi_end:
	.end
@---------------------------------------------------------------------------------