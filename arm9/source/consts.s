
#ifdef __ASSEMBLER__
address_dtcm = 0x04F00000 @0x01800000

reg_table = address_dtcm

address_count_bit_table = (address_dtcm + 0x40)

address_read_table_32bit_dtcm = (address_dtcm + 0x86C) @0x1000086C
address_read_table_16bit_dtcm = (address_dtcm + 0x974) @0x10000974
address_read_table_8bit_dtcm = (address_dtcm + 0xB80) @0x10000B80

address_write_table_32bit_dtcm = (address_dtcm + 0x140) @0x10000140
address_write_table_16bit_dtcm = (address_dtcm + 0x248) @0x10000248
address_write_table_8bit_dtcm = (address_dtcm + 0x454) @0x10000454

sd_cluster_cache = 0x06820000

sd_data_base = 0x06840000
sd_is_cluster_cached_table = (sd_data_base + (224 * 1024)) @(96 * 1024))
sd_cluster_cache_info = (sd_is_cluster_cached_table + (16 * 1024))
sd_sd_info = (sd_cluster_cache_info + (256 * 8 + 4)) @0x0685C404


pu_data_permissions = 0x33600003 @0x33660003

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
#define CACHE_STRATEGY_ROUND_ROBIN


#define CACHE_BLOCK_SIZE_SHIFT	9
#define CACHE_BLOCK_SIZE		(1 << CACHE_BLOCK_SIZE_SHIFT)
#define CACHE_BLOCK_SIZE_MASK	(CACHE_BLOCK_SIZE - 1)
