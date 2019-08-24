#pragma once
#include "fat/ff.h"

class INIWriter
{
    FIL* _file;
public:
    explicit INIWriter(FIL* file);

    void WriteSection(const char* name);
    void WriteProperty(const char* key, const char* value);
    void WriteBooleanProperty(const char* key, bool value);
    void WriteIntegerProperty(const char* key, int value);
};