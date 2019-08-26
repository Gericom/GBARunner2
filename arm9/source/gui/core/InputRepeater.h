#pragma once

class InputRepeater
{
	u16 _trigKeys;
	u16 _curKeys;
	u16 _relKeys;
	u16 _repKeys;
	u16 _state;
	u16 _frameCounter;
	u16 _mask;
	u16 _firstFrame;
	u16 _nextFrame;
public:
	InputRepeater(u16 mask, u16 firstFrame, u16 nextFrame)
		: _trigKeys(0), _curKeys(0), _relKeys(0), _repKeys(0), _state(0), _frameCounter(0), _mask(mask),
		  _firstFrame(firstFrame), _nextFrame(nextFrame)
	{
	}

	void Update(u16 keyData);

	u16 GetTriggeredKeys() const { return _trigKeys | _repKeys; }
};
