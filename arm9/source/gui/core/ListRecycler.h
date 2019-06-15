#pragma once
#include "UIElement.h"
#include "ListAdapter.h"
#include "math.h"

class ListRecycler : public UIElement
{
	int _x;
	int _y;
	int _width;
	int _height;
	int _paddingY;

	int _scrollY;

	int _elementHeight;

	int             _holderCount;
	ElementHolder** _holders;

	int _firstElement;
	int _lastElement;

	ListAdapter* _adapter;

	int _selectedElement;

	ElementHolder* GetUnboundPaneHolder() const;
	void           SetupListItem(int position) const;
	void           UpdateElementPositions();

public:
	ListRecycler(int x, int y, int width, int height, int paddingY, ListAdapter* adapter);

	void EnsureElementVisible(int idx);

	~ListRecycler();

	void Initialize(UIManager& uiManager);
	void Update(UIManager& uiManager);
	void VBlank(UIManager& uiManager);

	ListAdapter* GetAdapter() const { return _adapter; }

	void SetSelectedIdx(int idx);
};
