#include "string.h"

int uint_to_string(char* str, unsigned int value, int size) {
    int mod = 1;
    while(value / (mod * 10) != 0)
        mod *= 10;
    
    /* Break out if we exceed size or mod becomes 0. */
    int x;
    for(x = 0; x < (size - 3) && mod != 0; x++) {
        str[x] = (value / mod) + 0x30;
        value %= mod;
        mod /= 10;
    }

    str[x++] = '\n';
    str[x++] = '\r';
    str[x]   = '\0';
    if(value != 0)
        return 0;
    return 1;
}