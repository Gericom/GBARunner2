#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "sd_access.h"
#include "core/UIManager.h"
#include "core/NtftFont.h"
#include "Toolbar.h"
#include "uiutil.h"
#include "IconPlayCircle_nbfc.h"
#include "IconGamepad_nbfc.h"
#include "IconInformation_nbfc.h"
#include "SettingsCategoryListEntry.h"

u16 SettingsCategoryListEntry::sPlayCircleObjAddr;
u16 SettingsCategoryListEntry::sGamepadObjAddr;
u16 SettingsCategoryListEntry::sInfoObjAddr;

void SettingsCategoryListEntry::LoadCommonData(UIManager& uiManager)
{
	sPlayCircleObjAddr = uiManager.GetSubObjManager().Alloc(IconPlayCircle_nbfc_size);
	for (int i = 0; i < IconPlayCircle_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sPlayCircleObjAddr >> 1) + i] = ((u16*)IconPlayCircle_nbfc)[i];
	sGamepadObjAddr = uiManager.GetSubObjManager().Alloc(IconGamepad_nbfc_size);
	for (int i = 0; i < IconGamepad_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sGamepadObjAddr >> 1) + i] = ((u16*)IconGamepad_nbfc)[i];
	sInfoObjAddr = uiManager.GetSubObjManager().Alloc(IconInformation_nbfc_size);
	for (int i = 0; i < IconInformation_nbfc_size / 2; i++)
	 	SPRITE_GFX_SUB[(sInfoObjAddr >> 1) + i] = ((u16*)IconInformation_nbfc)[i];

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

void SettingsCategoryListEntry::Initialize(UIManager& uiManager)
{
	_nameObjAddr = uiManager.GetSubObjManager().Alloc(192 * 16 / 2);
}

void SettingsCategoryListEntry::Update(UIManager& uiManager)
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
	u16 icon;
	switch (_icon)
	{
		case SETTINGS_CATEGORY_ICON_PLAYCIRCLE:
			icon = sPlayCircleObjAddr;
			break;
		case SETTINGS_CATEGORY_ICON_GAMEPAD:
			icon = sGamepadObjAddr;
			break;
		case SETTINGS_CATEGORY_ICON_INFO:
			icon = sInfoObjAddr;
			break;
	}
	iconOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(4 + palOffset) | (icon >> 5);

	SpriteEntry* nameOams = oamMan.AllocOams(6);
	for (int i = 0; i < 6; i++)
	{
		nameOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE |
			OBJ_Y(_offsetY + 8);
		nameOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 47 + 32 * i);
		nameOams[i].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(3 + palOffset) | ((_nameObjAddr >> 5) + 8 * i);
	}
}

void SettingsCategoryListEntry::VBlank(UIManager& uiManager)
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

void SettingsCategoryListEntry::SetName(const char* name)
{
	int len = strlen(name);
	if (len > 63)
		len = 63;
	arm9_memcpy16((u16*)_name, (u16*)name, (len + 1) >> 1);
	MI_WriteByte(&_name[len], 0);
	_nameInvalidated = true;
}
