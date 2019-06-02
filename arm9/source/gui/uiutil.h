#pragma once

union XBGR1555
{
	uint16_t color;

	struct
	{
		uint16_t r : 5;
		uint16_t g : 5;
		uint16_t b : 5;
		uint16_t x : 1;
	};
};

void uiutil_convertToObj(u8* src, int width, int height, int stride, u16* dst);