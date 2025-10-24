#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char find_missing_letter(const char* arr, size_t len) {
    if (arr == NULL || len < 2) {
        fprintf(stderr, "Input array must have at least 2 characters.\n");
        exit(EXIT_FAILURE);
    }
    int first = (unsigned char)arr[0];
    int is_lower = islower(first);
    int is_upper = isupper(first);
    if (!is_lower && !is_upper) {
        fprintf(stderr, "Input must contain alphabetic characters.\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)arr[i];
        if (!isalpha(c)) {
            fprintf(stderr, "Input must contain only letters.\n");
            exit(EXIT_FAILURE);
        }
        if ((is_lower && !islower(c)) || (is_upper && !isupper(c))) {
            fprintf(stderr, "All letters must be in the same case.\n");
            exit(EXIT_FAILURE);
        }
    }
    int expected = (unsigned char)arr[0] + 1;
    for (size_t i = 1; i < len; ++i) {
        if ((int)(unsigned char)arr[i] != expected) {
            return (char)expected;
        }
        expected++;
    }
    fprintf(stderr, "No missing letter found.\n");
    exit(EXIT_FAILURE);
}

int main(void) {
    char t1[] = {'a','b','c','d','f'};
    char t2[] = {'O','Q','R','S'};
    char t3[] = {'m','n','p','q'};
    char t4[] = {'A','B','C','E'};
    char t5[] = {'t','v'};

    char res1 = find_missing_letter(t1, sizeof(t1)/sizeof(t1[0]));
    char res2 = find_missing_letter(t2, sizeof(t2)/sizeof(t2[0]));
    char res3 = find_missing_letter(t3, sizeof(t3)/sizeof(t3[0]));
    char res4 = find_missing_letter(t4, sizeof(t4)/sizeof(t4[0]));
    char res5 = find_missing_letter(t5, sizeof(t5)/sizeof(t5[0]));

    printf("Missing: '%c'\n", res1);
    printf("Missing: '%c'\n", res2);
    printf("Missing: '%c'\n", res3);
    printf("Missing: '%c'\n", res4);
    printf("Missing: '%c'\n", res5);

    return 0;
}