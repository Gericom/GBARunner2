#pragma once
#include <nds/arm9/sprite.h>
class OamManager
{
	OAMTable _oamTable;
	u16 _oamIdx;
	u16 _mtxIdx;

public:
	OamManager();

	SpriteEntry* AllocOams(int count);
	SpriteRotation* AllocMtxs(int count, int& mtxId);
	void Apply(u16* dst);
	void Clear();
};