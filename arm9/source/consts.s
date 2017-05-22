#ifndef __CONSTS_H__
#define __CONSTS_H__

#include "../../common/common_defs.s"

#define address_dtcm 0x04F00000 //0x01800000
#define reg_table address_dtcm

#define address_count_bit_table (address_dtcm + 0x40)

#define address_read_table_32bit_dtcm (address_dtcm + 0x86C) //0x1000086C
#define address_read_table_16bit_dtcm (address_dtcm + 0x974) //0x10000974
#define address_read_table_8bit_dtcm (address_dtcm + 0xB80) //0x10000B80

#define address_write_table_32bit_dtcm (address_dtcm + 0x140) //0x10000140
#define address_write_table_16bit_dtcm (address_dtcm + 0x248) //0x10000248
#define address_write_table_8bit_dtcm (address_dtcm + 0x454) //0x10000454

#define pu_data_permissions 0x33600603 //0x33600003 //0x33660003

#ifdef __ASSEMBLER__
@destroys r12, r13
.macro printreg reg
	mov r13, r0
	mov r0, \reg
	mov r12, lr
	ldr lr,= print_address_isnitro
	blx lr
	mov lr, r12
	mov r0, r13
.endm
#endif

/*#define CACHE_STRATEGY_LRU*/
/*this strategy is very bad as well*/
/*#define CACHE_STRATEGY_MRU*/
/*this strategy isn't the best either*/
/*#define CACHE_STRATEGY_LFU*/
/*#define CACHE_STRATEGY_ROUND_ROBIN*/

#define CACHE_STRATEGY_LRU_LIST


#define CACHE_BLOCK_SIZE_SHIFT	9
#define CACHE_BLOCK_SIZE		(1 << CACHE_BLOCK_SIZE_SHIFT)
#define CACHE_BLOCK_SIZE_MASK	(CACHE_BLOCK_SIZE - 1)

#endif
