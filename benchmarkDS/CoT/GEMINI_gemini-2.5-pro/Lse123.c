#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Processes a C-style string in-place by trimming trailing whitespace
 * and removing all newline characters.
 * @param str A mutable, null-terminated character array (string).
 */
void processString(char* str) {
    if (str == NULL) {
        return;
    }
    
    int len = strlen(str);

    // The for loop trims the trailing whitespace in the string.
    int i;
    for (i = len - 1; i >= 0; i--) {
        if (!isspace((unsigned char)str[i])) {
            break;
        }
    }
    // Null terminate the string after the last non-whitespace character.
    str[i + 1] = '\0';
    
    // The second for loop trims the newlines using a two-pointer approach.
    int writer_idx = 0;
    for (int reader_idx = 0; str[reader_idx] != '\0'; reader_idx++) {
        if (str[reader_idx] != '\n' && str[reader_idx] != '\r') {
            str[writer_idx] = str[reader_idx];
            writer_idx++;
        }
    }
    
    // The last line null terminates the string.
    str[writer_idx] = '\0';
}

int main(int argc, char* argv[]) {
    // The prompt implies command-line argument handling, but also requires
    // 5 test cases in main. We will run the test cases directly.
    // For command-line:
    // if (argc != 2) {
    //     fprintf(stderr, "Error: Exactly one argument is required.\n");
    //     return 1;
    // }
    
    // To process arguments safely, they must be copied to a mutable buffer.
    // char* buffer = malloc(strlen(argv[1]) + 1);
    // strcpy(buffer, argv[1]);
    // processString(buffer);
    // printf("Processed: %s\n", buffer);
    // free(buffer);
    
    char testCase1[] = "  hello world   \t ";
    char testCase2[] = "test with\n newlines \n and spaces   ";
    char testCase3[] = "   \n\t\r  ";
    char testCase4[] = "no_trailing_whitespace";
    char testCase5[] = "another\r\ncase with a final newline\n";

    char* testCases[] = { testCase1, testCase2, testCase3, testCase4, testCase5 };
    const char* originalStrings[] = {
        "  hello world   \t ",
        "test with\n newlines \n and spaces   ",
        "   \n\t\r  ",
        "no_trailing_whitespace",
        "another\r\ncase with a final newline\n"
    };

    printf("Running 5 C test cases:\n");
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("  Original:  \"%s\"\n", originalStrings[i]);
        
        processString(testCases[i]);
        
        printf("  Processed: \"%s\"\n\n", testCases[i]);
    }
    
    return 0;
}