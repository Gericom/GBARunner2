#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "sd_access.h"
#include "core/UIManager.h"
#include "core/NtftFont.h"
#include "Toolbar.h"
#include "uiutil.h"
#include "CheckboxOutline_nbfc.h"
#include "CheckboxMarked_nbfc.h"
#include "SettingsScreen.h"
#include "SettingsItemListEntry.h"

u16 SettingsItemListEntry::sCheckboxOutlineObjAddr;
u16 SettingsItemListEntry::sCheckboxMarkedObjAddr;

void SettingsItemListEntry::LoadCommonData(UIManager& uiManager)
{
	sCheckboxOutlineObjAddr = uiManager.GetSubObjManager().Alloc(CheckboxOutline_nbfc_size);
	for (int i = 0; i < CheckboxOutline_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sCheckboxOutlineObjAddr >> 1) + i] = ((u16*)CheckboxOutline_nbfc)[i];
	sCheckboxMarkedObjAddr = uiManager.GetSubObjManager().Alloc(CheckboxMarked_nbfc_size);
	for (int i = 0; i < CheckboxMarked_nbfc_size / 2; i++)
	 	SPRITE_GFX_SUB[(sCheckboxMarkedObjAddr >> 1) + i] = ((u16*)CheckboxMarked_nbfc)[i];

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

void SettingsItemListEntry::Initialize(UIManager& uiManager)
{
	_titleObjAddr = uiManager.GetSubObjManager().Alloc(192 * 16 / 2);
	_subTitleObjAddr = uiManager.GetSubObjManager().Alloc(224 * 16 / 2);
}

void SettingsItemListEntry::Update(UIManager& uiManager)
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
		bgOams[0].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		bgOams[1].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16);
		bgOams[1].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
		bgOams[1].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		bgOams[2].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16 + 10);
		bgOams[2].attribute[1] = ATTR1_SIZE_64 | OBJ_X(0) | ATTR1_ROTDATA(mtxId);
		bgOams[2].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		bgOams[3].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_offsetY - 16 + 10);
		bgOams[3].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
		bgOams[3].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(7) | (Toolbar::sBgObjAddr >> 5);
		palOffset = 2;
	}

	bool hasSubTitle =
		_settingsItem->mode == SETTINGS_ITEM_MODE_SIMPLE_ENUM ||
		(_settingsItem->subTitle && _settingsItem->subTitle[0] != 0);

	SpriteEntry* titleOams = oamMan.AllocOams(6);
	for (int i = 0; i < 6; i++)
	{
		titleOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE |
			OBJ_Y(_offsetY + (hasSubTitle ? 7 : 13));
		titleOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 10 + 32 * i);
		titleOams[i].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(3 + palOffset) | ((_titleObjAddr >> 5) + 8 * i);
	}

    if (hasSubTitle)
    {
        SpriteEntry* subTitleOams = oamMan.AllocOams(7);
        for (int i = 0; i < 7; i++)
        {
            subTitleOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE |
                OBJ_Y(_offsetY + 20);
            subTitleOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_offsetX + 10 + 32 * i);
            subTitleOams[i].attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(4 + palOffset) | ((_subTitleObjAddr >> 5) + 8 * i);
        }
    }

    if(_settingsItem->mode == SETTINGS_ITEM_MODE_CHECK)
    {
        SpriteEntry* checkboxOam = oamMan.AllocOams(1);
        checkboxOam->attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(_offsetY + 13);
        checkboxOam->attribute[1] = ATTR1_SIZE_16 | OBJ_X(_offsetX + 228);
        checkboxOam->attribute[2] = ATTR2_PRIORITY(3) | ATTR2_PALETTE(4 + palOffset) | 
            ((*_settingsItem->pValue ? sCheckboxMarkedObjAddr : sCheckboxOutlineObjAddr) >> 5);
    }

	if(_settingsItem->mode == SETTINGS_ITEM_MODE_SIMPLE_ENUM && _settingsItem->pValue && _oldValue != *_settingsItem->pValue)
	{
		_subTitleInvalidated = true;
		_oldValue = *_settingsItem->pValue;
	}
}

void SettingsItemListEntry::VBlank(UIManager& uiManager)
{
	if (_titleInvalidated)
	{
		u8* tmp = new u8[192 * 16];
		for (int i = 0; i < 192 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		_titleFont->CreateStringData(_settingsItem->title, tmp + (8 - ((_titleFont->GetFontHeight() + 1) >> 1)) * 192, 192);
		for (int i = 0; i < 6; i++)
			uiutil_convertToObj(tmp + i * 32, 32, 16, 192, &SPRITE_GFX_SUB[(_titleObjAddr >> 1) + i * 128]);
		delete[] tmp;
		_titleInvalidated = false;
	}
    if (_subTitleInvalidated)
	{
		u8* tmp = new u8[224 * 16];
		for (int i = 0; i < 224 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		const char* subTitle;
		if(_settingsItem->mode == SETTINGS_ITEM_MODE_SIMPLE_ENUM)
			subTitle = _settingsItem->enumValueNames[*_settingsItem->pValue];
		else
			subTitle = _settingsItem->subTitle;
		_subTitleFont->CreateStringData(subTitle, tmp + (8 - ((_subTitleFont->GetFontHeight() + 1) >> 1)) * 224, 224);
		for (int i = 0; i < 7; i++)
			uiutil_convertToObj(tmp + i * 32, 32, 16, 224, &SPRITE_GFX_SUB[(_subTitleObjAddr >> 1) + i * 128]);
		delete[] tmp;
		_subTitleInvalidated = false;
	}
}

void SettingsItemListEntry::SetSettingsItem(const settings_item_t* settingsItem) 
{
	_settingsItem = settingsItem;
	_titleInvalidated = true;
	_subTitleInvalidated = true;
	if(_settingsItem->pValue)
		_oldValue = *_settingsItem->pValue;
}