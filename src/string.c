#include "string.h"

int uint_to_string(char* s, unsigned int value, int size) {
    int index = 0;
    
    do {
        s[index++] = (value % 10) + 0x30;
        value /= 10;
    } while(index < (size-3) && value != 0);

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