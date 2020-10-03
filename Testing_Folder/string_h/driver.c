#include "string.h"
#include "../minunit.h"

#include <stdio.h>

/*
 * s1 and s2 are NULL.
 * EO: Return error flag indicating one or more params are NULL
 */
MU_TEST(s1_null_s2_null) {
    int val = test_strcmp(NULL, NULL);
    mu_assert(val == ERR_FLAG, "Passing NULL in both parameter fields should return ERR_FLAG.");
}

/* 
 * s1 and s2 are single character strings
 * E0: Return value of zero. Both are equal.
 */
MU_TEST(s1_singlechar_s2_singlechar) {
    int val = test_strcmp("a", "a");
    mu_assert(val == 0, "Return value should be zero.");
}

/* 
 * s1 and s2 are empty strings.
 * E0: Return value of zero. Both are equal despite being empty strings.
 */
MU_TEST(s1_empty_s2_empty) {
    int val = test_strcmp("\0", "\0");
    mu_assert(val == 0, "Return value should be zero.");
}

/* 
 * s1 is a single character string and s2 is a empty string
 * E0: Return positive value as "a" is greater value than \0.
 */
MU_TEST(s1_singlechar_s2_empty) {
    int val = test_strcmp("a", "\0");
    mu_assert(val > 0, "Return value should be positive.");
}

/* 
 * s1 is a empty string and s2 is a single character string
 * E0: Return negative value as \0 is less than "a".
 */
MU_TEST(s1_empty_s2_singlechar) {
    int val = test_strcmp("\0", "a");
    mu_assert(val < 0, "Return value should be negative.");
}

/*
 * s1 is a empty string and s2 is NULL.
 * EO: Return error flag indicating one or more params are NULL
 */
MU_TEST(s1_empty_s2_null) {
    int val = test_strcmp("\0", NULL);
    mu_assert(val == ERR_FLAG, "Passing NULL in both parameter fields should return ERR_FLAG.");
}

/*
 * s1 is a empty string and s2 is NULL.
 * EO: Return error flag indicating one or more params are NULL
 */
MU_TEST(s1_null_s2_empty) {
    int val = test_strcmp("\0", NULL);
    mu_assert(val == ERR_FLAG, "Passing NULL in both parameter fields should return ERR_FLAG.");
}

/*
 * s1 is a single character string and s2 is NULL.
 * EO: Return error flag indicating one or more params are NULL
 */
MU_TEST(s1_singlechar_s2_null) {
    int val = test_strcmp("a", NULL);
    mu_assert(val == ERR_FLAG, "Passing NULL in both parameter fields should return ERR_FLAG.");
}

/*
 * s1 is NULL and s2 is single character string.
 * EO: Return error flag indicating one or more params are NULL
 */
MU_TEST(s1_null_s2_singlechar) {
    int val = test_strcmp(NULL, "a");
    mu_assert(val == ERR_FLAG, "Passing NULL in both parameter fields should return ERR_FLAG.");
}

/*
 * s1 and s2 are normal strings.
 * EO: Return value should be less than zero, "N" is greater than "A".
 */ 
MU_TEST(s1_normalstring_s2_normalstring_1) {
    int val = test_strcmp("Normal_name", "Another Normal Name");
    mu_assert(val > 0, "Value should be greater than 0.");
}

/*
 * s1 and s2 are normal strings.
 * EO: Return value should be less than zero, "N" is greater than "A".
 */ 
MU_TEST(s1_normalstring_s2_normalstring_2) {
    int val = test_strcmp("Standard", "Stacdard");
    mu_assert(val > 0, "Value should be greater than 0.");
}

/*
 * s1 and s2 are normal strings.
 * EO: Return value should be zero.
 */ 
MU_TEST(s1_normalstring_s2_normalstring_3) {
    int val = test_strcmp("Standard", "Standard");
    mu_assert(val == 0, "Value should be zero.");
}

/*
 * Test cases to check functionality of string comparator.
 */
MU_TEST_SUITE(strcmp_test_suite) {
    MU_RUN_TEST(s1_null_s2_null);
    MU_RUN_TEST(s1_singlechar_s2_singlechar);
    MU_RUN_TEST(s1_empty_s2_empty);
    MU_RUN_TEST(s1_singlechar_s2_empty);
    MU_RUN_TEST(s1_empty_s2_singlechar);
    MU_RUN_TEST(s1_empty_s2_null);
    MU_RUN_TEST(s1_null_s2_empty);
    MU_RUN_TEST(s1_singlechar_s2_null);
    MU_RUN_TEST(s1_null_s2_singlechar);
    MU_RUN_TEST(s1_normalstring_s2_normalstring_1);
    MU_RUN_TEST(s1_normalstring_s2_normalstring_2);
    MU_RUN_TEST(s1_normalstring_s2_normalstring_3);
}

#define TEST_CHAR_ARR_LENGTH 16

/* 
 * Convert 0 to ascii string and store it within given char array.
 * EO: String will contain only a single character '0'.
 */ 
MU_TEST(zero_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, 0, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "0\n\r");
    mu_assert(val == 0, "String should contain only 0");
}

/* 
 * Convert 1 to ascii string and store it within given char array.
 * EO: String will contain only a single character '1'.
 */ 
MU_TEST(one_int_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, 1, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "1\n\r");
    mu_assert(val == 0, "String should contain only 1");
}

/* 
 * Convert maximum unsigned value to ascii string and store 
 * it within given char array.
 * EO: String will contain "4294967295".
 */ 
MU_TEST(unsigned_max_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, 0xFFFFFFFF, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "4294967295\n\r");
    mu_assert(val == 0, "String should contain only 4294967295");
}

/* 
 * Convert signed integer value to ascii string and store 
 * it within given char array.
 * EO: String will contain "4294967295".
 */ 
MU_TEST(signed_int_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, -1, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "4294967295\n\r");
    mu_assert(val == 0, "String should contain only 4294967295");
}

/* 
 * Convert a unsigned value of length 5 to ascii string. This ensures
 * that the reverse step of the algorithm works for odd length numbers.
 * EO: String should contain "45123"
 */ 
MU_TEST(normal_5size_uint_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, 45123, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "45123\n\r");
    mu_assert(val == 0, "String should contain only 45123");
}

/* 
 * Convert a unsigned value of length 4 to ascii string. This ensures
 * that the reverse step of the algorithm works for even length numbers.
 * EO: String should contain "4523"
 */ 
MU_TEST(normal_4size_uint_to_string) {
    char string[TEST_CHAR_ARR_LENGTH];
    zero_str(string, TEST_CHAR_ARR_LENGTH);
    test_uint_to_string(string, 4523, TEST_CHAR_ARR_LENGTH);
    int val = test_strcmp(string, "4523\n\r");
    mu_assert(val == 0, "String should contain only 4523");
}

/* 
 * Convert a unsigned value that has a number length greater than
 * what can be held in the string.
 * EO: Part of the unsigned value stored. In this case: "67295"
 */ 
MU_TEST(exceed_size_to_string) {
    int test_size = TEST_CHAR_ARR_LENGTH / 2;

    char string[test_size];
    zero_str(string, test_size);
    test_uint_to_string(string, 0xFFFFFFFF, test_size);
    int val = test_strcmp(string, "67295\n\r");
    mu_assert(val == 0, "String should contain only 67295\n\r");
}

/* 
 * Test cases for unsigned integer to string convert.
 */
MU_TEST_SUITE(uint_to_string_suite) {
    MU_RUN_TEST(zero_to_string);
    MU_RUN_TEST(one_int_to_string);
    MU_RUN_TEST(unsigned_max_to_string);
    MU_RUN_TEST(signed_int_to_string);
    MU_RUN_TEST(normal_5size_uint_to_string);
    MU_RUN_TEST(normal_4size_uint_to_string);
    MU_RUN_TEST(exceed_size_to_string);
}

int main() {
    MU_RUN_SUITE(strcmp_test_suite);
    //MU_RUN_SUITE(uint_to_string_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}