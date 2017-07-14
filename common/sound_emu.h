#ifndef __SOUND_EMU_H__
#define __SOUND_EMU_H__

#include "common_defs.s"

typedef struct
{
	volatile u8 req_size_lock;
	volatile u8 req_size;
	u8 req_write_ptr;
	u8 req_read_ptr;
	u32 req_queue[SOUND_EMU_QUEUE_LEN];
	volatile u8 resp_size_lock;
	volatile u8 resp_size;
	u8 resp_write_ptr;
	u8 resp_read_ptr;
	u32 resp_queue[SOUND_EMU_QUEUE_LEN][4];
} sound_emu_work_t;

#endif