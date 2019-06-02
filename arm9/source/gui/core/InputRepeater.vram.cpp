#include "vram.h"
#include "InputRepeater.h"

void InputRepeater::Update(u16 keyData)
{
	_trigKeys = keyData & (keyData ^ _curKeys);
	_relKeys = ~keyData & (keyData ^ _curKeys);
	_curKeys = keyData;
	_repKeys = 0;
	if (_state)
	{
		if (_state == 1)
		{
			if (keyData & _mask)
			{
				_frameCounter++;
				if (_frameCounter >= _firstFrame)
				{
					_state = 2;
					_frameCounter = 0;
					_repKeys = keyData & _mask;
				}
			}
			else
				_state = 0;
		}
		else if (_state == 2)
		{
			if (keyData & _mask)
			{
				_frameCounter++;
				if (_frameCounter >= _nextFrame)
				{
					_frameCounter = 0;
					_repKeys = keyData & _mask;
				}
			}
			else
				_state = 0;
		}
	}
	else if (keyData & _mask)
	{
		_state = 1;
		_frameCounter = 0;
		_repKeys = keyData & _mask;
	}
}
