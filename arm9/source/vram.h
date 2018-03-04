#ifndef __VRAM_H__
#define __VRAM_H__

#define ITCM_CODE	__attribute__((section(".itcm"), long_call))
#define PUT_IN_VRAM	__attribute__((section(".vram")))
#define PACKED __attribute__ ((packed))

#define BIT(n) (1 << (n))

#define NULL 0

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed long s32;
typedef signed long long int s64;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef s8 int8_t;
typedef s16 int16_t;
typedef s32 int32_t;
typedef s64 int64_t;

#endif