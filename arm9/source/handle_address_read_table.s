.section .text

.global address_read_table_32bit
address_read_table_32bit:
//0x04000000
.word read_address_dispcontrol
//0x04000004 TODO: VCOUNT fix for 32 bit read!
.word read_address_nomod_32
//0x04000008
.word read_address_nomod_32
//0x0400000C
.word read_address_nomod_32
//0x04000010-0x04000044
.rept 14
.word read_address_undefined_memory_32
.endr
//0x04000048
.word read_address_nomod_32
//0x0400004C
.word read_address_undefined_memory_32
//0x04000050
.word read_address_nomod_32
//0x04000054
.word read_address_undefined_memory_32
//0x04000058
.word read_address_undefined_memory_32
//0x0400005C
.word read_address_undefined_memory_32
//0x04000060-0x040000AC
.rept 20
.word read_address_ignore
.endr
//0x040000B0
.word read_address_undefined_memory_32
//0x040000B4
.word read_address_undefined_memory_32
//0x040000B8
.word read_address_nomod_32
//0x040000BC
.word read_address_undefined_memory_32
//0x040000C0
.word read_address_undefined_memory_32
//0x040000C4
.word read_address_nomod_32
//0x040000C8
.word read_address_undefined_memory_32
//0x040000CC
.word read_address_undefined_memory_32
//0x040000D0
.word read_address_nomod_32
//0x040000D4
.word read_address_undefined_memory_32
//0x040000D8
.word read_address_undefined_memory_32
//0x040000DC
.word read_address_nomod_32
//0x040000E0-0x040000FC
.rept 8
.word read_address_ignore
.endr
//0x04000100
.word read_address_timer
//0x04000104
.word read_address_timer
//0x04000108
.word read_address_timer
//0x0400010C
.word read_address_timer
//0x04000110-0x0400012C
.rept 8
.word read_address_undefined_memory_32
.endr
//0x04000130
.word read_address_nomod_32
//0x04000134-0x040001FC
.rept 51
.word read_address_ignore
.endr
//0x04000200
.word read_address_ie_if
//0x04000204
.word read_address_wait_control
//0x04000208
.word read_address_nomod_32

.global address_read_table_16bit
address_read_table_16bit:
//0x04000000
.word read_address_dispcontrol
//0x04000002
.word read_address_ignore
//0x04000004
.word read_address_nomod_16
//0x04000006
.word read_address_vcount
//0x04000008
.word read_address_nomod_16
//0x0400000A
.word read_address_nomod_16
//0x0400000C
.word read_address_nomod_16
//0x0400000E
.word read_address_nomod_16
//0x04000010-0x04000046
.rept 28
.word read_address_undefined_memory_16
.endr
//0x04000048
.word read_address_nomod_16
//0x0400004A
.word read_address_nomod_16
//0x0400004C
.word read_address_undefined_memory_16
//0x0400004E
.word read_address_undefined_memory_16
//0x04000050
.word read_address_nomod_16
//0x04000052
.word read_address_nomod_16
//0x04000054
.word read_address_undefined_memory_16
//0x04000056
.word read_address_undefined_memory_16
//0x04000058
.word read_address_undefined_memory_16
//0x0400005A
.word read_address_undefined_memory_16
//0x0400005C
.word read_address_undefined_memory_16
//0x0400005E
.word read_address_undefined_memory_16
//0x04000060-0x040000AE
.rept 40
.word read_address_ignore
.endr
//0x040000B0
.word read_address_undefined_memory_16
//0x040000B2
.word read_address_undefined_memory_16
//0x040000B4
.word read_address_undefined_memory_16
//0x040000B6
.word read_address_undefined_memory_16
//0x040000B8
.word read_address_undefined_memory_16
//0x040000BA
.word read_address_nomod_16
//0x040000BC
.word read_address_undefined_memory_16
//0x040000BE
.word read_address_undefined_memory_16
//0x040000C0
.word read_address_undefined_memory_16
//0x040000C2
.word read_address_undefined_memory_16
//0x040000C4
.word read_address_undefined_memory_16
//0x040000C6
.word read_address_nomod_16
//0x040000C8
.word read_address_undefined_memory_16
//0x040000CA
.word read_address_undefined_memory_16
//0x040000CC
.word read_address_undefined_memory_16
//0x040000CE
.word read_address_undefined_memory_16
//0x040000D0
.word read_address_undefined_memory_16
//0x040000D2
.word read_address_nomod_16
//0x040000D4
.word read_address_undefined_memory_16
//0x040000D6
.word read_address_undefined_memory_16
//0x040000D8
.word read_address_undefined_memory_16
//0x040000DA
.word read_address_undefined_memory_16
//0x040000DC
.word read_address_undefined_memory_16
//0x040000DE
.word read_address_nomod_16
//0x040000E0-0x040000FE
.rept 16
.word read_address_undefined_memory_16
.endr
//0x04000100
.word read_address_timer_counter
//0x04000102
.word read_address_nomod_16
//0x04000104
.word read_address_timer_counter
//0x04000106
.word read_address_nomod_16
//0x04000108
.word read_address_timer_counter
//0x0400010A
.word read_address_nomod_16
//0x0400010C
.word read_address_timer_counter
//0x0400010E
.word read_address_nomod_16
//0x04000110-0x0400012E
.rept 16
.word read_address_ignore
.endr
//0x04000130
.word read_address_nomod_16
//0x04000132
.word read_address_nomod_16
//0x04000134-0x040001FE
.rept 102
.word read_address_ignore
.endr
//0x04000200
.word read_address_ie
//0x04000202
.word read_address_if
//0x04000204
.word read_address_wait_control
//0x04000206
.word read_address_ignore
//0x04000208
.word read_address_nomod_16
//0x0400020A
.word read_address_ignore

.global address_read_table_8bit
address_read_table_8bit:
//0x04000000
.word read_address_dispcontrol_bottom8
//0x04000001
.word read_address_dispcontrol_top8
//0x04000002
.word read_address_ignore
//0x04000003
.word read_address_ignore
//0x04000004
.word read_address_nomod_8
//0x04000005
.word read_address_nomod_8
//0x04000006
.word read_address_vcount
//0x04000007
.word read_address_ignore
//0x04000008-0x0400005F
.rept 88
.word read_address_nomod_8
.endr
//0x04000060-0x040000AF
.rept 80
.word read_address_ignore
.endr
//0x040000B0-0x040000DF
//shouldn't dma access be implemented for 8 bit access aswell? This probably leads to bugs!
.rept 48
.word read_address_nomod_8
.endr
//0x040000E0-0x040000FF
.rept 32
.word read_address_undefined_memory_8
.endr
//shouldn't timer access be implemented for 8 bit access aswell? This probably leads to bugs!
//0x04000100-0x0400010F
.rept 16
.word read_address_nomod_8
.endr
//0x04000110-0x0400012F
.rept 32
.word read_address_ignore
.endr
//0x04000130
.word read_address_nomod_8
//0x04000131
.word read_address_nomod_8
//0x04000132
.word read_address_nomod_8
//0x04000133
.word read_address_nomod_8
//0x04000134-0x040001FF
.rept 204
.word read_address_ignore
.endr
//0x04000200
.word read_address_ie_bottom8
//0x04000201
.word read_address_ie_top8
//0x04000202
.word read_address_if_bottom8
//0x04000203
.word read_address_if_top8
//0x04000204
.word read_address_wait_control_bottom8
//0x04000205
.word read_address_wait_control_top8
//0x04000206
.word read_address_ignore
//0x04000207
.word read_address_ignore
//0x04000208
.word read_address_nomod_8
//0x04000209
.word read_address_nomod_8
//0x0400020A
.word read_address_ignore
//0x0400020B
.word read_address_ignore