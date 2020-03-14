#include "vram.h"
#include "sd_access.h"
#include "fat/ff.h"
#include "patchUtil.h"
#include "bios.h"

extern "C" void bios_cpuset_cache_patch();
extern "C" void bios_cpufastset_cache_patch();
extern "C" void bios_softResetPatch();
extern "C" void bios_swiPatch();

/**
 * \brief Relocates the gba bios so it can be executed from vram
 */
static void applyRelocation()
{
	const u32 base = (u32)gGbaBios;
	//swi table
	for (int i = 0; i < 43; i++)
		gGbaBios[(0x01C8 >> 2) + i] += base;
	gGbaBios[0x027C >> 2] += base;
	gGbaBios[0x0AB8 >> 2] += base;
	gGbaBios[0x0ABC >> 2] += base;
	gGbaBios[0x0AC4 >> 2] += base;
	gGbaBios[0x0ACC >> 2] += base;
	gGbaBios[0x0ADC >> 2] += base;
	gGbaBios[0x0AE0 >> 2] += base;
	gGbaBios[0x0AE4 >> 2] += base;
	gGbaBios[0x0AEC >> 2] += base;
	gGbaBios[0x0AF0 >> 2] += base;
	gGbaBios[0x0B0C >> 2] += base;
	gGbaBios[0x0B2C >> 2] += base;
	gGbaBios[0x1430 >> 2] += base;
	gGbaBios[0x16F8 >> 2] += base;
	gGbaBios[0x16FC >> 2] += base;
	gGbaBios[0x1700 >> 2] += base;
	gGbaBios[0x1788 >> 2] += base;
	gGbaBios[0x1924 >> 2] += base;
	gGbaBios[0x1D64 >> 2] += base;
	gGbaBios[0x1D6C >> 2] += base;
	gGbaBios[0x1D80 >> 2] += base;
	gGbaBios[0x1D84 >> 2] += base;
	gGbaBios[0x1D90 >> 2] += base;
	gGbaBios[0x1D9C >> 2] += base;
	gGbaBios[0x23A4 >> 2] += base;
	gGbaBios[0x2624 >> 2] += base;
	gGbaBios[0x26C0 >> 2] += base;
	gGbaBios[0x2C14 >> 2] += base;
	gGbaBios[0x30AC >> 2] += base;
	*(vu16*)(((u8*)gGbaBios) + 0x868) = 0;
	for (int i = 0; i < 38; i++)
		gGbaBios[(0x3738 >> 2) + i] += base;
	gGbaBios[0x37D4 >> 2] += base;
	gGbaBios[0x37D8 >> 2] += base;
	gGbaBios[0x37E0 >> 2] += base;
	gGbaBios[0x37E4 >> 2] += base;
	gGbaBios[0x381C >> 2] += base;
	gGbaBios[0x3820 >> 2] += base;
	gGbaBios[0x3824 >> 2] += base;
	gGbaBios[0x3828 >> 2] += base;
	gGbaBios[0x38A0 >> 2] += base;
	gGbaBios[0x38A4 >> 2] += base;
	gGbaBios[0x38A8 >> 2] += base;
	gGbaBios[0x38AC >> 2] += base;
	gGbaBios[0x38B0 >> 2] += base;
	gGbaBios[0x390C >> 2] += base;
	gGbaBios[0x3910 >> 2] += base;
	gGbaBios[0x3914 >> 2] += base;
	gGbaBios[0x3918 >> 2] += base;
	gGbaBios[0x391C >> 2] += base;
	gGbaBios[0x3920 >> 2] += base;
	gGbaBios[0x3924 >> 2] += base;
	gGbaBios[0x3984 >> 2] += base;
	gGbaBios[0x3988 >> 2] += base;
	gGbaBios[0x398C >> 2] += base;
	gGbaBios[0x3990 >> 2] += base;
	gGbaBios[0x3994 >> 2] += base;
	gGbaBios[0x3998 >> 2] += base;
	gGbaBios[0x399C >> 2] += base;
	gGbaBios[0x39C4 >> 2] += base;
	gGbaBios[0x39C8 >> 2] += base;
	gGbaBios[0x39CC >> 2] += base;
	//Setup the reset and swi vectors, gcc is annoying, so use a mirrored address
	//wanted to do an endless loop, but for whatever reason the bios intro sound does not work unless this is 0
	//*(vu32*)0x01000000 = 0x00000000;// 0xEAFFFFFE; //b .
	//*(vu32*)0x01000008 = 0xEAFFFFFE;
	*(vu32*)0x01000008 = 0xE3A0F51A; //mov pc, #bios_swiVeneer
}

/**
 * \brief Applies a couple of patches to the bios
 */
static void applyPatches()
{
	//patch for having the right protected op at boot
	gGbaBios[0xDC >> 2] = pcu_makeArmBranch((u32)&gGbaBios[0xDC >> 2], (u32)&bios_softResetPatch);

	//patch for having the right protected op after swi
	gGbaBios[0x184 >> 2] = pcu_makeArmBranch((u32)&gGbaBios[0x184 >> 2], (u32)&bios_swiPatch);

	//fix post boot redirect
	//todo: maybe I should correctly implement that register instead
	gGbaBios[0x74 >> 2] = 0;

	//fix sound bias hang
	*(vu16*)(((u8*)gGbaBios) + 0x800) = 0x4770;

	//replace the write to haltcnt in waitintr with a cp15 instruction that does the same on arm9
	gGbaBios[0x1B0 >> 2] = 0xEE070F90; //mcr p15,0,r0,c7,c0,4
	gGbaBios[0x344 >> 2] = 0xEE070F90; //mcr p15,0,r0,c7,c0,4

#ifdef ENABLE_WRAM_ICACHE
	//add some cache invalidation to the copy functions
	gGbaBios[0x1F4 >> 2] = (u32)&bios_cpuset_cache_patch;
	gGbaBios[0x1F8 >> 2] = (u32)&bios_cpufastset_cache_patch;
#endif
}


/**
 * \brief Loads the bios from 0:/bios.bin or 0:/gba/bios.bin to gGbaBios
 *		  and relocates it so it can be executed from there
 * \return BIOS_LOAD_RESULT_OK if loading was successful or an error code otherwise
 */
BiosLoadResult bios_load()
{
	FRESULT result = f_open(&vram_cd->fil, "0:/bios.bin", FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
		result = f_open(&vram_cd->fil, "0:/gba/bios.bin", FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
		result = f_open(&vram_cd->fil, "0:/_gba/bios.bin", FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
		return BIOS_LOAD_RESULT_NOT_FOUND;
	if (vram_cd->fil.obj.objsize != BIOS_SIZE)
	{
		f_close(&vram_cd->fil);
		return BIOS_LOAD_RESULT_INVALID;
	}
	UINT br;
	if (f_read(&vram_cd->fil, (void*)vram_cd->cluster_cache, BIOS_SIZE, &br) != FR_OK || br != BIOS_SIZE)
	{
		f_close(&vram_cd->fil);
		return BIOS_LOAD_RESULT_ERROR;
	}
	f_close(&vram_cd->fil);
	//arm9_memcpy16((u16*)0, (u16*)vram_cd->cluster_cache, BIOS_SIZE >> 1);

	arm9_memcpy16((u16*)((u8*)gGbaBios + 8), (u16*)(vram_cd->cluster_cache + 8), (BIOS_SIZE - 8) >> 1);
	applyRelocation();
	applyPatches();
	return BIOS_LOAD_RESULT_OK;
}
