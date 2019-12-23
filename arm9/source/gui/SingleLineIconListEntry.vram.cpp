#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "sd_access.h"
#include "core/UIManager.h"
#include "core/NtftFont.h"
#include "Toolbar.h"
#include "uiutil.h"
#include "SingleLineIconListEntry.h"

void SingleLineIconListEntry::LoadCommonData(UIManager& uiManager)
{
	PaletteManager& palMan = uiManager.GetSubObjPalManager();
	for (int i = 0; i < 16; i++)
	{
		int gray = 31 + ((4 - 31) * i) / 15;
		palMan.palette[i + (3 * 16)].color = RGB5(gray, gray, gray);
		int gray2 = 28 + ((4 - 28) * i) / 15;
		palMan.palette[i + (5 * 16)].color = RGB5(gray2, gray2, gray2);
	}
	for (int i = 0; i < 16; i++)
	{
		int gray = 31 + ((14 - 31) * i) / 15;
		palMan.palette[i + (4 * 16)].color = RGB5(gray, gray, gray);
		int gray2 = 28 + ((13 - 28) * i) / 15;
		palMan.palette[i + (6 * 16)].color = RGB5(gray2, gray2, gray2);
	}
	palMan.palette[1 + (7 * 16)].color = RGB5(28, 28, 28);
}

void SingleLineIconListEntry::Initialize(UIManager& uiManager)
{
	_nameObjAddr = uiManager.GetSubObjManager().Alloc(192 * 16 / 2);
}

void SingleLineIconListEntry::Update(UIManager& uiManager)
{
	OamManager&     oamMan = uiManager.GetSubOamManager();
	int palOffset = 0;
	if (_selected)
	{
		int             mtxId;
		SpriteRotation* bgMtx = oamMan.AllocMtxs(1, mtxId);
		bgMtx->hdx = 128;
		bgMtx->vdx = 0;
		bgMtx->hdy = 0;
		bgMtx->vdy = 256;
		SpriteEntry* bgOams = oamMan.AllocOams(2);
		bgOams[0].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16);
		bgOams[0].attribute[1] = ATTR1_SIZE_64 | OBJ_X(0) | ATTR1_ROTDATA(mtxId);
		bgOams[0].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		bgOams[1].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16);
		bgOams[1].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
		bgOams[1].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		palOffset = 2;
	}

	SpriteEntry* iconOam = oamMan.AllocOams(1);
	iconOam->attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(_offsetY + 8);
	iconOam->attribute[1] = ATTR1_SIZE_16 | OBJ_X(_offsetX + 10);
	iconOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(4 + palOffset) | (_iconObjAddr >> 5);

	SpriteEntry* nameOams = oamMan.AllocOams(6);
	for (int i = 0; i < 6; i++)
	{
		nameOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE |
			OBJ_Y(_offsetY + 8);
		nameOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 47 + 32 * i);
		nameOams[i].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(3 + palOffset) | ((_nameObjAddr >> 5) + 8 * i);
	}
}

void SingleLineIconListEntry::VBlank(UIManager& uiManager)
{
	if (_nameInvalidated)
	{
		u8* tmp = new u8[192 * 16];
		for (int i = 0; i < 192 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		_font->CreateStringData(_name, tmp + (8 - ((_font->GetFontHeight() + 1) >> 1)) * 192, 192);
		for (int i = 0; i < 6; i++)
			uiutil_convertToObj(tmp + i * 32, 32, 16, 192, &SPRITE_GFX_SUB[(_nameObjAddr >> 1) + i * 128]);
		delete[] tmp;
		_nameInvalidated = false;
	}
}

void SingleLineIconListEntry::SetName(const char* name)
{
	int len = strlen(name);
	if (len > 63)
		len = 63;
	arm9_memcpy16((u16*)_name, (u16*)name, (len + 1) >> 1);
	MI_WriteByte(&_name[len], 0);
	_nameInvalidated = true;
}
