#pragma once
#include "fat/ff.h"

typedef void (*inir_callback_t)(void* arg, const char* section, const char* key, const char* value);

class INIReader
{
public:
    static bool Parse(FIL* file, inir_callback_t callback, void* arg);
};