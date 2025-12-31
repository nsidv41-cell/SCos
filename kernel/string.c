/* ============================================================================
 * SCos 1.3.5 - String Functions
 * ============================================================================ */

#include "../include/scos.h"

/* String length */
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/* String copy */
char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

/* String copy with limit */
char *strncpy(char *dest, const char *src, size_t n) {
    char *ret = dest;
    while (n-- && (*dest++ = *src++));
    while (n-- > 0) {
        *dest++ = '\0';
    }
    return ret;
}

/* String compare */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/* String compare with limit */
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    if (n == (size_t)-1) {
        return 0;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/* String concatenate */
char *strcat(char *dest, const char *src) {
    char *ret = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
    return ret;
}

/* String concatenate with limit */
char *strncat(char *dest, const char *src, size_t n) {
    char *ret = dest;
    while (*dest) {
        dest++;
    }
    while (n-- && (*dest++ = *src++));
    *dest = '\0';
    return ret;
}

/* Find character in string */
char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == c) {
            return (char *)str;
        }
        str++;
    }
    return (c == '\0') ? (char *)str : NULL;
}

/* Find last occurrence of character */
char *strrchr(const char *str, int c) {
    const char *last = NULL;
    while (*str) {
        if (*str == c) {
            last = str;
        }
        str++;
    }
    return (c == '\0') ? (char *)str : (char *)last;
}

/* Find substring */
char *strstr(const char *haystack, const char *needle) {
    size_t needle_len = strlen(needle);
    if (needle_len == 0) {
        return (char *)haystack;
    }
    
    while (*haystack) {
        if (strncmp(haystack, needle, needle_len) == 0) {
            return (char *)haystack;
        }
        haystack++;
    }
    return NULL;
}

/* Memory set */
void *memset(void *ptr, int value, size_t num) {
    unsigned char *p = (unsigned char *)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

/* Memory copy */
void *memcpy(void *dest, const void *src, size_t num) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (num--) {
        *d++ = *s++;
    }
    return dest;
}

/* Memory move (handles overlapping) */
void *memmove(void *dest, const void *src, size_t num) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    if (d < s) {
        while (num--) {
            *d++ = *s++;
        }
    } else {
        d += num;
        s += num;
        while (num--) {
            *--d = *--s;
        }
    }
    return dest;
}

/* Memory compare */
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/* String to integer */
int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    
    while (isspace(*str)) {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

/* Integer to string */
void itoa(int value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        ptr1++;
        value = -value;
    }
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

/* Unsigned integer to string */
void utoa(uint32_t value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    uint32_t tmp_value;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

/* Character classification */
int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || 
           c == '\r' || c == '\f' || c == '\v';
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

/* Tokenize string */
static char *strtok_ptr = NULL;

char *strtok(char *str, const char *delim) {
    if (str != NULL) {
        strtok_ptr = str;
    }
    
    if (strtok_ptr == NULL) {
        return NULL;
    }
    
    /* Skip leading delimiters */
    while (*strtok_ptr && strchr(delim, *strtok_ptr)) {
        strtok_ptr++;
    }
    
    if (*strtok_ptr == '\0') {
        return NULL;
    }
    
    char *token_start = strtok_ptr;
    
    /* Find end of token */
    while (*strtok_ptr && !strchr(delim, *strtok_ptr)) {
        strtok_ptr++;
    }
    
    if (*strtok_ptr) {
        *strtok_ptr++ = '\0';
    }
    
    return token_start;
}

/* Simple sprintf implementation */
int sprintf(char *buf, const char *fmt, ...) {
    char *str = buf;
    const char *p = fmt;
    
    /* Get variable arguments */
    uint32_t *args = (uint32_t *)(&fmt + 1);
    int arg_idx = 0;
    
    while (*p) {
        if (*p != '%') {
            *str++ = *p++;
            continue;
        }
        
        p++;  /* Skip '%' */
        
        /* Handle flags and width */
        int zero_pad = 0;
        int width = 0;
        
        if (*p == '0') {
            zero_pad = 1;
            p++;
        }
        
        while (isdigit(*p)) {
            width = width * 10 + (*p - '0');
            p++;
        }
        
        char tmp[32];
        int len;
        
        switch (*p) {
            case 's': {
                const char *s = (const char *)args[arg_idx++];
                if (s == NULL) s = "(null)";
                while (*s) {
                    *str++ = *s++;
                }
                break;
            }
            
            case 'c':
                *str++ = (char)args[arg_idx++];
                break;
            
            case 'd':
            case 'i': {
                int val = (int)args[arg_idx++];
                itoa(val, tmp, 10);
                len = strlen(tmp);
                while (width > len) {
                    *str++ = zero_pad ? '0' : ' ';
                    width--;
                }
                for (int i = 0; tmp[i]; i++) {
                    *str++ = tmp[i];
                }
                break;
            }
            
            case 'u': {
                uint32_t val = args[arg_idx++];
                utoa(val, tmp, 10);
                len = strlen(tmp);
                while (width > len) {
                    *str++ = zero_pad ? '0' : ' ';
                    width--;
                }
                for (int i = 0; tmp[i]; i++) {
                    *str++ = tmp[i];
                }
                break;
            }
            
            case 'x':
            case 'X': {
                uint32_t val = args[arg_idx++];
                utoa(val, tmp, 16);
                len = strlen(tmp);
                while (width > len) {
                    *str++ = zero_pad ? '0' : ' ';
                    width--;
                }
                for (int i = 0; tmp[i]; i++) {
                    *str++ = (*p == 'X') ? toupper(tmp[i]) : tmp[i];
                }
                break;
            }
            
            case 'p': {
                uint32_t val = args[arg_idx++];
                *str++ = '0';
                *str++ = 'x';
                utoa(val, tmp, 16);
                for (int i = 0; tmp[i]; i++) {
                    *str++ = tmp[i];
                }
                break;
            }
            
            case '%':
                *str++ = '%';
                break;
            
            default:
                *str++ = '%';
                *str++ = *p;
                break;
        }
        
        p++;
    }
    
    *str = '\0';
    return str - buf;
}
