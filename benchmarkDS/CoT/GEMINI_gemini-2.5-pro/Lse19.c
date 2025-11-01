#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Parses a string to a long long, adds 1000, and prints the result or an error.
 * Uses strtoll for safe conversion, checking for errors like invalid format or out of range.
 * @param input The C-style string to be converted.
 */
void addThousandAndPrint(const char* input) {
    char* endptr;
    errno = 0; // To distinguish success/failure after the call

    long long number = strtoll(input, &endptr, 10);

    // Check for various conversion errors
    if (endptr == input) {
        // No digits were found
        fprintf(stderr, "  Error: Invalid number format. No digits found.\n");
    } else if (*endptr != '\0') {
        // The string contains non-numeric characters after the number
        fprintf(stderr, "  Error: Invalid number format. Trailing characters found: %s\n", endptr);
    } else if (errno == ERANGE) {
        // The number was out of the range of a long long
        fprintf(stderr, "  Error: Number is out of range for a long long.\n");
    } else {
        // Successful conversion
        printf("  Output: %lld\n", number + 1000);
    }
}

int main(int argc, char* argv[]) {
    // --- 1. Process Command Line Argument ---
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        fprintf(stderr, "Note: Exactly one command-line argument is required.\n");
    } else {
        printf("Result from command line input '%s':\n", argv[1]);
        addThousandAndPrint(argv[1]);
    }
    
    // --- 2. Run 5 Test Cases ---
    printf("\n--- Running 5 Internal Test Cases ---\n");
    const char* testInputs[] = {"77", "-77", "0", "12345non-digit", "9223372036854775807"}; // Last one will cause ERANGE
    for (int i = 0; i < 5; ++i) {
        printf("\nTest Case %d: Input = \"%s\"\n", i + 1, testInputs[i]);
        addThousandAndPrint(testInputs[i]);
    }

    return 0;
}