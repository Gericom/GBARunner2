#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "NtftFont.h"

NtftFont::NtftFont(const void* data)
{
	const ntft_header_t* font = (const ntft_header_t*)data;
	_font.header = font;
	_font.characterInfo = (const ntft_cinfo_t*)((u32)font + font->charInfoOffset);
	_font.glyphData = (const ntft_gdata_t*)((u32)font + font->glyphDataOffset);
}

void NtftFont::MeasureString(const char* text, int &width, int &height) const
{
	width = 0;
	height = _font.characterInfo->characterHeight;
	int tmpwidth = 0;
	char c = *text++;
	while (c != 0)
	{
		int end = 0;
		if (c == '\n')
		{
			if (width < tmpwidth)
				width = tmpwidth;
			tmpwidth = 0;
			height += _font.characterInfo->characterHeight + 1;
		}
		else
		{
			if ((tmpwidth + _font.characterInfo->characters[c].characterBeginOffset) >= 0)
				tmpwidth += _font.characterInfo->characters[c].characterBeginOffset;
			tmpwidth += _font.characterInfo->characters[c].characterWidth;
			end = _font.characterInfo->characters[c].characterEndOffset;
		}
		c = *text++;
		if (c != 0) 
			tmpwidth += end;
	}
	if (width < tmpwidth) 
		width = tmpwidth;
}

void NtftFont::CreateStringData(const char* text, u8* dst, int stride) const
{
	//todo: maybe support alignment
	//int width;
	//int height;
	//GetStringSize(text, width, height);
	int xpos = 0;
	int ypos = 0;
	bool nodraw = false;
	char c = *text++;
	while (c != 0)
	{
		if (c == '\n')
		{
			xpos = 0;
			ypos += _font.characterInfo->characterHeight + 1;
			nodraw = false;
		}
		else
		{
			if ((xpos + _font.characterInfo->characters[c].characterBeginOffset) >= 0)
				xpos += _font.characterInfo->characters[c].characterBeginOffset;
			if(xpos + _font.characterInfo->characters[c].characterWidth > stride)
				nodraw = true;
			if (!nodraw)
			{
				u8* glyph = (uint8_t*)&_font.glyphData->glyphData[_font.characterInfo->characters[c].glyphDataOffset];
				u8* dst_ptr = dst + ypos * stride + xpos;
				for (int y = 0; y < _font.characterInfo->characterHeight; y++)
				{
					for (int x = 0; x < _font.characterInfo->characters[c].characterWidth; x++)
					{
						u8 data = *glyph++;
						int oldval = *dst_ptr;
						int newval = oldval + data;
						if (newval > 255)
							newval = 255;
						//write the byte via read-modify-write for vram compatibility
						MI_WriteByte(dst_ptr, newval);
						dst_ptr++;
						//*dst_ptr++ = newval;
					}
					dst_ptr -= _font.characterInfo->characters[c].characterWidth;
					dst_ptr += stride;
				}
				xpos += _font.characterInfo->characters[c].characterWidth;
				xpos += _font.characterInfo->characters[c].characterEndOffset;
			}
		}
		c = *text++;
	}
}
