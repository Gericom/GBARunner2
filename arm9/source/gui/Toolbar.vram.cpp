#include <nds/arm9/sprite.h>
#include "vram.h"
#include "vramheap.h"
#include "core/UIManager.h"
#include "core/OamManager.h"
#include "core/NtftFont.h"
#include "IconArrowLeft_nbfc.h"
#include "IconSettings_nbfc.h"
#include "uiutil.h"
#include "Toolbar.h"

u16 Toolbar::sBgObjAddr;

void Toolbar::LoadCommonData(UIManager& uiManager)
{
	sBgObjAddr = uiManager.GetSubObjManager().Alloc(1024);
	for (int i = 0; i < 512; i++)
		SPRITE_GFX_SUB[(sBgObjAddr >> 1) + i] = 0x1111;
}

void Toolbar::Initialize(UIManager& uiManager)
{
	u16 backArrowAddr = uiManager.GetSubObjManager().Alloc(128);
	for (int i = 0; i < 128 / 2; i++)
		SPRITE_GFX_SUB[(backArrowAddr >> 1) + i] = ((u16*)IconArrowLeft_nbfc)[i];
	_settingsObjAddr = uiManager.GetSubObjManager().Alloc(128);
	for (int i = 0; i < 128 / 2; i++)
		SPRITE_GFX_SUB[(_settingsObjAddr >> 1) + i] = ((u16*)IconSettings_nbfc)[i];

	_textObjAddr = uiManager.GetSubObjManager().Alloc(160 * 16 / 2);
}

void Toolbar::Update(UIManager& uiManager)
{
	OamManager&     oamMan = uiManager.GetSubOamManager();
	int             mtxId;
	SpriteRotation* bgMtx = oamMan.AllocMtxs(1, mtxId);
	bgMtx->hdx = 128;
	bgMtx->vdx = 0;
	bgMtx->hdy = 0;
	bgMtx->vdy = 128;
	SpriteEntry* bgOams = oamMan.AllocOams(2);
	bgOams[0].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(-28);
	bgOams[0].attribute[1] = ATTR1_SIZE_64 | OBJ_X(0) | ATTR1_ROTDATA(mtxId);
	bgOams[0].attribute[2] = ATTR2_PRIORITY(2) | ATTR2_PALETTE(0) | (sBgObjAddr >> 5);
	bgOams[1].attribute[0] = ATTR0_ROTSCALE_DOUBLE | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(-28);
	bgOams[1].attribute[1] = ATTR1_SIZE_64 | OBJ_X(128) | ATTR1_ROTDATA(mtxId);
	bgOams[1].attribute[2] = ATTR2_PRIORITY(2) | ATTR2_PALETTE(0) | (sBgObjAddr >> 5);

	SpriteEntry* titleOams = oamMan.AllocOams(5);
	int          text_x = 10;
	for (int i = 0; i < 5; i++)
	{
		titleOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		titleOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(text_x + 32 * i);
		titleOams[i].attribute[2] = ATTR2_PRIORITY(2) | ATTR2_PALETTE(1) | ((_textObjAddr >> 5) + 8 * i);
	}

	SpriteEntry* settingsOam = oamMan.AllocOams(1);
	settingsOam->attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(10);
	settingsOam->attribute[1] = ATTR1_SIZE_16 | OBJ_X(_flags.showMenuButton ? 204 : 230);
	settingsOam->attribute[2] = ATTR2_PRIORITY(2) | ATTR2_PALETTE(2) | (_settingsObjAddr >> 5);

	if (_flags.colorInvalidated)
	{
		PaletteManager& palMan = uiManager.GetSubObjPalManager();
		palMan.palette[1] = _bgColor;
		for (int i = 0; i < 16; i++)
		{
			int rnew = _bgColor.r + ((_iconColor.r - _bgColor.r) * i + 7) / 15;
			int gnew = _bgColor.g + ((_iconColor.g - _bgColor.g) * i + 7) / 15;
			int bnew = _bgColor.b + ((_iconColor.b - _bgColor.b) * i + 7) / 15;
			palMan.palette[i + 32].color = RGB5(rnew, gnew, bnew);
		}
		for (int i = 0; i < 16; i++)
		{
			int rnew = _bgColor.r + ((_textColor.r - _bgColor.r) * i + 7) / 15;
			int gnew = _bgColor.g + ((_textColor.g - _bgColor.g) * i + 7) / 15;
			int bnew = _bgColor.b + ((_textColor.b - _bgColor.b) * i + 7) / 15;
			palMan.palette[i + 16].color = RGB5(rnew, gnew, bnew);
		}
		_flags.colorInvalidated = 0;
	}
}

void Toolbar::VBlank(UIManager& uiManager)
{
	if (_flags.titleInvalidated)
	{
		u8* tmp = new u8[160 * 16];
		for (int i = 0; i < 160 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		_font->CreateStringData(_title, tmp + (8 - ((_font->GetFontHeight() + 1) >> 1)) * 160, 160);
		for (int i = 0; i < 5; i++)
			uiutil_convertToObj(tmp + 32 * i, 32, 16, 160, &SPRITE_GFX_SUB[(_textObjAddr >> 1) + 128 * i]);
		delete[] tmp;
		_flags.titleInvalidated = 0;
	}
	/*if (_flags.buttonsInvalidated)
	{
		int text_x;
		if (!_flags.showBackButton)
		{
			OAM_SUB[OAM_BACK_ICON_IDX * 4] = ATTR0_DISABLED;
			text_x = 10;
		}
		else
		{
			OAM_SUB[OAM_BACK_ICON_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(10);
			OAM_SUB[OAM_BACK_ICON_IDX * 4 + 1] = ATTR1_SIZE_16 | OBJ_X(10);
			OAM_SUB[OAM_BACK_ICON_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (16);
			text_x = 47;
		}
		OAM_SUB[OAM_TITLE_TEXT_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		OAM_SUB[OAM_TITLE_TEXT_IDX * 4 + 1] = ATTR1_SIZE_32 | OBJ_X(text_x);
		OAM_SUB[OAM_TITLE_TEXT_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(1) | (16 + 2 + 2 + 1 + 1);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 1) * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 1) * 4 + 1] = ATTR1_SIZE_32 | OBJ_X(text_x + 32);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 1) * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(1) | (16 + 2 + 2 + 1 + 1 + 4);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 2) * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 2) * 4 + 1] = ATTR1_SIZE_32 | OBJ_X(text_x + 2 * 32);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 2) * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(1) | (16 + 2 + 2 + 1 + 1 + 2 * 4);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 3) * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 3) * 4 + 1] = ATTR1_SIZE_32 | OBJ_X(text_x + 3 * 32);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 3) * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(1) | (16 + 2 + 2 + 1 + 1 + 3 * 4);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 4) * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(11);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 4) * 4 + 1] = ATTR1_SIZE_32 | OBJ_X(text_x + 4 * 32);
		OAM_SUB[(OAM_TITLE_TEXT_IDX + 4) * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(1) | (16 + 2 + 2 + 1 + 1 + 4 * 4);
		if (!_flags.showMenuButton)
			OAM_SUB[OAM_MENU_ICON_IDX * 4] = ATTR0_DISABLED;
		else
		{
			OAM_SUB[OAM_MENU_ICON_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_TALL | OBJ_Y(10);
			OAM_SUB[OAM_MENU_ICON_IDX * 4 + 1] = ATTR1_SIZE_8 | OBJ_X(238);
			OAM_SUB[OAM_MENU_ICON_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (16 + 2 + 2);
		}
		if (!_flags.showSearchButton)
			OAM_SUB[OAM_SEARCH_ICON_IDX * 4] = ATTR0_DISABLED;
		else
		{
			OAM_SUB[OAM_SEARCH_ICON_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(10);
			OAM_SUB[OAM_SEARCH_ICON_IDX * 4 + 1] = ATTR1_SIZE_16 | OBJ_X((_flags.showMenuButton ? 204 : 230));
			OAM_SUB[OAM_SEARCH_ICON_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (16 + 2);
		}
		if (!_flags.showClearButton)
			OAM_SUB[OAM_CLEAR_ICON_IDX * 4] = ATTR0_DISABLED;
		else
		{
			OAM_SUB[OAM_CLEAR_ICON_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(10);
			OAM_SUB[OAM_CLEAR_ICON_IDX * 4 + 1] = ATTR1_SIZE_16 | OBJ_X((_flags.showMenuButton ? 204 : 230) - (_flags.showSearchButton ? 32 : 0));
			OAM_SUB[OAM_CLEAR_ICON_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (16 + 2 + 2 + 1 + 1 + 5 * 4);
		}
		_flags.buttonsInvalidated = 0;
	}
	if (_flags.cursorInvalidated)
	{
		if (!_flags.showCursor)
			OAM_SUB[OAM_TEXT_CURSOR_IDX * 4] = ATTR0_DISABLED;
		else
		{
			OAM_SUB[OAM_TEXT_CURSOR_IDX * 4] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_TALL | OBJ_Y(10);
			OAM_SUB[OAM_TEXT_CURSOR_IDX * 4 + 1] = ATTR1_SIZE_8 | OBJ_X((_flags.showBackButton ? 47 : 10) + mCursorX);
			OAM_SUB[OAM_TEXT_CURSOR_IDX * 4 + 2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(3) | (16 + 2 + 2 + 1);
		}
		_flags.cursorInvalidated = 0;
	}*/
}

void Toolbar::SetTitle(const char* title)
{
	int len = strlen(title);
	if (len > 63)
		len = 63;
	arm9_memcpy16((u16*)_title, (u16*)title, (len + 1) >> 1);
	MI_WriteByte(&_title[len], 0);
	_flags.titleInvalidated = 1;
}
