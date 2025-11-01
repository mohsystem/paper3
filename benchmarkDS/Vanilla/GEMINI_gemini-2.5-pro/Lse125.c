#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void trimTrailing(char *str) {
    if (str == NULL) {
        return;
    }
    int len = strlen(str);
    if (len == 0) {
        return;
    }
    int i = len - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }
    str[i + 1] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        char* arg_copy = (char*)malloc(strlen(argv[1]) + 1);
        if (arg_copy == NULL) {
            perror("Failed to allocate memory");
            return 1;
        }
        strcpy(arg_copy, argv[1]);
        trimTrailing(arg_copy);
        printf("%s\n", arg_copy);
        free(arg_copy);
    } else {
        printf("--- Running Test Cases ---\n");
        const char* testStrings[] = {
            "Hello World   ",
            "  leading and trailing  ",
            "no_whitespace",
            "   \t\n",
            "Trailing tab\t"
        };
        int num_tests = sizeof(testStrings) / sizeof(testStrings[0]);

        for (int i = 0; i < num_tests; i++) {
            char buffer[100];
            strcpy(buffer, testStrings[i]);

            printf("Original: [%s]\n", testStrings[i]);
            trimTrailing(buffer);
            printf("Trimmed:  [%s]\n", buffer);
            printf("\n");
        }
    }
    return 0;
}