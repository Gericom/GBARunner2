#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "fat/ff.h"
#include "INIReader.h"

bool INIReader::Parse(FIL* file, inir_callback_t callback, void* arg)
{
    char line[128];
    char section[128];
    while (f_gets(line, sizeof(line), file))
    {
        if(line[0] == ';')
            continue;
        char* newLine = strchr(line, '\n');
        *newLine = 0;
        if(line[0] == '[')
        {
            int len = strlen(line) - 2;
            for(int i = 0; i < len; i++)
                section[i] = line[i + 1];
            section[len] = 0;
            continue;
        }
        char* equals = strchr(line, '=');
        if(!equals)
            continue;
        *equals = 0;
        callback(arg, section, line, equals + 1);
    }
}