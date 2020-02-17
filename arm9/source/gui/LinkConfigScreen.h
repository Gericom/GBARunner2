#pragma once

class LinkConfigScreen
{
    UIContext* _uiContext;

	ListRecycler* _listRecycler;
	ListAdapter* _adapter;

	InputRepeater _inputRepeater;

	int _selectedEntry;

    u16 _vramState;

public:
    LinkConfigScreen(UIContext* uiContext)
        : _uiContext(uiContext), _listRecycler(NULL), _adapter(NULL), _inputRepeater(0x3F3, 20, 8), _selectedEntry(0)
	{
	}

	~LinkConfigScreen()
	{
		if (_listRecycler)
			delete _listRecycler;
		if (_adapter)
			delete _adapter;
	}

	void Run();
};