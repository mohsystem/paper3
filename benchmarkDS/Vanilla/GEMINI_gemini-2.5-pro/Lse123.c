#include <stdio.h>
#include <string.h>

// The argument is char*, which is the C representation for a string.
void processString(char* str) {
    if (str == NULL) {
        return;
    }

    int len = strlen(str);

    // The for loop trims the trailing whitespace (space, tab) in the string.
    int i;
    for (i = len - 1; i >= 0; i--) {
        if (str[i] != ' ' && str[i] != '\t') {
            break;
        }
    }
    str[i + 1] = '\0'; // Null-terminate after non-whitespace

    // The second for loop trims the newlines.
    len = strlen(str); // Update length after first trim
    int j;
    for (j = len - 1; j >= 0; j--) {
        if (str[j] != '\n' && str[j] != '\r') {
            break;
        }
    }
    // The last line null terminates the string.
    str[j + 1] = '\0';
}

void print_escaped(const char* s) {
    printf("\"");
    for (int i = 0; s[i] != '\0'; ++i) {
        switch (s[i]) {
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:   putchar(s[i]); break;
        }
    }
    printf("\"");
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    if (argc == 2) {
        // The argument is a string (char*).
        // It's not safe to modify argv directly, so we copy it to a local buffer.
        char buffer[1024];
        strncpy(buffer, argv[1], 1023);
        buffer[1023] = '\0';

        printf("Processing command-line argument:\n");
        printf("Original:  ");
        print_escaped(argv[1]);
        printf("\n");

        processString(buffer);

        printf("Processed: ");
        print_escaped(buffer);
        printf("\n---------------------------------\n");
    } else {
        printf("Incorrect argument count. Running test cases instead.\n");
        printf("---------------------------------\n");
    }

    // Test cases must be mutable char arrays, not string literals
    char test1[] = "  hello world   \t\n\r";
    char test2[] = "test case 2\t\t";
    char test3[] = "no trailing whitespace";
    char test4[] = "only newlines\n\n\r\n";
    char test5[] = "   \t \n \r ";
    
    char* testCases[] = { test1, test2, test3, test4, test5 };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        printf("Test Case %d:\n", i + 1);
        printf("Original:  ");
        print_escaped(testCases[i]);
        printf("\n");

        processString(testCases[i]);

        printf("Processed: ");
        print_escaped(testCases[i]);
        printf("\n\n");
    }

    return 0;
}