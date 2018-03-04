#include "vram.h"
#include "string.h"

PUT_IN_VRAM int strlen(char* str)
{
	char* ptr = str;
	while (*ptr != '\0')
		++ptr;
	return ptr - str;
}

PUT_IN_VRAM char *strchr(const char *s, int c)
{
	while (*s != (char)c)
		if (!*s++)
			return 0;
	return (char *)s;
}

PUT_IN_VRAM char *strrchr(const char *s, int c)
{
	char* ret = 0;
	do {
		if (*s == (char)c)
			ret = (char*)s;
	} while (*s++);
	return ret;
}

PUT_IN_VRAM char *strpbrk(const char *s1, const char *s2)
{
	while (*s1)
		if (strchr(s2, *s1++))
			return (char*)--s1;
	return 0;
}

PUT_IN_VRAM int strcasecmp(const char* one, const char* another)
{
	for (; to_upper(*one) == to_upper(*another); ++one, ++another)
		if (*one == '\0')
			return 0;
	return to_upper(*one) - to_upper(*another);
}

PUT_IN_VRAM int strcmp(const char* one, const char* another)
{
	for (; *one == *another; ++one, ++another)
		if (*one == '\0')
			return 0;
	return *one - *another;
}