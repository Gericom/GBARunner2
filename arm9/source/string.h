#pragma once

int strlen(char* str);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strpbrk(const char *s1, const char *s2);

static inline int to_upper(const char c)
{
	return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
}

int strcasecmp(const char* one, const char* another);
int strcmp(const char* one, const char* another);