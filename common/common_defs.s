#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

//#define ARM7_DLDI

//#define USE_DSI_16MB
//#define USE_3DS_32MB

#define USE_GBA_ADJUSTED_SYNC

//this is currently broken
//#define USE_LOW_LATENCY_IRQ_AUDIO

#if defined(USE_DSI_16MB)
#define UNCACHED_OFFSET     (-0x0A000000)
#define MAIN_MEMORY_BASE    0x0C000000
#define MAIN_MEMORY_END     0x0D000000
#elif defined(USE_3DS_32MB)
#define UNCACHED_OFFSET     (-0x0A000000)
#define MAIN_MEMORY_BASE    0x0C000000
#define MAIN_MEMORY_END     0x0E000000
#else
#define UNCACHED_OFFSET	    0x00800000
#define MAIN_MEMORY_BASE    0x02000000
#define MAIN_MEMORY_END     0x02400000
#endif

#if defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
#define USE_DSP_AUDIO
#endif

#ifndef USE_DSP_AUDIO
//#define USE_MP2000_PATCH
#endif

#define SD_CACHE_SIZE	                    (1424 * 1024)
#define GBARUNNER_DATA_SIZE                 0x1C0000

#ifdef USE_3DS_32MB
#define MAIN_MEMORY_ADDRESS_ROM_DATA		(MAIN_MEMORY_BASE + 0x00040000 + GBARUNNER_DATA_SIZE)
#else
#define MAIN_MEMORY_ADDRESS_ROM_DATA		(MAIN_MEMORY_BASE + 0x00040000)
#endif

#if defined(USE_DSI_16MB)
#define MAIN_MEMORY_ADDRESS_GBARUNNER_DATA	(MAIN_MEMORY_BASE + 0x00E40000)
#elif defined(USE_3DS_32MB)
#define MAIN_MEMORY_ADDRESS_GBARUNNER_DATA	(MAIN_MEMORY_BASE + 0x00040000)
#else
#define MAIN_MEMORY_ADDRESS_GBARUNNER_DATA	(MAIN_MEMORY_BASE + 0x00240000)
#endif
#define SAVE_DATA_SIZE						0x20000
#ifdef USE_3DS_32MB
#define MAIN_MEMORY_ADDRESS_SAVE_DATA		(MAIN_MEMORY_ADDRESS_GBARUNNER_DATA + GBARUNNER_DATA_SIZE - SAVE_DATA_SIZE)
#else
#define MAIN_MEMORY_ADDRESS_SAVE_DATA		(MAIN_MEMORY_END - SAVE_DATA_SIZE)
#endif
#ifdef USE_3DS_32MB
#define ROM_DATA_LENGTH						(MAIN_MEMORY_END - MAIN_MEMORY_ADDRESS_ROM_DATA)
#else
#define ROM_DATA_LENGTH						(MAIN_MEMORY_ADDRESS_GBARUNNER_DATA - MAIN_MEMORY_ADDRESS_ROM_DATA)
#endif
#define ROM_ADDRESS_MAX						(0x08000000 + ROM_DATA_LENGTH)

#define GBA_ADDR_TO_DS_HIGH                 ((MAIN_MEMORY_ADDRESS_ROM_DATA - 0x08000000) & 0xFF000000)
#define GBA_ADDR_TO_DS_LOW                  ((MAIN_MEMORY_ADDRESS_ROM_DATA - 0x08000000 - GBA_ADDR_TO_DS_HIGH) & 0xFFFF0000)

#define sd_cluster_cache  (MAIN_MEMORY_ADDRESS_GBARUNNER_DATA) //0x06820000

#define sd_data_base (sd_cluster_cache + SD_CACHE_SIZE) //0x06840000
#define sd_is_cluster_cached_table (sd_data_base + (32 * 1024)) //(96 * 1024))
#define sd_cluster_cache_info (sd_is_cluster_cached_table + (2 * 64 * 1024))
#define sd_cluster_cache_linked_list (sd_cluster_cache_info + (4096 * 8))
#define sd_sd_info (sd_cluster_cache_linked_list + (4096 * 4 + 4 + 4)) //0x0685C404

#define SOUND_EMU_QUEUE_LEN		64

#define sound_sound_emu_work (sd_sd_info + 36)
#define sound_sound_emu_work_uncached (sound_sound_emu_work + UNCACHED_OFFSET)
#define save_save_work (sound_sound_emu_work + 0x530)
#define save_save_work_uncached (save_save_work + UNCACHED_OFFSET)
#define save_save_work_state_uncached (save_save_work_uncached + ((128 * 1024 / 512) * 4) + 1)
#define open_menu_irq_flag_uncached (save_save_work_uncached + ((128 * 1024 / 512) * 4) + 8)
#define extKeys_uncached (open_menu_irq_flag_uncached + 4)
#ifdef USE_LOW_LATENCY_IRQ_AUDIO
#define gbaDsndChanIrqFlags_uncached (extKeys_uncached + 4)
#define gbaDsndChans0_uncached ((gbaDsndChanIrqFlags_uncached + 2 + 0x1F) & ~0x1F)
#endif

#define CACHE_LINKED_LIST_NIL	4096 //0x8000

#endif