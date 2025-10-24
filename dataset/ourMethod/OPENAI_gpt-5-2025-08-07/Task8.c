#include <stdio.h>
#include <stddef.h>

char find_missing_letter(const char* arr, size_t len) {
    if (arr == NULL || len < 2) return '\0';
    for (size_t i = 1; i < len; ++i) {
        int diff = (int)arr[i] - (int)arr[i - 1];
        if (diff <= 0) return '\0'; // not strictly increasing -> invalid
        if (diff > 1) {
            char candidate = (char)(arr[i - 1] + 1);
            if ((candidate >= 'a' && candidate <= 'z') || (candidate >= 'A' && candidate <= 'Z')) {
                return candidate;
            }
            return '\0';
        }
    }
    return '\0'; // should not happen if exactly one letter is missing
}

int main(void) {
    // Test case 1
    char t1[] = {'a','b','c','d','f'};
    printf("Test 1 expected e, got %c\n", find_missing_letter(t1, sizeof(t1)/sizeof(t1[0])) ? find_missing_letter(t1, sizeof(t1)/sizeof(t1[0])) : '?');

    // Test case 2
    char t2[] = {'O','Q','R','S'};
    printf("Test 2 expected P, got %c\n", find_missing_letter(t2, sizeof(t2)/sizeof(t2[0])) ? find_missing_letter(t2, sizeof(t2)/sizeof(t2[0])) : '?');

    // Test case 3
    char t3[] = {'x','z'};
    printf("Test 3 expected y, got %c\n", find_missing_letter(t3, sizeof(t3)/sizeof(t3[0])) ? find_missing_letter(t3, sizeof(t3)/sizeof(t3[0])) : '?');

    // Test case 4
    char t4[] = {'b','c','d','e','f','g','i'};
    printf("Test 4 expected h, got %c\n", find_missing_letter(t4, sizeof(t4)/sizeof(t4[0])) ? find_missing_letter(t4, sizeof(t4)/sizeof(t4[0])) : '?');

    // Test case 5
    char t5[] = {'M','N','P'};
    printf("Test 5 expected O, got %c\n", find_missing_letter(t5, sizeof(t5)/sizeof(t5[0])) ? find_missing_letter(t5, sizeof(t5)/sizeof(t5[0])) : '?');

    return 0;
}