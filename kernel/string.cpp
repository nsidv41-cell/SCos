// SCos String Library Implementation

#include "string.h"

namespace String {

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* orig = dest;
    while ((*dest++ = *src++));
    return orig;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig = dest;
    while (n && (*dest++ = *src++)) {
        n--;
    }
    while (n--) {
        *dest++ = '\0';
    }
    return orig;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcat(char* dest, const char* src) {
    char* orig = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return orig;
}

void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* itoa(int value, char* str, int base) {
    int i = 0;
    bool isNegative = false;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }
    
    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        value = value / base;
    }
    
    if (isNegative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse(str, i);
    return str;
}

char* utoa(unsigned int value, char* str, int base) {
    int i = 0;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    while (value != 0) {
        unsigned int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        value = value / base;
    }
    
    str[i] = '\0';
    reverse(str, i);
    return str;
}

} // namespace String
