#pragma once
class ListEntry;

class ElementHolder
{
	ListEntry* _itemElement;
	int        _itemPosition;
	int        _isBound;
public:
	explicit ElementHolder(ListEntry* itemElement)
		: _itemElement(itemElement), _itemPosition(-1), _isBound(false)
	{
	}

	ListEntry* GetItemElement() const { return _itemElement; }

	int  GetItemPosition() const { return _itemPosition; }
	void SetItemPosition(int position) { _itemPosition = position; }

	bool GetIsBound() const { return _isBound; }
	void SetIsBound(bool bound) { _isBound = bound; }
};

class ListAdapter
{
protected:
	virtual void OnBindElementHolder(ElementHolder* elementHolder, int position) = 0;
public:
	virtual ~ListAdapter()
	{
	}

	virtual ElementHolder* CreateElementHolder() = 0;
	virtual void           DestroyElementHolder(ElementHolder* elementHolder) = 0;

	void BindElementHolder(ElementHolder* elementHolder, int position)
	{
		OnBindElementHolder(elementHolder, position);
		elementHolder->SetItemPosition(position);
		elementHolder->SetIsBound(true);
	}

	virtual int GetItemCount() = 0;
	virtual int GetElementHeight() = 0;
};
