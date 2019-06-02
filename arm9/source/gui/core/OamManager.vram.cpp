#include <nds/arm9/sprite.h>
#include "vram.h"
#include "sd_access.h"
#include "OamManager.h"

OamManager::OamManager()
{
	Clear();
}

SpriteEntry* OamManager::AllocOams(int count)
{
	_oamIdx -= count;
	return &_oamTable.oamBuffer[_oamIdx];
}

SpriteRotation* OamManager::AllocMtxs(int count, int& mtxId)
{
	mtxId = _mtxIdx;
	SpriteRotation* result = &_oamTable.matrixBuffer[_mtxIdx];
	_mtxIdx += count;
	return result;
}

void OamManager::Apply(u16* dst)
{
	arm9_memcpy16(dst, (u16*)&_oamTable, sizeof(OAMTable) >> 1);
}

void OamManager::Clear()
{
	for (int i = 0; i < (sizeof(OAMTable) >> 1); i++)
		((u16*)&_oamTable)[i] = 0xC0C0;
	_oamIdx = 128;
	_mtxIdx = 0;
}