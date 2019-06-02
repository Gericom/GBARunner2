#pragma once

#include "core/UIElement.h"
#include "string.h"
#include "sd_access.h"
#include "uiutil.h"
class UIManager;

class NtftFont;

class Toolbar : public UIElement
{
	XBGR1555 _bgColor;
	XBGR1555 _iconColor;
	XBGR1555 _textColor;
	u16      _textObjAddr;
	u16      _settingsObjAddr;

	union
	{
		struct
		{
			vu32 colorInvalidated : 1;
			vu32 titleInvalidated : 1;
			vu32 buttonsInvalidated : 1;
			vu32 cursorInvalidated : 1;
			vu32 : 12;
			vu32 showBackButton : 1;
			vu32 showClearButton : 1;
			vu32 showSearchButton : 1;
			vu32 showMenuButton : 1;
			vu32 showCursor : 1;
			vu32 : 11;
		};

		vu32 flags;
	}        _flags;

	const NtftFont* _font;
	char            _title[64];
public:
	static u16 sBgObjAddr;
	static void LoadCommonData(UIManager& uiManager);

	Toolbar(u16 bgColor, u16 iconColor, const NtftFont* font, const char* title, u16 textColor)
		: _font(font)
	{
		_flags.flags = 0;
		SetBGColor(bgColor);
		SetIconColor(iconColor);
		SetTextColor(textColor);
		SetTitle(title);
	}

	void Initialize(UIManager& uiManager);
	void Update(UIManager&     uiManager);
	void VBlank(UIManager&     uiManager);

	void SetBGColor(u16 color)
	{
		_bgColor.color = color;
		_flags.colorInvalidated = 1;
	}

	void SetIconColor(u16 color)
	{
		_iconColor.color = color;
		_flags.colorInvalidated = 1;
	}

	void SetTextColor(u16 color)
	{
		_textColor.color = color;
		_flags.colorInvalidated = 1;
	}

	void SetFont(const NtftFont* font)
	{
		_font = font;
		_flags.titleInvalidated = true;
	}

	void SetTitle(const char* title);
};
