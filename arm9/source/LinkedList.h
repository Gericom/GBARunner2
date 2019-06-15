#pragma once
#include "vram.h"

template <class T>
class LinkedListEntry
{
public:
	struct
	{
		T* prev;
		T* next;
	}      link;

	LinkedListEntry()
		: link({ NULL, NULL })
	{
	}
};


template <class T>
class LinkedList
{
	T*  _head;
	T*  _tail;
	int _count;

public:
	LinkedList()
		: _head(NULL), _tail(NULL), _count(0)
	{
	}

	void Append(T* object)
	{
		object->link.prev = _tail;
		object->link.next = NULL;
		if (_tail)
			_tail->link.next = object;
		_tail = object;
		if (!_head)
			_head = object; //object also is head when the list is empty
		_count++;
	}

	void Prepend(T* object)
	{
		object->link.prev = NULL;
		object->link.next = _head;
		if (_head)
			_head->link.prev = object;
		_head = object;
		if (!_tail)
			_tail = object; //object also is tail when the list is empty
		_count++;
	}

	void InsertBefore(T* object, T* target)
	{
		object->link.prev = target->link.prev;
		target->link.prev = object;
		object->link.next = target;
		if (object->link.next)
			object->link.next->link.next = object;
		_count++;
	}

	void Remove(T* object, bool free = false)
	{
		if (!object->link.prev)
			_head = object->link.next;
		else
			object->link.prev->link.next = object->link.next;

		if (!object->link.next)
			_tail = object->link.prev;
		else
			object->link.next->link.prev = object->link.prev;
		object->link.prev = NULL;
		object->link.next = NULL;
		_count--;
		if (free)
			delete object;
	}

	T* GetNext(T* object) const
	{
		return !object ? _head : object->link.next;
	}

	T* GetPrev(T* object) const
	{
		return !object ? _tail : object->link.prev;
	}

	T* operator[](int idx) const
	{
		int i = 0;
		T*  entry = NULL;
		while ((entry = GetNext(entry)) != NULL)
			if (idx == i++)
				return entry;
		return NULL;
	}

	void Clear(bool free = false)
	{
		while (_head)
			Remove(_head, free);
	}

	int GetLength() const { return _count; }
};