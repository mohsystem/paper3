#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* expandedForm(int num) {
    char* result = (char*)malloc(128 * sizeof(char));
    if (result == NULL) {
        return NULL; 
    }
    result[0] = '\0';

    int divisor = 1;
    while (divisor <= num / 10) {
        divisor *= 10;
    }

    int temp_num = num;
    while (divisor > 0) {
        int digit = temp_num / divisor;
        if (digit > 0) {
            int part = digit * divisor;
            if (strlen(result) > 0) {
                strcat(result, " + ");
            }
            char part_str[20];
            sprintf(part_str, "%d", part);
            strcat(result, part_str);
            temp_num %= divisor;
        }
        divisor /= 10;
    }

    return result;
}

int main() {
    int test_cases[] = {12, 42, 70304, 999, 800000};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_cases; i++) {
        char* result = expandedForm(test_cases[i]);
        if (result != NULL) {
            printf("%d -> %s\n", test_cases[i], result);
            free(result); // Free the dynamically allocated memory
        }
    }
    
    return 0;
}