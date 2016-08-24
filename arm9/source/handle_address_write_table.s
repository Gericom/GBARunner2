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
.rept 22
.word write_address_nomod_32
.endr
//0x04000060-0x040000AC
.rept 19
.word write_address_ignore
.endr
//0x040000B0
.word write_address_dma_src
//0x040000B4
.word write_address_dma_dst
//0x040000B8
.word write_address_dma_size_control
//0x040000BC
.word write_address_dma_src
//0x040000C0
.word write_address_dma_dst
//0x040000C4
.word write_address_dma_size_control
//0x040000C8
.word write_address_dma_src
//0x040000CC
.word write_address_dma_dst
//0x040000D0
.word write_address_dma_size_control
//0x040000D4
.word write_address_dma_src
//0x040000D8
.word write_address_dma_dst
//0x040000DC
.word write_address_dma_size_control
//0x040000E0-0x040000FC
.rept 7
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
.rept 7
.word
.endr
//0x04000130
.word write_address_nomod_32
//0x04000134-0x040001FC
.rept 50
.word write_address_ignore
.endr
//0x04000200
.word write_address_ie_if
//0x04000204
.word write_address_wait_control
//0x04000208
.word write_address_nomod_32