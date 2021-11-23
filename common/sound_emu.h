#pragma once

#include "common_defs.s"

typedef struct
{
	volatile u8 req_size_lock;
	volatile u8 req_size;
	volatile u8 req_write_ptr;
	volatile u8 req_read_ptr;
	u32 req_queue[SOUND_EMU_QUEUE_LEN];
	volatile u8 resp_size_lock;
	volatile u8 resp_size;
	volatile u8 resp_write_ptr;
	volatile u8 resp_read_ptr;
	u32 resp_queue[SOUND_EMU_QUEUE_LEN][4];
	//sound registers
	u16 reg_gb_nr10;
	u16 reg_gb_nr11_12;
	u32 reg_gb_nr13_14;

	u16 reg_gb_nr21_22;
	u16 padding;
	u32 reg_gb_nr23_24;

	u16 reg_gb_nr30;
	u16 reg_gb_nr31_32;
	u32 reg_gb_nr33_34;

	u32 reg_gb_nr41_42;
	u32 reg_gb_nr43_44;

	u16 reg_gb_nr50_51;
	u16 reg_gba_snd_mix;

	u32 reg_gb_nr52;
} sound_emu_work_t;

#ifdef USE_LOW_LATENCY_IRQ_AUDIO
typedef struct
{
	s8 fifoBuffer[32];
	u32 writeOffset;
	u32 dmaSrc;
} __attribute__((aligned(32))) gba_dsnd_channel_t;
#endif