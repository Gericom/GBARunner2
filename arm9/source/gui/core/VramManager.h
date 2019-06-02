#pragma once

class VramManager
{
	u16 _offset;

public:
	VramManager()
		: _offset(0)
	{
		
	}

	u16 Alloc(u16 length)
	{
		u16 result = _offset;
		_offset += length;
		return result;
	}

	u16 GetState() const { return _offset; }
	void SetState(u16 state) { _offset = state; }
};