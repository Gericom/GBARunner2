#include "vram.h"
#include "sd_access.h"
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

//static const u8 sVerifyEepromDwordV120Sig[0x10] =
//	{0x30, 0xB5, 0x82, 0xB0, 0x0C, 0x1C, 0x00, 0x04, 0x01, 0x0C, 0x00, 0x25, 0x03, 0x48, 0x00, 0x68};

//not in EEPROM_V111
//could be used to identify the eeprom size, but not strictly needed
/*static u16 identifyEeprom(u16 kbitSize)
{
	return 0;
}*/

static u16 readEepromDword(u16 epAdr, u16* dst)
{
	//reading from main memory is safe without changing permissions
	u16* pSave = (u16*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (epAdr << 3));
	for (int i = 0; i < 8; i += 2)
		*dst++ = *pSave++;
	return 0;
}

static u16 programEepromDword(u16 epAdr, u16* src)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) | 0x00800000);
	u16*       pSave = (u16*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (epAdr << 3));
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < 8; i += 2)
			*pSave++ = *src++;
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

/*static u16 VerifyEepromDword(u16 epAdr, u16* src)
{
	//todo: maybe implement parameter error 0x80ff

	//reading from main memory is safe without changing permissions
	u16* pSave = (u16*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (epAdr << 3));
	for (int i = 0; i < 8; i += 2)
		if (*pSave++ != *src++)
			return 0x8000;
	return 0;
}*/

bool eeprom_patchV111(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadEepromDwordV111Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = save_findSignature(sProgramEepromDwordV111Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV120(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = save_findSignature(sProgramEepromDwordV120Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV124(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = save_findSignature(sProgramEepromDwordV124Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}

bool eeprom_patchV126(const save_type_t* type)
{
	u32* readFunc = save_findSignature(sReadEepromDwordV120Sig);
	if (!readFunc)
		return false;
	save_injectJump(readFunc, (void*)readEepromDword);

	u32* progFunc = save_findSignature(sProgramEepromDwordV126Sig);
	if (!progFunc)
		return false;
	save_injectJump(progFunc, (void*)programEepromDword);
	return true;
}
