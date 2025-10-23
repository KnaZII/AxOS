#ifndef STRING_H
#define STRING_H

#include "kernel.h"

int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int n);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);

void* memset(void* ptr, int value, int num);
void* memcpy(void* dest, const void* src, int num);
int memcmp(const void* ptr1, const void* ptr2, int num);

char* itoa(int value, char* str, int base);
int atoi(const char* str);

#endif