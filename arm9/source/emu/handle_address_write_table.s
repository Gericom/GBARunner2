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
//0x04000004-0x0400005C
.rept 23
.word write_address_nomod_32
.endr
//0x04000060-0x040000AC
//.rept 20
//.word write_address_ignore
//.endr
//0x04000060-0x0400007C
.rept 8
.word write_address_ignore
.endr
//0x04000080
.word write_address_sound_cnt
//0x04000084-0x0400009C
.rept 7
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
.word write_dma_shadow_32 //write_address_dma_src
//0x040000B4
.word write_dma_shadow_32 //write_address_dma_dst
//0x040000B8
.word write_dma_size_control //write_address_dma_size_control
//0x040000BC
.word write_dma_shadow_32 //write_address_dma_src
//0x040000C0
.word write_dma_shadow_32 //write_address_dma_dst
//0x040000C4
.word write_dma_size_control //write_address_dma_size_control
//0x040000C8
.word write_dma_shadow_32 //write_address_dma_src
//0x040000CC
.word write_dma_shadow_32 //write_address_dma_dst
//0x040000D0
.word write_dma_size_control //write_address_dma_size_control
//0x040000D4
.word write_dma_shadow_32 //write_address_dma_src
//0x040000D8
.word write_dma_shadow_32 //write_address_dma_dst
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
//0x04000006-0x0400005E
.rept 45
.word write_address_nomod_16
.endr
//0x04000060-0x040000AE
//.rept 40
//.word write_address_ignore
//.endr
//0x04000060-0x0400007C
.rept 16
.word write_address_ignore
.endr
//0x04000080
.word write_address_ignore //write_address_sound_cnt_bottom16
//0x04000082
.word write_address_sound_cnt_top16
//0x04000084-0x040000AC
.rept 22
.word write_address_ignore
.endr
//0x040000B0
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000B2
.word write_dma_shadow_16 //write_address_dma_src_top16
//0x040000B4
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000B6
.word write_dma_shadow_16 //write_address_dma_dst_top16
//0x040000B8
.word write_dma_shadow_16 //write_address_dma_size
//0x040000BA
.word write_dma_control_2 //write_address_dma_control
//0x040000BC
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000BE
.word write_dma_shadow_16 //write_address_dma_src_top16
//0x040000C0
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000C2
.word write_dma_shadow_16 //write_address_dma_dst_top16
//0x040000C4
.word write_dma_shadow_16 //write_address_dma_size
//0x040000C6
.word write_dma_control_2 //write_address_dma_control
//0x040000C8
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000CA
.word write_dma_shadow_16 //write_address_dma_src_top16
//0x040000CC
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000CE
.word write_dma_shadow_16 //write_address_dma_dst_top16
//0x040000D0
.word write_dma_shadow_16 //write_address_dma_size
//0x040000D2
.word write_dma_control_2 //write_address_dma_control
//0x040000D4
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000D6
.word write_dma_shadow_16 //write_address_dma_src_top16
//0x040000D8
.word write_dma_shadow_16 //write_address_nomod_16
//0x040000DA
.word write_dma_shadow_16 //write_address_dma_dst_top16
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
//0x04000060-0x040000AF
.rept 80
.word write_address_ignore
.endr
//0x040000B0-0x040000DF
.rept 10
.word write_dma_shadow_8
.endr
.word write_dma_control_bot8
.word write_dma_control_top8
.rept 10
.word write_dma_shadow_8
.endr
.word write_dma_control_bot8
.word write_dma_control_top8
.rept 10
.word write_dma_shadow_8
.endr
.word write_dma_control_bot8
.word write_dma_control_top8
.rept 10
.word write_dma_shadow_8
.endr
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