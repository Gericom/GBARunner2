#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "sd_access.h"
#include "core/UIManager.h"
#include "core/NtftFont.h"
#include "ListCircle_nbfc.h"
#include "IconFolder_nbfc.h"
#include "IconGamepadVariant_nbfc.h"
#include "uiutil.h"
#include "FileBrowserListEntry.h"
#include "Toolbar.h"

u16 FileBrowserListEntry::sCircleObjAddr;
u16 FileBrowserListEntry::sFolderObjAddr;
u16 FileBrowserListEntry::sGameObjAddr;

void FileBrowserListEntry::LoadCommonData(UIManager& uiManager)
{
	sCircleObjAddr = uiManager.GetSubObjManager().Alloc(ListCircle_nbfc_size);
	for (int i = 0; i < ListCircle_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sCircleObjAddr >> 1) + i] = ((u16*)ListCircle_nbfc)[i];
	sFolderObjAddr = uiManager.GetSubObjManager().Alloc(IconFolder_nbfc_size);
	for (int i = 0; i < IconFolder_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sFolderObjAddr >> 1) + i] = ((u16*)IconFolder_nbfc)[i];
	sGameObjAddr = uiManager.GetSubObjManager().Alloc(IconGamepadVariant_nbfc_size);
	for (int i = 0; i < IconGamepadVariant_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sGameObjAddr >> 1) + i] = ((u16*)IconGamepadVariant_nbfc)[i];

	PaletteManager& palMan = uiManager.GetSubObjPalManager();
	for (int i = 0; i < 16; i++)
	{
		int gray = 31 + ((4 - 31) * i) / 15;
		palMan.palette[i + (3 * 16)].color = RGB5(gray, gray, gray);
		int gray2 = 28 + ((4 - 28) * i) / 15;
		palMan.palette[i + (6 * 16)].color = RGB5(gray2, gray2, gray2);
	}
	for (int i = 0; i < 16; i++)
	{
		int gray = 31 + ((19 - 31) * i) / 15;
		palMan.palette[i + (4 * 16)].color = RGB5(gray, gray, gray);
		int gray2 = 28 + ((19 - 28) * i) / 15;
		palMan.palette[i + (7 * 16)].color = RGB5(gray2, gray2, gray2);
	}
	for (int i = 0; i < 16; i++)
	{
		int gray = 19 + ((31 - 19) * i) / 15;
		palMan.palette[i + (5 * 16)].color = RGB5(gray, gray, gray);
	}
	palMan.palette[1 + (8 * 16)].color = RGB5(28, 28, 28);
}

void FileBrowserListEntry::Initialize(UIManager& uiManager)
{
	_nameObjAddr = uiManager.GetSubObjManager().Alloc(192 * 16 / 2);
}

void FileBrowserListEntry::Update(UIManager& uiManager)
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
		SpriteEntry* bgOams = oamMan.AllocOams(4);
		bgOams[0].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16);
		bgOams[0].attribute[1] = ATTR1_SIZE_64 | OBJ_X(0) | ATTR1_ROTDATA(mtxId);
		bgOams[0].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(8) | (Toolbar::sBgObjAddr >> 5);
		bgOams[1].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16);
		bgOams[1].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
		bgOams[1].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(8) | (Toolbar::sBgObjAddr >> 5);
		bgOams[2].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16 + 4);
		bgOams[2].attribute[1] = ATTR1_SIZE_64 | OBJ_X(0) | ATTR1_ROTDATA(mtxId);
		bgOams[2].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(8) | (Toolbar::sBgObjAddr >> 5);
		bgOams[3].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16 + 4);
		bgOams[3].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
		bgOams[3].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(8) | (Toolbar::sBgObjAddr >> 5);
		palOffset = 3;
	}

	SpriteEntry* circleOam = oamMan.AllocOams(1);
	circleOam->attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(_offsetY + 5);
	circleOam->attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 10);
	circleOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(4 + palOffset) | (sCircleObjAddr >> 5);

	SpriteEntry* iconOam = oamMan.AllocOams(1);
	iconOam->attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(_offsetY + 10);
	iconOam->attribute[1] = ATTR1_SIZE_16 | OBJ_X(_offsetX + 15);
	if (_entryType == FILE_BROWSER_ENTRY_TYPE_FOLDER)
		iconOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(5) | (sFolderObjAddr >> 5);
	else if (_entryType == FILE_BROWSER_ENTRY_TYPE_GAME)
		iconOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(5) | (sGameObjAddr >> 5);

	SpriteEntry* nameOams = oamMan.AllocOams(6);
	for (int i = 0; i < 6; i++)
	{
		nameOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE |
			OBJ_Y(_offsetY + 10);
		nameOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 47 + 32 * i);
		nameOams[i].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(3 + palOffset) | ((_nameObjAddr >> 5) + 8 * i);
	}
}

void FileBrowserListEntry::VBlank(UIManager& uiManager)
{
	if (_nameInvalidated)
	{
		int w, h;
		_font->MeasureString(_name, w, h);
		u8* tmp = new u8[192 * 16];
		for (int i = 0; i < 192 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		_font->CreateStringData(_name, tmp + (8 - ((h + 1) >> 1)) * 192, 192);
		for (int i = 0; i < 6; i++)
			uiutil_convertToObj(tmp + i * 32, 32, 16, 192, &SPRITE_GFX_SUB[(_nameObjAddr >> 1) + i * 128]);
		delete[] tmp;
		_nameInvalidated = false;
	}
}

void FileBrowserListEntry::SetName(const char* name)
{
	int len = strlen(name);
	if (len > 63)
		len = 63;
	arm9_memcpy16((u16*)_name, (u16*)name, (len + 1) >> 1);
	MI_WriteByte(&_name[len], 0);
	_nameInvalidated = true;
}
