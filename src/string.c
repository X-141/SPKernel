#include "string.h"

int uint_to_string(char* s, unsigned int value, int size) {
    int index = 0;
    int fit = 1;
    do {
        s[index++] = (value % 10) + 0x30;
        value /= 10;
        if(index == (size-3)) {
            fit = 0;
            break;
        }
    } while(/*index<(size-3)&&*/value != 0);

    // Pad last three spaces of the string with terminal 
    // characters
    s[index]    = '\n';
    s[index+1]  = '\r';
    s[index+2]  = '\0';
    index--;
    // Reverse
    int reverse_index = 0;
    char swap_char = '\0';
    while(reverse_index < index) {
        swap_char = s[reverse_index];
        s[reverse_index] = s[index];
        s[index] = swap_char;
        
        reverse_index++;
        index--;
    }
    return fit;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 != '\0') {
        if(*s1 != *s2)
            break;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a = s1,*b = s2;
    while(n-- > 0){ 
        if(*a != *b) { 
            return *a - *b; 
        } 
        a++; 
        b++;
    }
    return 0;
}

int memcpy(void *dest, const void *src, int n) {
    char* _d = (char *) dest;
    const char* _s = (const char *) src;
    for(int x = 0; x < n ; x++)
        _d[x] = _s[x];
    return 0;
}

int zerodata(void *arr, int n) {
    char* _a = (char *) arr;
    for(int x = 0; x < n; x++)
        _a[x] = 0;
    return 0;
}
