#pragma once

enum RomLoadResult
{
	ROM_LOAD_RESULT_OK,
	ROM_LOAD_RESULT_ROM_READ_ERR,
	ROM_LOAD_RESULT_SAVE_CREATE_ERR,
	ROM_LOAD_RESULT_SAVE_TOO_SMALL,
    ROM_LOAD_RESULT_SAVE_READ_ERR
};

void gbab_setupGfx();
void gbab_setupCache();
void gbab_loadFrame(u32 id);
RomLoadResult gbab_loadRom(const char* path);