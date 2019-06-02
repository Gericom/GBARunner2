#include "vram.h"
#include "ListEntry.h"
#include "ListRecycler.h"

ListRecycler::ListRecycler(int x, int y, int width, int height, int paddingY, ListAdapter* adapter)
	: _x(x), _y(y), _width(width), _height(height), _paddingY(paddingY), _scrollY(0), _adapter(adapter),
	  _selectedElement(-1)
{
	_elementHeight = _adapter->GetElementHeight();
	_holderCount = 2 + math_div(_height, _elementHeight);
	_holders = new ElementHolder*[_holderCount];
	for (int i = 0; i < _holderCount; i++)
		_holders[i] = _adapter->CreateElementHolder();
	int visibleItemCount = math_div(_height + (_elementHeight - 1), _elementHeight);
	int count = _adapter->GetItemCount();
	if (count < visibleItemCount)
		visibleItemCount = count;
	for (int i = 0; i < visibleItemCount; i++)
		SetupListItem(i);
	//_scrollYMax = _listItemBaseOffsetY + _listAdapter->GetItemCount() * _listItemHeight - _pane->GetHeight();
	_firstElement = 0;
	_lastElement = visibleItemCount - 1;
}

void ListRecycler::EnsureElementVisible(int idx)
{
	int pos = _paddingY + idx * _elementHeight + _scrollY;
	if(pos < _paddingY)
	{
		//scroll in from top
		_scrollY -= pos - _paddingY;
		UpdateElementPositions();
	}
	else if(pos + _elementHeight > _height)
	{
		//scroll in from bottom
		_scrollY -= (pos + _elementHeight) - _height;
		UpdateElementPositions();
	}
}

ElementHolder* ListRecycler::GetUnboundPaneHolder() const
{
	for (int i = 0; i < _holderCount; i++)
		if (!_holders[i]->GetIsBound())
			return _holders[i];
	while (1);
	//OS_Panic("Pane underrun!");
	return NULL;
}

void ListRecycler::SetupListItem(int position) const
{
	ElementHolder* h = GetUnboundPaneHolder();
	_adapter->BindElementHolder(h, position);
	h->GetItemElement()->SetOffset(_x, _y + _paddingY + position * _elementHeight + _scrollY);
	//h->GetItemPane()->SetVisible(true);
}

void ListRecycler::UpdateElementPositions()
{
	for (int i = 0; i < _holderCount; i++)
	{
		if (!_holders[i]->GetIsBound())
			continue;
		int newY = _paddingY + _holders[i]->GetItemPosition() * _elementHeight + _scrollY;
		if (newY + _elementHeight < 0 || newY > _height)
		{
			if (_holders[i]->GetItemPosition() == _firstElement)
				_firstElement++;
			else
				_lastElement--;
			//_holders[i]->GetItemElement()->SetVisible(false);
			_holders[i]->SetIsBound(false);
			continue;
		}
		_holders[i]->GetItemElement()->SetOffset(_x, _y + newY);
	}
	int firstY = _paddingY + _firstElement * _elementHeight + _scrollY;
	while (_firstElement > 0 && firstY > 0)
	{
		SetupListItem(--_firstElement);
		firstY = _paddingY + _firstElement * _elementHeight + _scrollY;
	}
	int lastY = _paddingY + _lastElement * _elementHeight + _scrollY;
	while (_lastElement < _adapter->GetItemCount() - 1 && lastY + _elementHeight < _height)
	{
		SetupListItem(++_lastElement);
		lastY = _paddingY + _lastElement * _elementHeight + _scrollY;
	}
}

void ListRecycler::Initialize(UIManager& uiManager)
{
	for (int i = 0; i < _holderCount; i++)
		_holders[i]->GetItemElement()->Initialize(uiManager);
}

void ListRecycler::Update(UIManager& uiManager)
{
	for (int i = 0; i < _holderCount; i++)
	{
		if (!_holders[i]->GetIsBound())
			continue;
		_holders[i]->GetItemElement()->SetSelected(_holders[i]->GetItemPosition() == _selectedElement);
		_holders[i]->GetItemElement()->Update(uiManager);
	}
}

void ListRecycler::VBlank(UIManager& uiManager)
{
	for (int i = 0; i < _holderCount; i++)
	{
		if (!_holders[i]->GetIsBound())
			continue;
		_holders[i]->GetItemElement()->VBlank(uiManager);
	}
}

void ListRecycler::SetSelectedIdx(int idx)
{
	_selectedElement = idx;
	EnsureElementVisible(idx);
}
