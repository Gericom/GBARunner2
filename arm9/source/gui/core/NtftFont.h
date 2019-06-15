#pragma once

#define NTFT_SIGNATURE_HEADER			MKTAG('N', 'T', 'F', 'T')
#define NTFT_SIGNATURE_CHARACTER_INFO	MKTAG('C', 'I', 'N', 'F')
#define NTFT_SIGNATURE_GLYPH_DATA		MKTAG('G', 'L', 'P', 'D')

struct ntft_header_t
{
	u32 signature;
	u32 fileSize;
	u32 charInfoOffset;
	u32 glyphDataOffset;
};

struct ntft_cinfo_char_t
{
	u32 glyphDataOffset;
	s32 characterBeginOffset;
	u32 characterWidth;
	s32 characterEndOffset;
};

struct ntft_cinfo_t
{
	u32               signature;
	u32               blockSize;
	u32               characterHeight;
	ntft_cinfo_char_t characters[256];
};

struct ntft_gdata_t
{
	uint32_t signature;
	uint32_t blockSize;
	uint32_t glyphDataSize;
	uint8_t  glyphData[0];
};

struct ntft_t
{
	const ntft_header_t* header;
	const ntft_cinfo_t*  characterInfo;
	const ntft_gdata_t*  glyphData;
};

class NtftFont
{
	ntft_t _font;

public:
	explicit NtftFont(const void* data);

	void MeasureString(const char* text, int& width, int& height) const;
	void CreateStringData(const char* text, u8* dst, int stride) const;
	int  GetFontHeight() const { return _font.characterInfo->characterHeight; }
};
