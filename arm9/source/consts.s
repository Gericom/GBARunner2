#ifndef __CONSTS_H__
#define __CONSTS_H__

#include "../../common/common_defs.s"

#define address_dtcm 0x07800000 //0x01800000
//#define reg_table address_dtcm

//#define address_count_bit_table (address_dtcm + 0x40)

//#define address_read_table_32bit_dtcm (address_dtcm + 0x86C) //0x1000086C
//#define address_read_table_16bit_dtcm (address_dtcm + 0x974) //0x10000974
//#define address_read_table_8bit_dtcm (address_dtcm + 0xB80) //0x10000B80

//#define address_write_table_32bit_dtcm (address_dtcm + 0x140) //0x10000140
//#define address_write_table_16bit_dtcm (address_dtcm + 0x248) //0x10000248
//#define address_write_table_8bit_dtcm (address_dtcm + 0x454) //0x10000454

//#define address_thumb_table_dtcm (address_dtcm + 0xFA0) //(address_dtcm + 0xF98)

#define pu_data_permissions 0x33600603 //0x33600003 //0x33660003

//for debugging the abort handler only!
//registers will be destroyed by a fiq interrupt though
//#define ALLOW_FIQ

#ifdef ALLOW_FIQ
#define CPSR_IRQ_FIQ_BITS	0x80
#else
#define CPSR_IRQ_FIQ_BITS	0xC0
#endif

//enabling the wram icache can significantly improve
//speed in some games (dk3, rayman3, riviera), however
//it may lead to crashes depending on the game.
//This is not good for games that use a lot of
//self-modifying code for instance.
//In general it's better to keep it off
//#define ENABLE_WRAM_ICACHE

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

#define FIRST(a, ...) a
#define SECOND(a, b, ...) b
#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1
#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()
#define BOOL(x) NOT(NOT(x))
#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)(0))
#define _END_OF_ARGUMENTS_(...) BOOL(FIRST(__VA_ARGS__))

#define CAT(a, ...) a ## __VA_ARGS__

#define IF(c) _IF(BOOL(c))
#define _IF(c) CAT(_IF_,c)
#define _IF_0(...)
#define _IF_1(...) __VA_ARGS__

#define IF_ELSE(c) _IF_ELSE(BOOL(c))
#define _IF_ELSE(c) CAT(_IF_ELSE_,c)
#define _IF_ELSE_0(t,f) f
#define _IF_ELSE_1(t,f) t

#define EMPTY()
#define DEFER1(m) m EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()
#define DEFER3(m) m EMPTY EMPTY EMPTY()()()

#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...) EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define FIELDS_MORE(prev, prevsize, name, size, ...) \
	IF_ELSE(name)( \
		name = (prev + (prevsize)); \
		IF(HAS_ARGS(__VA_ARGS__))(    \
			DEFER3(FIELDS_MORE_)()(name, (size), __VA_ARGS__)   \
		) \
	, \
		IF(HAS_ARGS(__VA_ARGS__))(    \
			DEFER3(FIELDS_MORE_)()(prev, (prevsize) + (size), __VA_ARGS__)   \
		) \
	)


#define FIELDS_MORE_() FIELDS_MORE
	
#define FIELDS(baseoffset, name, size, ...) \
	name = (baseoffset); \
	EVAL(FIELDS_MORE(name, (size), ##__VA_ARGS__))

FIELDS(address_dtcm, 
	reg_table, 0x40,
	address_cpu_mode_switch_dtcm, 4 * 31,
	address_thumb_table_dtcm, 4 * 128,
	address_arm_table_dtcm, 4 * 256,
	address_count_bit_table, 0x100,
	address_write_table_32bit_dtcm, 0x108,
	address_write_table_16bit_dtcm, 0x20C,
	address_write_table_8bit_dtcm, 0x418,
	address_read_table_32bit_dtcm, 0x108,
	address_read_table_16bit_dtcm, 0x20C,
	address_read_table_8bit_dtcm, 0x418,
	address_DISPCNT_copy, 0x4,
	address_shadow_dispstat, 0x4,
	address_WAITCNT_copy, 0x4,
	address_dma_shadow_regs_dtcm, 0x30
)

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
