#include "vram.h"
#include "sd_access.h"
#include "FlashSave.h"

//== API functions
//u16 IdentifyFlash();
//u16 SetFlashTimerIntr(u8 timerNo, void (**IntrFunc)(void));
//u16 (*EraseFlashChip)();
//u16 (*EraseFlashSector)(u16 secNo);
//u16 (*ProgramFlashSector)(u16 secNo,u8 *src);
//u32 VerifyFlashSector(u16 secNo,u8 *src);
//void ReadFlash(u16 secNo,u32 offset,u8 *dst,u32 size);
//== API variables
//const flashType *flash;
//u16 flash_remainder;

enum FlashVersion : u32
{
	FLASH_VERSION_NONE,
	FLASH_VERSION_V123_V124
};

struct flash_patchinfo_t
{
	FlashVersion version;
	u32          tagAddress;
};

static flash_patchinfo_t sPatchInfo;

//FLASH_V123 and FLASH_V124
static const char sFlashV123Tag[12] = "FLASH_V123";
static const char sFlashV124Tag[12] = "FLASH_V124";

#define FLASH_V123_OFFSET_PROG_SECTOR		0x24
#define FLASH_V123_OFFSET_ERASE_CHIP		0x28
#define FLASH_V123_OFFSET_ERASE_SECTOR		0x2C
#define FLASH_V123_OFFSET_POLLING_SR		0x30
#define FLASH_V123_OFFSET_FL_MAXTIME		0x34
#define FLASH_V123_OFFSET_FLASH				0x38
#define FLASH_V123_OFFSET_READ				0x90
#define FLASH_V123_OFFSET_VERIFY_SECTOR		0x9C

struct flash_v123_sector
{
	u32 size;
	u8  shift;
	u16 count;
	u16 top;
};

struct flash_v123_type
{
	u32               romSize;
	flash_v123_sector sector;
	u16               agbWait[2];
	u8                makerID;
	u8                deviceID;
};

static flash_v123_type sFlashType;
static const u16       sMaxTime[] = {0xA, 0xFFBD, 0xC2, 0xA, 0xFFBD, 0xC2, 0x28, 0xFFBD, 0xC2, 0xC8, 0xFFBD, 0xC2};

/*
 *  ProgramFlashSector = **v1;
  EraseFlashChip = (*v1)[1];
  EraseFlashSector = (*v1)[2];
  pollingSR = (*v1)[3];
  fl_maxtime = (*v1)[4];
  flash = (*v1 + 5);
 */

static const u8 sIdentifyFlashV123Sig[0x10] =
	{0x10, 0xB5, 0x07, 0x4A, 0x10, 0x88, 0x07, 0x49, 0x08, 0x40, 0x03, 0x21, 0x08, 0x43, 0x10, 0x80};
static const u8 sReadFlashV123Sig[0x10] =
	{0xF0, 0xB5, 0xA0, 0xB0, 0x0D, 0x1C, 0x16, 0x1C, 0x1F, 0x1C, 0x00, 0x04, 0x04, 0x0C, 0x08, 0x4A};

#define CP15_SET_DATA_PROT(x)		do { asm volatile("mcr p15, 0, %0, c5, c0, 2" :: "r"((x))); } while(0)
#define CP15_INVALIDATE_ICACHE()	do { asm volatile("mcr p15, 0, %0, c7, c5, 0" :: "r"(0)); } while(0)

static u16 eraseFlashChip()
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) | 0x00800000);
	u8*        pSave = (u8*)MAIN_MEMORY_ADDRESS_SAVE_DATA;
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < SAVE_DATA_SIZE; i++)
		{
			*pSave++ = 0xFF;
			vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		}
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

static u16 eraseFlashSector(u16 secNo)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) | 0x00800000);
	u8*        pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12));
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < (1 << 12); i++)
		{
			*pSave++ = 0xFF;
			vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		}
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

static u16 programFlashSector(u16 secNo, u8* src)
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) | 0x00800000);
	u8*        pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12));
	//disable irqs
	u32 irq = *(vu32*)0x04000208;
	*(vu32*)0x04000208 = 0;
	{
		CP15_SET_DATA_PROT(0x33333333);
		for (int i = 0; i < (1 << 12); i++)
		{
			*pSave++ = *src++;
			vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_DIRTY;
		}
		CP15_SET_DATA_PROT(pu_data_permissions);
	}
	//restore irqs
	*(vu32*)0x04000208 = irq;
	return 0;
}

static u32 verifyFlashSector(u16 secNo, u8* src)
{
	//reading from main memory is safe without changing permissions
	const u32 addr = secNo << 12;
	u8*       pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + addr);
	for (int i = 0; i < (1 << 12); i++)
		if (*pSave++ != *src++)
			return addr + i;
	return 0;
}

static void readFlash(u16 secNo, u32 offset, u8* dst, u32 size)
{
	//reading from main memory is safe without changing permissions
	u8* pSave = (u8*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + (secNo << 12) + offset);
	for (int i = 0; i < size; i++)
		*dst++ = *pSave++;
}

static u16 identifyFlash()
{
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_PROG_SECTOR) = (u32)&programFlashSector;
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_ERASE_CHIP) = (u32)&eraseFlashChip;
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_ERASE_SECTOR) = (u32)&eraseFlashSector;
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_POLLING_SR) = 0;
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_FL_MAXTIME) = (u32)sMaxTime;

	sFlashType.romSize = SAVE_DATA_SIZE;
	sFlashType.sector.size = 0x1000;
	MI_WriteByte(&sFlashType.sector.shift, 12);
	sFlashType.sector.count = sFlashType.sector.size >> sFlashType.sector.shift;
	sFlashType.sector.top = 0;
	sFlashType.agbWait[0] = 0;
	sFlashType.agbWait[1] = 3;
	MI_WriteByte(&sFlashType.makerID, 3);
	MI_WriteByte(&sFlashType.deviceID, 0);
	**(u32**)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_FLASH) = (u32)&sFlashType;
	return 0;
}

bool flash_tryPatch()
{
	sPatchInfo.version = FLASH_VERSION_NONE;
	sPatchInfo.tagAddress = NULL;
	u32* pRom = (u32*)MAIN_MEMORY_ADDRESS_ROM_DATA;
	for (int i = 0; i < ROM_DATA_LENGTH; i += 4)
	{
		if ((pRom[0] == ((u32*)sFlashV123Tag)[0] && pRom[1] == ((u32*)sFlashV123Tag)[1] &&
				pRom[2] == ((u32*)sFlashV123Tag)[2]) ||
			(pRom[0] == ((u32*)sFlashV124Tag)[0] && pRom[1] == ((u32*)sFlashV124Tag)[1] &&
				pRom[2] == ((u32*)sFlashV124Tag)[2]))
		{
			sPatchInfo.version = FLASH_VERSION_V123_V124;
			sPatchInfo.tagAddress = (u32)pRom;
			break;
		}
		pRom++;
	}
	if (sPatchInfo.version == FLASH_VERSION_NONE)
		return false;

	if (sPatchInfo.version == FLASH_VERSION_V123_V124)
	{
		//find the identify flash function
		pRom = (u32*)MAIN_MEMORY_ADDRESS_ROM_DATA;
		bool found = false;
		for (int i = 0; i < ROM_DATA_LENGTH; i += 4)
		{
			if (pRom[0] == ((u32*)sIdentifyFlashV123Sig)[0] && pRom[1] == ((u32*)sIdentifyFlashV123Sig)[1] &&
				pRom[2] == ((u32*)sIdentifyFlashV123Sig)[2] && pRom[3] == ((u32*)sIdentifyFlashV123Sig)[3])
			{
				found = true;
				break;
			}
			pRom++;
		}
		if (!found)
			return false;

		pRom[0] = 0x47104A00; //ldr r2, [pc]; bx r2
		pRom[1] = (u32)&identifyFlash;

		u32* readFunc = (u32*)((*(u32*)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_READ) & ~1) - 0x08000000 +
			MAIN_MEMORY_ADDRESS_ROM_DATA);
		if (((u32)readFunc & 2) != 0)
		{
			*(u16*)readFunc = 0x0000;
			readFunc = (u32*)((u32)readFunc + 2);
		}
		readFunc[0] = 0x00004778; //bx pc; nop
		readFunc[1] = 0xE51FF004; //ldr pc,= address
		readFunc[2] = (u32)&readFlash;

		u32* verifyFunc = (u32*)((*(u32*)(sPatchInfo.tagAddress + FLASH_V123_OFFSET_VERIFY_SECTOR) & ~1) - 0x08000000 +
			MAIN_MEMORY_ADDRESS_ROM_DATA);
		if (((u32)verifyFunc & 2) != 0)
		{
			*(u16*)verifyFunc = 0x0000;
			verifyFunc = (u32*)((u32)verifyFunc + 2);
		}
		verifyFunc[0] = 0x47104A00; //ldr r2, [pc]; bx r2
		verifyFunc[1] = (u32)&verifyFlashSector;
	}

	return false;
}
