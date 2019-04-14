//#define DEBUG_ENABLED

#include "consts.s"

.global __aeabi_unwind_cpp_pr0
.equ __aeabi_unwind_cpp_pr0, 0
.global __aeabi_unwind_cpp_pr1
.equ __aeabi_unwind_cpp_pr1, 0

.global gba_setup
gba_setup:
	//Make sure interupts are disabled!
	ldr r0,= 0x4000210
	mov r1, #0
	str r1, [r0, #-8]
	str r1, [r0]
	mov r1, #0xFFFFFFFF
	str r1, [r0, #4]

	ldr r1,= 0x8203
	mov	r0, #0x04000000
	add	r0, r0, #0x304
	strh r1, [r0]

	ldr r0,= 0x78//0x50078
	mcr p15, 0, r0, c1, c0, 0

	//setup dtcm
	ldr r0,= (address_dtcm + 0xA)
	mcr p15, 0, r0, c9, c1, 0

	//setup itcm
	mov	r0,#0x20
	mcr	p15, 0, r0, c9, c1,1

	//enable tcm
	ldr r0,= 0x50078
	mcr p15, 0, r0, c1, c0, 0

	mov sp, #(16 * 1024)
	bl initSystem

	//copy the itcm in place
	ldr r0,= __itcm_lma
	ldr r1,= (32 * 1024)
	ldr r2,= __itcm_start
itcm_setup_copyloop:
	ldmia r0!, {r3-r10}
	stmia r2!, {r3-r10}
	subs r1, #0x20
	bne itcm_setup_copyloop

	ldr r0,= fiq_hook
	sub r0, #0x1C	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0x1C
	str r1, [r0]
	
	//map the gba cartridge to the arm7 and nds card too
	ldr r0,= 0x4000204
	ldrh r1, [r0]
#ifdef ARM7_DLDI
	orr r1, #0x80
	orr r1, #0x800 //card to arm7
#else
	bic r1, #0x80
	bic r1, #0x800 //card to arm9
#endif
	bic r1, #0x8000	//set memory priority to arm9
	strh r1, [r0]

	//abcd to arm9
	ldr r0,= 0x4000240
	mov r1, #0x80
	strb r1, [r0]
	ldr r0,= 0x4000241
	mov r1, #0x80
	strb r1, [r0]
	//map vram block c to 0x06000000 on the arm7
	ldr r0,= 0x4000242
	mov r1, #0x80
	strb r1, [r0]
	//map vram block d to 0x06020000 on the arm7
	ldr r0,= 0x4000243
	mov r1, #0x80
	strb r1, [r0]

	//setup display capture
	ldr r0,= 0x320000
	ldr r1,= 0x4000064
	str r0, [r1]

	//copy the vram code to a
	ldr r0,= __vram_lma
	ldr r1,= (128 * 1024)
	ldr r2,= __vram_start
vram_setup_copyloop:
	ldmia r0!, {r3-r10}
	stmia r2!, {r3-r10}
	subs r1, #0x20
	bne vram_setup_copyloop

#ifdef ISNITRODEBUG
	//this enables debug printing on IS-NITRO-EMULATOR
	//this is not completely correct code, it should actually read the
	//gba mapping address from 0x027FFF7C or something
	ldr r0,= 0x09F80000
	ldr r1,= 0x202
	strh r1, [r0, #0xFE] //enable debug print
#endif

	//dim the screen a bit for gba colors:
	//ldr r0,= 0x0400006C
	//ldr r1,= 0x8008
	//str r1, [r0]

	//setup protection regions
	//region 0	bg region		0x00000000-0xFFFFFFFF	2 << 31		r/w/x
	//region 1	io region		0x04000000-0x04FFFFFF	2 << 23		-/-/-
	//region 2 card				0x08000000-0x0FFFFFFF	2 << 26		-/-/-
	//- region 2	card region	1	0x08000000-0x0BFFFFFF	2 << 25		-/-/-
	//- region 3	card region 2	0x0C000000-0x0DFFFFFF	2 << 24		-/-/-
	//- region 4	save region		0x0E000000-0x0E00FFFF	2 << 15		-/-/-
	//region 3	oam vram region	0x06010000-0x06017FFF	2 << 14		-/-/-
	//region 4  bg vram relative to fixed oam	0x063F0000 - 0x063FFFFF	2 << 15	-/-/-
	//region 5 real card region for i-cache	0x02000000-0x023FFFFF	r/w/x
	//-region 6 exclusion region for i-cache	0x02000000-0x0203FFFF	r/w/x

	//region 0	bg region		0x00000000-0xFFFFFFFF	2 << 31		r/w/x
	//ldr r0,= (1 | (31 << 1) | 0)
	//mcr p15, 0, r0, c6, c0, 0
	ldr r0,= (1 | (26 << 1) | 0)
	mcr p15, 0, r0, c6, c0, 0

	//region 1	io region		0x04000000-0x04FFFFFF	2 << 23		-/-/-
	ldr r0,= (1 | (23 << 1) | 0x04000000)
	mcr p15, 0, r0, c6, c1, 0

	//region 2 card				0x08000000-0x0FFFFFFF	2 << 26		-/-/-
	//ldr r0,= (1 | (26 << 1) | 0x08000000)
	//mcr p15, 0, r0, c6, c2, 0
	//mov r0, #0
	//mcr p15, 0, r0, c6, c2, 0
	//vram; write protected, cause byte writes are not possible :/
	ldr r0,= (1 | (16 << 1) | 0x06000000)
	mcr p15, 0, r0, c6, c2, 0

	//region 3	oam vram region	0x06010000-0x06017FFF	2 << 14		-/-/-
	ldr r0,= (1 | (14 << 1) | 0x06010000)
	mcr p15, 0, r0, c6, c3, 0

	//bios protection + itcm
	//ldr r0,= (1 | (13 << 1) | 0x00000000)
	//ldr r0,= (1 | (23 << 1) | 0x00000000)
	//mcr p15, 0, r0, c6, c4, 0
	//ldr r0,= (1 | (24 << 1) | 0x00000000)
	//this does not protect the whole itcm, because we don't want reading and writing to bios to work,
	//but itcm should be able to read itself!
	//ldr r0,= (1 | (23 << 1) | 0x00000000)
	//mcr p15, 0, r0, c6, c4, 0
	ldr r0,= (1 | (24 << 1) | 0x00000000)
	mcr p15, 0, r0, c6, c4, 0

	//main memory
	ldr r0,= (1 | (21 << 1) | 0x02000000)
	mcr p15, 0, r0, c6, c5, 0

	//ldr r0,= (1 | (14 << 1) | 0x03000000)
	//mcr p15, 0, r0, c6, c6, 0
	ldr r0,= (1 | (18 << 1) | 0x06800000)
	mcr p15, 0, r0, c6, c6, 0

	//ewram
	ldr r0,= (1 | (17 << 1) | 0x02000000)
	mcr p15, 0, r0, c6, c7, 0

	//TODO: cartridge area protection and d-cache

	//mov r0, #0
	//mcr p15, 0, r0, c6, c5, 0
	//mcr p15, 0, r0, c6, c6, 0
	//mcr p15, 0, r0, c6, c7, 0

	ldr r0,= pu_data_permissions
	//mov r0, #3
	//orr r0, r0, #(0x6 << (4 * 4))
	//orr r0, r0, #(0x36 << (4 * 5))
	//orr r0, r0, #(0x3 << (4 * 7))
	mcr p15, 0, r0, c5, c0, 2
	ldr r0,= 0x33660303
	mcr p15, 0, r0, c5, c0, 3

	//only instruction and data cache for (fake) cartridge
	mov r0, #(1 << 5)
	orr r0, #(1 << 6)
	mcr p15, 0, r0, c2, c0, 0	//data cache
	//orr r0, #(1 << 6)
	//orr r1, #(1 << 6)
	//orr r1, #(1 << 7)
	mov r0, #((1 << 5) | (1 << 6))


#if defined(ENABLE_WRAM_ICACHE) && !defined(POSTPONED_ICACHE)
	orr r0, #(1 << 0)
	orr r0, #(1 << 7)
#endif
	mcr p15, 0, r0, c2, c0, 1	//instruction cache

	//write buffer
	mov r0, #((1 << 5) | (1 << 6))
	mcr p15, 0, r0, c3, c0, 0

	//Copy GBA Bios in place
//	ldr r0,= bios_bin
//	mov r1, #0x4000
//	mov r2, #0
//gba_setup_copyloop:
//	ldmia r0!, {r3-r10}
//	stmia r2!, {r3-r10}
//	subs r1, #0x20
//	bne gba_setup_copyloop

	//Setup debugging on the bottom screen
	//Set vram block H for sub bg
	ldr r0,= 0x04000248
	mov r1, #0x81
	strb r1, [r0]
	//decompress debugFont to 0x06200000
	//ldr r0,= debugFont
	//ldr r1,= 0x06200000
	//ldr r2,=0x1194
	//blx r2
	//svc 0x120000

	//Copy debug font
	ldr r0,= debugFont
	mov r1, #0x4000
	ldr r2,= 0x06200000
font_setup_copyloop:
	ldmia r0!, {r3-r10}
	stmia r2!, {r3-r10}
	subs r1, #0x20
	bne font_setup_copyloop

	ldr r0,= 0x04001000
	ldr r1,= 0x10801
	str r1, [r0]

	ldr r0,= 0x0400100E
	ldr r1,= 0x4400
	strh r1, [r0]

	ldr r0,= 0x04001030
	ldr r1,= 0x100
	strh r1, [r0]
	strh r1, [r0, #6]

	ldr r1,= 0x00
	strh r1, [r0, #2]
	strh r1, [r0, #4]

	str r1, [r0, #0xC]

	mov r0, #0
	ldr r1,= 0x06202000
	mov r2, #1024
gba_setup_fill_sub_loop:
	str r0, [r1], #4
	subs r2, #4
	bne gba_setup_fill_sub_loop

	ldr r0,= 0x06202000
	ldr r1,= 0x54534554
	str r1, [r0]

	ldr r0,= 0x05000400
	ldr r1,= 0x7FFF
	strh r1, [r0], #2
	ldr r1,= 0x0000
	strh r1, [r0]

	ldr r0,= __dtcm2_lma
	ldr r1,= (16 * 1024)
	ldr r2,= __dtcm2_start
dtcm_setup_copyloop:
	ldmia r0!, {r3-r10}
	stmia r2!, {r3-r10}
	subs r1, #0x20
	bne dtcm_setup_copyloop

	ldr r0,= address_write_table_32bit_dtcm_setup
	blx r0
	ldr r0,= address_write_table_16bit_dtcm_setup
	blx r0
	ldr r0,= address_write_table_8bit_dtcm_setup
	blx r0
	ldr r0,= address_read_table_32bit_dtcm_setup
	blx r0
	ldr r0,= address_read_table_16bit_dtcm_setup
	blx r0
	ldr r0,= address_read_table_8bit_dtcm_setup
	blx r0
	//ldr r0,= thumb_table_dtcm_setup
	//blx r0
	
	//wait for the arm7 to copy the dldi data
	//enable the arm7-arm9 fifo
	ldr r0,= 0x04000180
	ldr r1,= (0x8000 | (1 << 3))
	str r1, [r0, #4]

	//enable arm7 irq
	mov r1, #(1 << 14)
	str r1, [r0]

	ldr r0,= 0x04000210
	mov r1, #(1 << 16)
	str r1, [r0]

	//send setup command to arm7
#ifdef ARM7_DLDI
	ldr r2,= _dldi_start
#endif
	ldr r0,= 0x04000188
	ldr r1,= 0xAA5555AA
	//ldr r3,= bios_tmp
	str r1, [r0]
#ifdef ARM7_DLDI
	str r2, [r0]
#endif
	//str r3, [r0]

	//wait for the arm7 sync command
	ldr r3,= 0x55AAAA55
	ldr r2,= 0x06202000
	ldr r4,= 0x04000184
fifo_loop_1:
	ldr r1, [r4]
	tst r1, #(1 << 8)
	bne fifo_loop_1
	ldr r0,= 0x04100000
	ldr r1, [r0]	//read word from fifo
	cmp r1, r3
	strne r1, [r2]
	bne fifo_loop_1

	ldr sp,= address_dtcm + (16 * 1024)

#ifndef ARM7_DLDI
	//setup dldi
	ldr r0,= (_io_dldi + 8)
	ldr r0, [r0]
	blx r0
	cmp r0, #0
		beq .
#endif

	ldr r0,= _dldi_start + 16
	ldr r1,= 0x06202000
	mov r2, #48
dldi_name_copy:
	ldr r3, [r0], #4
	str r3, [r1], #4
	subs r2, #4
	bne dldi_name_copy

	mov r0, #0x01000000
	bl sd_init

	//more displaycap stuff
	ldr r0,= 0x04001000
	ldr r1,= 0x13823
	str r1, [r0]
	ldr r1,= 0x4084
	strh r1, [r0, #0xE]
	//center
	ldr r1,= -(8 * 256)
	str r1, [r0, #0x38]
	ldr r1,= -(16 * 256)
	str r1, [r0, #0x3C]
	//mask
	ldr r1,= 0x08F8
	strh r1, [r0, #0x40]
	ldr r1,= 0x10B0
	strh r1, [r0, #0x44]
	mov r1, #0x18
	strh r1, [r0, #0x48]
	mov r1, #0x00
	strh r1, [r0, #0x4A]

	ldr r2,= 0x05000400
	strh r1, [r2]

	//setup the oam
	ldr r2,= 0x07000400
	mov r1, #0
1:
	mov r0, #0
2:
	orr r3, r1, #0x0C00 //bitmap obj
	add r3, #16
	strh r3, [r2], #2
	orr r3, r0, #0xC000 //64x64
	add r3, #8
	strh r3, [r2], #2
	mov r3, r1, lsr #3
	mov r3, r3, lsl #5
	add r3, r3, r0, lsr #3
	orr r3, #0xF000 //fully visible
	strh r3, [r2], #4

	add r0, #64
	cmp r0, #256
	blt 2b

	add r1, #64
	cmp r1, #192
	blt 1b

	//disable vram h
	ldr r0,= 0x04000248
	mov r1, #0x00
	strb r1, [r0]

	//disable the 3d geometry and render engine and swap the screens
	ldr r0,= 0x04000304
	ldrh r1, [r0]
	bic r1, #0xC
	eor r1, #0x8000
	strh r1, [r0]

	//turn off the main engine display using masterbright
	ldr r0,= 0x0400006C
	ldr r1,= 0x801F
	strh r1, [r0]

	//Copy GBA Bios in place
//	ldr r0,= bios_tmp
//	mov r1, #0x4000
//	mov r2, #0
//gba_setup_copyloop:
//	ldmia r0!, {r3-r10}
//	stmia r2!, {r3-r10}
//	subs r1, #0x20
//	bne gba_setup_copyloop

	//patch the stack to be in dtcm for speed
	//doesn't seem to work right for some reason
	//ldr r0,= 0x020400F4
	//ldr r1, [r0]
	//ldr r2,= 0x03007E00
	//cmp r1, r2
	//ldreq r1,= 0x10004000
	//streq r1, [r0]

	//in order for this to work, we need to find a way to boot up in retail mode 
	//(so that your flashcard emulates a real ds card and we can use card commands, which is 
	//much faster as direct sd access, because we don't need to deal with the fat filesystem)
	//we tempoarly need a stack for this
	//ldr sp,= 0x10004000
	//bl card_interface_init

	//copy simple gba rom to 02040000
//	ldr r0,= rom_bin //simpleRom
//	ldr r1,= rom_bin_size //simpleRomSize
//	ldr r1, [r1]
//	mov r2, #0x02040000
	//Copy in reverse to prevent overwriting itself
//	add r0, r1
//	add r2, r1
//gba_setup_copyloop_rom2:
//	ldmdb r0!, {r3-r10}
//	stmdb r2!, {r3-r10}
//	subs r1, #0x20
//	bgt gba_setup_copyloop_rom2

	//Make bios jump to 02040000
	ldr r0,= 0xE3A0E781
	ldr r1,= 0xCC
	str r0, [r1]

	//Move wram into place
	ldr r0,= 0x4000247
	mov r1, #0
	strb r1, [r0]

	//move vram into place
	//The gba had 3 ram banks:
	//A - 64kb - always mapped to bg (0x06000000)	-> same on nds as block E
	//B - 16kb - either mapped to bg or obj (bg in modes 3,4,5) (0x06010000) -> same only for bg as block F
	//C - 16kb - always mapped to obj (0x06014000)	-> different on nds as block G

	ldr r0,= 0x04000240
	//mov r1, #0
	//strb r1, [r0, #0] //disable bank a
	//strb r1, [r0, #1] //disable bank b
	////used for debugging on the sub screen, so don't disable it
	//mapped to the arm7, so don't disable it
	//strb r1, [r0, #2] //disable bank c
	//mapped to the arm7, so don't disable it
	//strb r1, [r0, #3] //disable bank d

	mov r1, #0x81
	strb r1, [r0, #4]	//bank E to bg at 0x06000000

	mov r1, #0x82	
	strb r1, [r0, #5]	//bank F to obj at 0x06400000	(would be 0x91 for mapping to bg at 0x06010000)

	mov r1, #0x8A
	strb r1, [r0, #6]	//bank G to obj at 0x06404000


	//Move bg vram into place
	//ldr r0,= 0x04000244
	//mov r1, #0x81
	//strb r1, [r0]

	//ldr r0,= 0x04000245
	//mov r1, #0x91
	//strb r1, [r0]

	//ldr r0,= 0x04000246
	//mov r1, #0x99
	//strb r1, [r0]

	//ldr r0,= 0x04000240
	//mov r1, #0x82
	//strb r1, [r0]

	//map vram h to lcdc for use with eeprom shit
	//ldr r0,= 0x04000248
	//mov r1, #0x80
	//strb r1, [r0]

//	mov r0, #0 //#0xFFFFFFFF
//	ldr r1,= 0x23F0000 //(0x02400000 - (1584 * 2) - (32 * 1024))//0x06898000
//	mov r2, #(64 * 1024)
//gba_setup_fill_H_loop:
//	str r0, [r1], #4
//	subs r2, #4
//	bne gba_setup_fill_H_loop

	ldr r0,= 0x74
	mov r1, #0
	str r1, [r0]//fix post boot redirect
	ldr r0,= 0x800
	ldr r1,= 0x4770
	strh r1, [r0]//fix sound bias hang

	//Patch the CpuSet and CpuFastSet bios functions for faster sd access (aka larger blocks instead of 4 bytes each time)
	//CpuFastSet
	//ROM:00000BD4                 BEQ     loc_C24
	//replace with jump to address check code

	//this works unsafe at the moment, because it seems interrupts can occur fucking everything up

	//ldr r0,= bios_cpufastset_sd_patch
	//ldr r1,= 0xBD4
	//sub r0, r1	//relative to source address
	//sub r0, #8	//pc + 8 compensation
	//mov r1, #0xEA000000
	//orr r1, r0, lsr #2
	//ldr r0,= 0xBD4
	//str r1, [r0]

	//replace the write to haltcnt in waitintr with a cp15 instruction that does the same on arm9
	ldr r0,= bios_waitintr_fix
	ldr r0, [r0]
	mov r1, #0x344
	str r0, [r1]
	mov r1, #0x1B0
	str r0, [r1]

#ifdef ENABLE_WRAM_ICACHE
	ldr r0,= bios_cpuset_cache_patch
	mov r1, #0x1F4
	str r0, [r1]

	ldr r0,= bios_cpufastset_cache_patch
	mov r1, #0x1F8
	str r0, [r1]
#endif

	//We need to get into privileged mode, misuse the undefined mode for it
	//ldr r0,= gba_start_bkpt
	//sub r0, #0xC	//relative to source address
	//sub r0, #8	//pc + 8 compensation
	//mov r1, #0xEA000000
	//orr r1, r0, lsr #2
	//mov r0, #0xC
	//str r1, [r0]

	ldr r0,= gba_start_bkpt_vram
	bx r0

	//bkpt #0
	//Try out bios checksum
	//swi #0xD0000
	//ldr r1,= 0xBAAE187F
	//cmp r0, r1
	//ldreq r0,= 0x05000000
	//ldreq r1,= 0x3E0
	//streqh r1, [r0]
	//ldr r0,= swi_handler
	//sub r0, #8	//relative to source address
	//sub r0, #8	//pc + 8 compensation
	//mov r1, #0xEA000000
	//orr r1, r0, lsr #2
	//mov r0, #8
	//str r1, [r0]
	//swi #0
gba_setup_loop:
	b gba_setup_loop

//.section .itcm
//swi_handler:
//	push {r0-r12,r14}
//	ldr r0,= 0x05000000
//	ldr r1,= 0x3E0
//	strh r1, [r0]
//swi_handler_loop:
//	b swi_handler_loop
//	pop {r0-r12,r14}
//	movs pc, r14

.section .itcm
//.org 0x4000
//Jump to reset vector
//gba_start_bkpt:
//	ldr r0,= gba_start_bkpt_vram
//	bx r0

.global instruction_abort_handler
instruction_abort_handler:
#ifdef ABT_NO_FIQ
	msr cpsr_c, #0xD7	//immediately disable fiqs
#endif
	cmp lr, #0x08000000
	blt instruction_abort_handler_error
	cmp lr, #0x0E000000
	bge instruction_abort_handler_error
instruction_abort_handler_cont:
	bic lr, #0x06000000
	sub lr, #0x05000000
	sub lr, #0x00FC0000
	subs pc, lr, #4

instruction_abort_handler_error:
	str r12, [r13, #1]
	mov r12, #0x06000000
	orr r12, #0x00010000
	cmp lr, r12
	blt instruction_abort_handler_error_cont
	orr r12, #0x00008000
	cmp lr, r12
	bge instruction_abort_handler_error_cont
	add lr, #0x3F0000
	ldr r12, [r13, #1]
	subs pc, lr, #4
instruction_abort_handler_error_cont:
	add r12, lr, #0x5000000
	add r12, #0x0FC0000
	cmp r12, #0x08000000
	blt instruction_abort_handler_error_2
	cmp r12, #0x0E000000
	movlt lr, r12
	ldrlt r12, [r13, #1]
	blt instruction_abort_handler_cont
instruction_abort_handler_error_2:
#ifdef ABT_NO_FIQ
	msr cpsr_c, #0x97	//enable fiqs
#endif
	mrc p15, 0, r0, c1, c0, 0
	bic r0, #(1 | (1 << 2))	//disable pu and data cache
	bic r0, #(1 << 12) //and cache
	mcr p15, 0, r0, c1, c0, 0

	ldr r0,= 0x06202000
	ldr r1,= 0x46455250
	str r1, [r0]

	sub r0, lr, #4
/*	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 8)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2*/

	b .

.align 4

//undef_inst_handler:
//	ldr pc,= undef_inst_handler_vram

.align 4

.global undef_inst_handler
undef_inst_handler:
	//TODO: if this is an is-nitro breakpoint (arm 0xE7FFFFFF; thumb 0xEFFF), pass control to the debugger

	//make use of the backwards compatible version
	//of the data rights register, so we can use 0xFFFFFFFF instead of 0x33333333
	mov sp, #0xFFFFFFFF
	mcr p15, 0, sp, c5, c0, 0

	mrs sp, spsr
	tst sp, #0x20

	bne bkpt_test_thumb
bkpt_test_arm:
	ldr sp, [lr, #-4]
	cmp sp, #0xE7FFFFFF
	beq is_bkpt
	sub lr, lr, #4
	b no_bkpt
bkpt_test_thumb:
	ldrh sp, [lr, #-2]
	//ugh, lack of registers
	mov sp, sp, lsl #16
	orr sp, sp, #0xFF00
	orr sp, sp, #0x00FF
	cmp sp, #0xEFFFFFFF
	subne lr, lr, #2
	bne no_bkpt
is_bkpt:
	MRS     SP, CPSR
	ORR     SP, SP, #0xC0
	MSR     CPSR_cxsf, SP
	b fiq_hook_cp15_done

no_bkpt:
	//mrc p15, 0, r0, c1, c0, 0
	//bic r0, #(1 | (1 << 2))	//disable pu and data cache
	//bic r0, #(1 << 12) //and cache
	//mcr p15, 0, r0, c1, c0, 0

	//ldr r0,= 0x06202000
	//ldr r1,= 0x46444E55
	//str r1, [r0]

/*	mov r0, lr
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 8)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldr r0, [lr]
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 9)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2*/

	b .

/*	mrs r0, spsr
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 10)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	msr cpsr_c, #0x9F
	mov r0, lr
	msr cpsr_c, #0x9B
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 11)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	b .*/

//for is-nitro
.global fiq_hook
fiq_hook:
	MRS     SP, CPSR
	ORR     SP, SP, #0xC0
	MSR     CPSR_cxsf, SP

	//make use of the backwards compatible version
	//of the data rights register, so we can use 0xFFFFFFFF instead of 0x33333333
	mov sp, #0xFFFFFFFF
	mcr p15, 0, sp, c5, c0, 0

fiq_hook_cp15_done:

	LDR     SP, =0x27FFD9C
	ADD     SP, SP, #1
	STMFD   SP!, {R12,LR}
	MRS     LR, SPSR
	MRC     p15, 0, R12,c1,c0, 0
	STMFD   SP!, {R12,LR}
	BIC     R12, R12, #1
	MCR     p15, 0, R12,c1,c0, 0
	BIC     R12, SP, #1
	LDR     R12, [R12,#0x10]
	CMP     R12, #0
	BLXNE   R12
	LDMFD   SP!, {R12,LR}
	MCR     p15, 0, R12,c1,c0, 0
	MSR     SPSR_cxsf, LR
	LDMFD   SP!, {R12,LR}

	ldr sp,= pu_data_permissions
	mcr p15, 0, sp, c5, c0, 2

	SUBS    PC, LR, #4

.align 4
.pool