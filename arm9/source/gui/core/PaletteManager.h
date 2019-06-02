#pragma once
#include "gui/uiutil.h"

class PaletteManager
{
	u16 _dimmedRows;

public:
	XBGR1555 palette[256];

	PaletteManager()
		: _dimmedRows(0)
	{
		Clear();
	}

	void Clear();
	void Apply(u16* dst);

	void DimRows(u16 rows)
	{
		_dimmedRows |= rows;
	}

	void UndimRows(u16 rows)
	{
		_dimmedRows &= ~rows;
	}
};
