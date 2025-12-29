// SCos String Library
// Custom string manipulation functions (no external dependencies)

#ifndef STRING_H
#define STRING_H

#include "kernel.h"

namespace String {
    // String length
    size_t strlen(const char* str);
    
    // String copy
    char* strcpy(char* dest, const char* src);
    char* strncpy(char* dest, const char* src, size_t n);
    
    // String compare
    int strcmp(const char* s1, const char* s2);
    int strncmp(const char* s1, const char* s2, size_t n);
    
    // String concatenate
    char* strcat(char* dest, const char* src);
    
    // Integer to string
    char* itoa(int value, char* str, int base);
    char* utoa(unsigned int value, char* str, int base);
    
    // Reverse string
    void reverse(char* str, int length);
}

#endif // STRING_H
