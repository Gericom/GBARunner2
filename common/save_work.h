#ifndef __SAVE_WORK_H__
#define __SAVE_WORK_H__

#define SAVE_WORK_STATE_CLEAN	0
#define SAVE_WORK_STATE_DIRTY	1
#define SAVE_WORK_STATE_WAIT	2
#define SAVE_WORK_STATE_SDSAVE	3

typedef struct
{
	uint32_t save_fat_table[128 * 1024 / 512];
	uint8_t save_enabled;
	uint8_t save_state;
	uint16_t fat_table_crc;
	uint32_t saveSize;
} save_work_t;

#endif