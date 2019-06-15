#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "string.h"
#include "core/UIManager.h"
#include "Dialog.h"

void Dialog::Initialize(UIManager& uiManager)
{
	_titleTextObjAddr = uiManager.GetSubObjManager().Alloc(160 * 16 / 2);
	_bodyTextObjAddr = uiManager.GetSubObjManager().Alloc(160 * 32 / 2);
	//create bg map
	int tileX = _width >> 3;
	int tileY = _height >> 3;
	for(int y = 0; y < 24; y++)
	{
		for(int x = 0; x < 32; x++)
		{
			u16 mapEntry;
			if (x == 0 && y == 0)
				mapEntry = (15 << 12) | 2;
			else if (x == tileX - 1 && y == 0)
				mapEntry = (15 << 12) | (1 << 10) | 2;
			else if (x == 0 && y == tileY - 1)
				mapEntry = (15 << 12) | (1 << 11) | 2;
			else if (x == tileX - 1 && y == tileY - 1)
				mapEntry = (15 << 12) | (1 << 11) | (1 << 10) | 2;
			else if (x >= tileX || y >= tileY)
				mapEntry = 0;
			else
				mapEntry = (15 << 12) | 3;
			BG_GFX_SUB[(0x4000 >> 1) + y * 32 + x] = mapEntry;
		}
	}
	BG_OFFSET_SUB[2].x = -_x;
	BG_OFFSET_SUB[2].y = -_y;
	//BG_GFX_SUB[(0x3800 >> 1) + i] = ((u16*)BarShadow_nbfs)[i];
}

void Dialog::Update(UIManager& uiManager)
{
	OamManager&  oamMan = uiManager.GetSubOamManager();
	SpriteEntry* titleOams = oamMan.AllocOams(5);
	for (int i = 0; i < 5; i++)
	{
		titleOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_WIDE | OBJ_Y(_y + 14);
		titleOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_x + 16 + 32 * i);
		titleOams[i].attribute[2] = ATTR2_PRIORITY(1) | ATTR2_PALETTE(14) | ((_titleTextObjAddr >> 5) + 8 * i);
	}

	SpriteEntry* bodyOams = oamMan.AllocOams(5);
	for (int i = 0; i < 5; i++)
	{
		bodyOams[i].attribute[0] = ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE | OBJ_Y(_y + 38);
		bodyOams[i].attribute[1] = ATTR1_SIZE_32 | OBJ_X(_x + 16 + 32 * i);
		bodyOams[i].attribute[2] = ATTR2_PRIORITY(1) | ATTR2_PALETTE(15) | ((_bodyTextObjAddr >> 5) + 16 * i);
	}

	if (_flags.colorInvalidated)
	{
		PaletteManager& palMan = uiManager.GetSubObjPalManager();
		//SPRITE_PALETTE_SUB[1] = _bgColor.color;
		for (int i = 0; i < 16; i++)
		{
			int rnew = _bgColor.r + ((_titleTextColor.r - _bgColor.r) * i + 7) / 15;
			int gnew = _bgColor.g + ((_titleTextColor.g - _bgColor.g) * i + 7) / 15;
			int bnew = _bgColor.b + ((_titleTextColor.b - _bgColor.b) * i + 7) / 15;
			palMan.palette[i + 14 * 16].color = RGB5(rnew, gnew, bnew);
		}
		for (int i = 0; i < 16; i++)
		{
			int rnew = _bgColor.r + ((_bodyTextColor.r - _bgColor.r) * i + 7) / 15;
			int gnew = _bgColor.g + ((_bodyTextColor.g - _bgColor.g) * i + 7) / 15;
			int bnew = _bgColor.b + ((_bodyTextColor.b - _bgColor.b) * i + 7) / 15;
			palMan.palette[i + 15 * 16].color = RGB5(rnew, gnew, bnew);
		}
		_flags.colorInvalidated = 0;
	}
}

void Dialog::VBlank(UIManager& uiManager)
{
	if (_flags.titleInvalidated)
	{
		u8* tmp = new u8[160 * 16];
		for (int i = 0; i < 160 * 16; i += 2)
			*((u16*)&tmp[i]) = 0;
		_titleFont->CreateStringData(_titleText, tmp + (8 - ((_titleFont->GetFontHeight() + 1) >> 1)) * 160, 160);
		for (int i = 0; i < 5; i++)
			uiutil_convertToObj(tmp + 32 * i, 32, 16, 160, &SPRITE_GFX_SUB[(_titleTextObjAddr >> 1) + 128 * i]);
		delete[] tmp;
		_flags.titleInvalidated = 0;
	}
	if (_flags.bodyInvalidated)
	{
		u8* tmp = new u8[160 * 32];
		for (int i = 0; i < 160 * 32; i += 2)
			*((u16*)&tmp[i]) = 0;
		_bodyFont->CreateStringData(_bodyText, tmp + (8 - ((_bodyFont->GetFontHeight() + 1) >> 1)) * 160, 160);
		for (int i = 0; i < 5; i++)
			uiutil_convertToObj(tmp + 32 * i, 32, 32, 160, &SPRITE_GFX_SUB[(_bodyTextObjAddr >> 1) + 256 * i]);
		delete[] tmp;
		_flags.bodyInvalidated = 0;
	}
}

void Dialog::SetTitleText(const char* titleText)
{
	int len = strlen(titleText);
	if (len > 63)
		len = 63;
	arm9_memcpy16((u16*)_titleText, (u16*)titleText, (len + 1) >> 1);
	MI_WriteByte(&_titleText[len], 0);
	_flags.titleInvalidated = true;
}

void Dialog::SetBodyText(const char* bodyText)
{
	int len = strlen(bodyText);
	if (len > 255)
		len = 255;
	arm9_memcpy16((u16*)_bodyText, (u16*)bodyText, (len + 1) >> 1);
	MI_WriteByte(&_bodyText[len], 0);
	_flags.titleInvalidated = true;
}
