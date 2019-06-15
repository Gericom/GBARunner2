#include "vram.h"
#include "sd_access.h"
#include "Save.h"
#include "SramSave.h"

static const u8 sReadSramV110Sig[0x10] =
	{0x90, 0xB5, 0xA7, 0xB0, 0x6F, 0x46, 0x38, 0x60, 0x79, 0x60, 0xBA, 0x60, 0x17, 0x48, 0x17, 0x49};

static const u8 sReadSramV111Sig[0x10] =
	{0x70, 0xB5, 0xA0, 0xB0, 0x04, 0x1C, 0x0D, 0x1C, 0x16, 0x1C, 0x08, 0x4A, 0x10, 0x88, 0x08, 0x49};

static const u8 sWriteSramV110Sig[0x10] =
	{0x80, 0xB5, 0x83, 0xB0, 0x6F, 0x46, 0x38, 0x60, 0x79, 0x60, 0xBA, 0x60, 0x09, 0x48, 0x09, 0x49};

static const u8 sWriteSramV111Sig[0x10] =
	{0x30, 0xB5, 0x05, 0x1C, 0x0C, 0x1C, 0x13, 0x1C, 0x0B, 0x4A, 0x10, 0x88, 0x0B, 0x49, 0x08, 0x40};

static const u8 sVerifySramV110Sig[0x10] =
	{0x90, 0xB5, 0xB7, 0xB0, 0x6F, 0x46, 0x38, 0x60, 0x79, 0x60, 0xBA, 0x60, 0x17, 0x48, 0x17, 0x49};

static const u8 sVerifySramV111Sig[0x10] =
	{0x70, 0xB5, 0xB0, 0xB0, 0x04, 0x1C, 0x0D, 0x1C, 0x16, 0x1C, 0x08, 0x4A, 0x10, 0x88, 0x08, 0x49};

static void readSram(u8* src, u8* dst, u32 size)
{
	//reading from main memory is safe without changing permissions
	u8* pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + ((u32)src & 0xFFFF));
	for (int i = 0; i < size; i++)
		*dst++ = *pSave++;
}

static void writeSram(u8* src, u8* dst, u32 size)
{
	for (int i = 0; i < size; i++)
		*dst++ = *src++;
}

static u32 verifySram(u8* src, u8* tgt, u32 size)
{
	//reading from main memory is safe without changing permissions
	const u32 addr = (u32)tgt & 0xFFFF;
	u8*       pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + addr);
	for (int i = 0; i < size; i++)
		if (*pSave++ != *src++)
			return 0x0E000000 | (addr + i);
	return 0;
}


bool sram_patchV110(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadSramV110Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readSram);

	u32* writeFunc = save_findSignature(sWriteSramV110Sig);
	if (!writeFunc)
		return false;
	save_injectJump(writeFunc, (void*)writeSram);

	u32* verifyFunc = save_findSignature(sVerifySramV110Sig);
	if (!verifyFunc)
		return false;
	save_injectJump(verifyFunc, (void*)verifySram);
	return true;
}

bool sram_patchV111(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadSramV111Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readSram);

	u32* writeFunc = save_findSignature(sWriteSramV111Sig);
	if (!writeFunc)
		return false;
	save_injectJump(writeFunc, (void*)writeSram);

	u32* verifyFunc = save_findSignature(sVerifySramV111Sig);
	if (!verifyFunc)
		return false;
	save_injectJump(verifyFunc, (void*)verifySram);
	return true;
}
