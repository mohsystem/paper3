/* C implementation
   Safe function using size-aware iteration; no unsafe I/O; validates input length. */
#include <stdio.h>
#include <stdlib.h>

char findMissingLetter(const char* array, size_t length) {
    if (array == NULL || length < 2) {
        /* For safety, signal error in a defined way; caller can handle '\0' as error sentinel. */
        return '\0';
    }
    for (size_t i = 1; i < length; ++i) {
        int diff = (int)array[i] - (int)array[i - 1];
        if (diff != 1) {
            return (char)(array[i - 1] + 1);
        }
    }
    return '\0'; /* Should not occur with valid input */
}

int main(void) {
    char t1[] = {'a','b','c','d','f'};
    char t2[] = {'O','Q','R','S'};
    char t3[] = {'w','x','z'};
    char t4[] = {'A','B','C','E'};
    char t5[] = {'m','n','p','q','r'};

    char res1 = findMissingLetter(t1, sizeof(t1)/sizeof(t1[0]));
    char res2 = findMissingLetter(t2, sizeof(t2)/sizeof(t2[0]));
    char res3 = findMissingLetter(t3, sizeof(t3)/sizeof(t3[0]));
    char res4 = findMissingLetter(t4, sizeof(t4)/sizeof(t4[0]));
    char res5 = findMissingLetter(t5, sizeof(t5)/sizeof(t5[0]));

    printf("Test 1 missing letter: %c\n", res1 ? res1 : '?');
    printf("Test 2 missing letter: %c\n", res2 ? res2 : '?');
    printf("Test 3 missing letter: %c\n", res3 ? res3 : '?');
    printf("Test 4 missing letter: %c\n", res4 ? res4 : '?');
    printf("Test 5 missing letter: %c\n", res5 ? res5 : '?');

    return 0;
}