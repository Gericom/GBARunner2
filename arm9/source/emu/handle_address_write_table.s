//this will be copied to the dtcm at runtime (and converted to 16 bit, because the 
//FUCKING COMPILER OBVIOUSLY DOES NOT HAVE A FEATURE TO EMIT ONLY THE BOTTOM 16 BITS OF AN ADDRESS)
.section .text

//32 bit writes means 32 bit alignment
//this means only address / 4
//each entry is a 16 bit offset in the itcm
.global address_write_table_32bit
address_write_table_32bit:
//0x04000000
.word write_address_dispcontrol
//0x04000004-0x0400001C
.rept 7
.word write_address_nomod_32
.endr
//0x04000020
.word gfx_writeBg2PAPB
//0x04000024
.word gfx_writeBg2PCPD
//0x04000028
.word gfx_writeBg2X
//0x0400002C
.word gfx_writeBg2Y
//0x04000028-0x0400005C
.rept 12
.word write_address_nomod_32
.endr
//0x04000060-0x04000084
.rept 10
.word write_address_snd_32
.endr
//0x04000088-0x0400009C
.rept 6
.word write_address_ignore
.endr
//0x040000A0
.word write_address_snd_fifo_A
//0x040000A4
.word write_address_ignore
//0x040000A8
.word write_address_ignore
//0x040000AC
.word write_address_ignore
//0x040000B0
.word write_dma_shadow_src_internal_32 //write_address_dma_src
//0x040000B4
.word write_dma_shadow_dst_internal_32 //write_address_dma_dst
//0x040000B8
.word write_dma_size_control //write_address_dma_size_control
//0x040000BC
.word write_dma_shadow_src_all_32 //write_address_dma_src
//0x040000C0
.word write_dma_shadow_dst_internal_32 //write_address_dma_dst
//0x040000C4
.word write_dma_size_control //write_address_dma_size_control
//0x040000C8
.word write_dma_shadow_src_all_32 //write_address_dma_src
//0x040000CC
.word write_dma_shadow_dst_internal_32 //write_address_dma_dst
//0x040000D0
.word write_dma_size_control //write_address_dma_size_control
//0x040000D4
.word write_dma_shadow_src_all_32 //write_address_dma_src
//0x040000D8
.word write_dma_shadow_dst_all_32 //write_address_dma_dst
//0x040000DC
.word write_dma_size_control
//0x040000E0-0x040000FC
.rept 8
.word write_address_ignore
.endr
//0x04000100
.word write_address_timer
//0x04000104
.word write_address_timer
//0x04000108
.word write_address_timer
//0x0400010C
.word write_address_timer
//0x04000110-0x0400012C
.rept 8
.word write_address_ignore
.endr
//0x04000130
.word write_address_nomod_32
//0x04000134-0x040001FC
.rept 51
.word write_address_ignore
.endr
//0x04000200
.word write_address_ie_if
//0x04000204
.word write_address_wait_control
//0x04000208
.word write_address_nomod_32

//16 bit writes means 16 bit alignment
//this means only address / 2
//each entry is a 16 bit offset in the itcm
.global address_write_table_16bit
address_write_table_16bit:
//0x04000000
.word write_address_dispcontrol
//0x04000002
.word write_address_ignore
//0x04000004
.word write_address_dispstat
//0x04000006
.word write_address_ignore
//0x04000008
.word write_address_nomod_16
//0x0400000A
.word write_address_nomod_16
//0x0400000C
.word gfx_writeBg2Cnt
//0x0400000E-0x0400001E
.rept 9
.word write_address_nomod_16
.endr
//0x04000020
.word gfx_writeBg2PA
//0x04000022
.word gfx_writeBg2PB
//0x04000024
.word gfx_writeBg2PC
//0x04000026
.word gfx_writeBg2PD
//0x04000028
.word gfx_writeBg2X_L
//0x0400002A
.word gfx_writeBg2X_H
//0x0400002C
.word gfx_writeBg2Y_L
//0x0400002E
.word gfx_writeBg2Y_H
//0x04000030-0x0400005E
.rept 24
.word write_address_nomod_16
.endr
//0x04000060-0x04000084
.rept 19
.word write_address_snd_16
.endr
//0x04000086-0x040000AC
.rept 21
.word write_address_ignore
.endr
//0x040000B0
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000B2
.word write_dma_shadow_src_internal_hi16 //write_address_dma_src_top16
//0x040000B4
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000B6
.word write_dma_shadow_dst_internal_hi16 //write_address_dma_dst_top16
//0x040000B8
.word write_dma_shadow_16 //write_address_dma_size
//0x040000BA
.word write_dma_control_2 //write_address_dma_control
//0x040000BC
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000BE
.word write_dma_shadow_src_all_hi16 //write_address_dma_src_top16
//0x040000C0
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000C2
.word write_dma_shadow_dst_internal_hi16 //write_address_dma_dst_top16
//0x040000C4
.word write_dma_shadow_16 //write_address_dma_size
//0x040000C6
.word write_dma_control_2 //write_address_dma_control
//0x040000C8
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000CA
.word write_dma_shadow_src_all_hi16 //write_address_dma_src_top16
//0x040000CC
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000CE
.word write_dma_shadow_dst_internal_hi16 //write_address_dma_dst_top16
//0x040000D0
.word write_dma_shadow_16 //write_address_dma_size
//0x040000D2
.word write_dma_control_2 //write_address_dma_control
//0x040000D4
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000D6
.word write_dma_shadow_src_all_hi16 //write_address_dma_src_top16
//0x040000D8
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000DA
.word write_dma_shadow_dst_all_hi16 //write_address_dma_dst_top16
//0x040000DC
.word write_dma_shadow_16 //write_address_dma_size
//0x040000DE
.word write_dma_control_2 //write_address_dma_control
//0x040000E0-0x040000FE
.rept 16
.word write_address_ignore
.endr
//0x04000100
.word write_address_timer_counter
//0x04000102
.word write_address_timer_control
//0x04000104
.word write_address_timer_counter
//0x04000106
.word write_address_timer_control
//0x04000108
.word write_address_timer_counter
//0x0400010A
.word write_address_timer_control
//0x0400010C
.word write_address_timer_counter
//0x0400010E
.word write_address_timer_control
//0x04000110-0x0400012E
.rept 16
.word write_address_ignore
.endr
//0x04000130
.word write_address_ignore
//0x04000132
.word write_address_nomod_16
//0x04000134-0x040001FE
.rept 102
.word write_address_ignore
.endr
//0x04000200
.word write_address_ie
//0x04000202
.word write_address_if
//0x04000204
.word write_address_wait_control
//0x04000206
.word write_address_ignore
//0x04000208
.word write_address_nomod_16
//0x0400020A
.word write_address_ignore

//each entry is a 16 bit offset in the itcm
.global address_write_table_8bit
address_write_table_8bit:
//0x04000000
.word write_address_dispcontrol_bottom8
//0x04000001
.word write_address_dispcontrol_top8
//0x04000002
.word write_address_ignore
//0x04000003
.word write_address_ignore
//0x04000004-0x0400005F
.rept 92
.word write_address_nomod_8
.endr
//0x04000060-0x04000084
.rept 37
.word write_address_snd_8
.endr
//0x04000085-0x040000AF
.rept 43
.word write_address_ignore
.endr
//0x040000B0-0x040000DF
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_src_internal_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_dst_internal_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_control_bot8
.word write_dma_control_top8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_src_all_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_dst_internal_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_control_bot8
.word write_dma_control_top8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_src_all_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_dst_internal_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_control_bot8
.word write_dma_control_top8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_src_all_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_shadow_dst_all_hi8
.word write_dma_shadow_8
.word write_dma_shadow_8
.word write_dma_control_bot8
.word write_dma_control_top8
//0x040000E0-0x040000FF
.rept 32
.word write_address_ignore
.endr
//shouldn't timer access be implemented for 8 bit access aswell? This probably leads to bugs!
//0x04000100-0x0400010F
.rept 16
.word write_address_nomod_8
.endr
//0x04000110-0x0400012F
.rept 32
.word write_address_ignore
.endr
//0x04000130
.word write_address_ignore
//0x04000131
.word write_address_ignore
//0x04000132
.word write_address_nomod_8
//0x04000133
.word write_address_nomod_8
//0x04000134-0x040001FF
.rept 204
.word write_address_ignore
.endr
//0x04000200
.word write_address_ie_bottom8
//0x04000201
.word write_address_ie_top8
//0x04000202
.word write_address_if_bottom8
//0x04000203
.word write_address_if_top8
//0x04000204
.word write_address_wait_control_bottom8
//0x04000205
.word write_address_wait_control_top8
//0x04000206
.word write_address_ignore
//0x04000207
.word write_address_ignore
//0x04000208
.word write_address_nomod_8
//0x04000209
.word write_address_nomod_8
//0x0400020A
.word write_address_ignore
//0x0400020B
.word write_address_ignore