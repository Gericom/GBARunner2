#pragma once

#define DIV_CNT_MODE_32_32		0
#define DIV_CNT_MODE_64_32		1
#define DIV_CNT_MODE_64_64		2

#define DIV_CNT_DIV_BY_ZERO		(1 << 14)
#define DIV_CNT_BUSY			(1 << 15)

#define REG_DIV_CNT				(*(vu16*)0x04000280)
#define REG_DIV_NUMER32			(*(vu32*)0x04000290)
#define REG_DIV_NUMER64			(*(vu64*)0x04000290)
#define REG_DIV_DENOM			(*(vu64*)0x04000298)
#define REG_DIV_RESULT32		(*(vu32*)0x040002A0)
#define REG_DIV_RESULT64		(*(vu64*)0x040002A0)
#define REG_DIV_REM_RESULT32	(*(vu32*)0x040002A8)
#define REG_DIV_REM_RESULT64	(*(vu64*)0x040002A8)

#ifdef __cplusplus
extern "C" {
#endif

int math_div(int a, int b);

#ifdef __cplusplus
}
#endif
