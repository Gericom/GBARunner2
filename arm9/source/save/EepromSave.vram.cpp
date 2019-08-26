#include "vram.h"
#include "sd_access.h"
#include "gamePatches.h"
#include "Save.h"
#include "EepromSave.h"

//todo: Moero!! Jaleco Collection (Japan) reports EEPROM_V124, but the signatures below don't work!

static const u8 sReadEepromDwordV111Sig[0x10] =
	{0xB0, 0xB5, 0xAA, 0xB0, 0x6F, 0x46, 0x79, 0x60, 0x39, 0x1C, 0x08, 0x80, 0x38, 0x1C, 0x01, 0x88};

static const u8 sReadEepromDwordV120Sig[0x10] =
	{0x70, 0xB5, 0xA2, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x03, 0x0C, 0x03, 0x48, 0x00, 0x68, 0x80, 0x88};

static const u8 sProgramEepromDwordV111Sig[0x10] =
	{0x80, 0xB5, 0xAA, 0xB0, 0x6F, 0x46, 0x79, 0x60, 0x39, 0x1C, 0x08, 0x80, 0x38, 0x1C, 0x01, 0x88};

//changed in EEPROM_V124
static const u8 sProgramEepromDwordV120Sig[0x10] =
	{0x30, 0xB5, 0xA9, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x04, 0x0C, 0x03, 0x48, 0x00, 0x68, 0x80, 0x88};

//changed in EEPROM_V126
static const u8 sProgramEepromDwordV124Sig[0x10] =
	{0xF0, 0xB5, 0xAC, 0xB0, 0x0D, 0x1C, 0x00, 0x04, 0x01, 0x0C, 0x12, 0x06, 0x17, 0x0E, 0x03, 0x48};

static const u8 sProgramEepromDwordV126Sig[0x10] =
	{0xF0, 0xB5, 0x47, 0x46, 0x80, 0xB4, 0xAC, 0xB0, 0x0E, 0x1C, 0x00, 0x04, 0x05, 0x0C, 0x12, 0x06};

//not in EEPROM_V111
//could be used to identify the eeprom size, but not strictly needed
/*static u16 identifyEeprom(u16 kbitSize)
{
	return 0;
}*/

extern "C" u16 readEepromDword_impl(u16 epAdr, u16* dst)
{
	//reading from main memory is safe without changing permissions
	u8* pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (epAdr << 3));
	for (int i = 0; i < 8; i++)
		((u8*)dst)[7 - i] = *pSave++;
	return 0;
}

GBA_INTERWORK_BRIDGE(readEepromDword)

extern "C" u16 programEepromDword_impl(u16 epAdr, u16* src)
{
	//I would rather have written to the save in main memory directly,
	//but it wouldn't work right for whatever reason :/
	u8* pSave = (u8*)(0x0E000000 + (epAdr << 3));
	for (int i = 0; i < 8; i++)
		*pSave++ = ((u8*)src)[7 - i];
	return 0;
}

GBA_INTERWORK_BRIDGE(programEepromDword)

bool eeprom_patchV111(const save_type_t* type)
{
	u32* readFunc = gptc_findSignature(sReadEepromDwordV111Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = gptc_findSignature(sProgramEepromDwordV111Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV120(const save_type_t* type)
{
	u32* readFunc = gptc_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = gptc_findSignature(sProgramEepromDwordV120Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV124(const save_type_t* type)
{
	u32* readFunc = gptc_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = gptc_findSignature(sProgramEepromDwordV124Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV126(const save_type_t* type)
{
	u32* readFunc = gptc_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = gptc_findSignature(sProgramEepromDwordV126Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}
