#include "string.h"
#include <stddef.h>
#include <stdio.h>

//! This solution is too slow for large numbers.
// int test_uint_to_string(char* s, unsigned int value, int size) {
//         int mod = 1;
//     while(value / (mod * 10) != 0)
//         mod *= 10;
//     /* Break out if we exceed size or mod becomes 0. */
//     int x;
//     for(x = 0; x < (size - 3) && mod != 0; x++) {
//         s[x] = (value / mod) + 0x30;
//         value %= mod;
//         mod /= 10;
//     }
//     // May get rid of these
//     s[x++] = '\n';
//     s[x++] = '\r';
//     s[x]   = '\0';
//     if(value != 0)
//         return 0;
//     return 1;
// }

int test_uint_to_string(char* s, unsigned int value, int size) {
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

int test_strcmp(const char* s1, const char* s2) {
    if(s1 != NULL && s2 != NULL) {
        while(*s1 != '\0') {
            if(*s1 != *s2)
                break;
            s1++;
            s2++;
        }
        return *s1 - *s2;
    }
    /*
     * For our purposes (prevent seg fault), we will check if s1 and s2
     * are NULL. We should double check for possible issues like this in 
     * the kernel as well.
    */
   return ERR_FLAG; // We don't have a special flag. ERR_STRING will be a placeholder
}

void zero_str(char* s, int size) {
    if(s != NULL) {
        for(int x = 0; x < size; x++)
            s[x] = '\0';
    }
}