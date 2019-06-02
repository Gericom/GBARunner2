#include "vram.h"
#include "math.h"

int math_div(int a, int b)
{
	REG_DIV_CNT = DIV_CNT_MODE_32_32;
	REG_DIV_NUMER32 = a;
	REG_DIV_DENOM = b;
	while (REG_DIV_CNT & DIV_CNT_BUSY);
	return REG_DIV_RESULT32;
}