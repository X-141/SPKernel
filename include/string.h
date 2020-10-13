#ifndef _STRING_H
#define _STRING_H

/*
 * Converts given value to ascii string and stores it into str.
 * it in str.
 * size parameter will limit how much of the converted value can
 * be stored in str.
 * 
 * Return 0 when value can't entirely fit into str.
 * Return 1 on success.
*/
int uint_to_string(char* s, unsigned int value, int size);

/*
 * Compare const char string s1 and s2
 * 
 * Returns 0 if s1 and s2 are equal.
 * Return positive value if character in s1 is greater than s2.
 * Return negative value if character in s1 is less than s2.
*/
int strcmp(const char* s1, const char *s2);

#endif