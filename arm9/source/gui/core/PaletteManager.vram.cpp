#include "vram.h"
#include "sd_access.h"
#include "PaletteManager.h"

void PaletteManager::Clear()
{
	for (int i = 0; i < 256; i++)
		palette[i].color = 0;
	UndimRows(0xFFFF);
}

void PaletteManager::Apply(u16* dst)
{
	if (_dimmedRows == 0)
		arm9_memcpy16(dst, (u16*)&palette, sizeof(palette) >> 1);
	else
	{
		for (int i = 0; i < 16; i++)
		{
			if (_dimmedRows & (1 << i))
				for (int j = 0; j < 16; j++)
					*dst++ = (palette[i * 16 + j].color & 0x7BDE) >> 1;
			else
				for (int j = 0; j < 16; j++)
					*dst++ = palette[i * 16 + j].color;
		}
	}
}
