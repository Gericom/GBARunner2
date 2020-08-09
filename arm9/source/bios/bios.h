#pragma once

enum BiosLoadResult
{
	BIOS_LOAD_RESULT_OK,
	BIOS_LOAD_RESULT_ERROR,
	BIOS_LOAD_RESULT_NOT_FOUND,
	BIOS_LOAD_RESULT_INVALID
};

#define BIOS_SIZE	0x4000

extern vu32 gGbaBios[BIOS_SIZE >> 2];

BiosLoadResult bios_load();