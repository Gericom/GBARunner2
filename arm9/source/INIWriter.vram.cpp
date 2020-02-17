#include "vram.h"
#include "vramheap.h"
#include "fat/ff.h"
#include "INIWriter.h"

INIWriter::INIWriter(FIL* file)
    : _file(file)
{

}

void INIWriter::WriteSection(const char* name)
{
    f_printf(_file, "[%s]\n", name);
}

void INIWriter::WriteProperty(const char* key, const char* value)
{
    f_printf(_file, "%s=%s\n", key, value);
}

void INIWriter::WriteBooleanProperty(const char* key, bool value)
{
    WriteProperty(key, value ? "true" : "false");
}

void INIWriter::WriteIntegerProperty(const char* key, int value)
{
    f_printf(_file, "%s=%d\n", key, value);
}

void INIWriter::WriteMacAddressProperty(const char* key, const u8* macAddress)
{
    f_printf(_file, "%s=%02X%02X%02X%02X%02X%02X\n", key,
        macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
}