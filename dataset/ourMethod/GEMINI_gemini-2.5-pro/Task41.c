#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define a reasonable buffer size to prevent unbounded memory usage on the stack.
#define BUFFER_SIZE 128

/**
 * @brief Processes the given string by printing it and its length.
 * 
 * @param input A pointer to a constant character array (string).
 * @param test_case_num The number of the current test case.
 */
void processString(const char* input, int test_case_num) {
    printf("\n--- Processing Test Case %d ---\n", test_case_num);
    printf("Input string: \"%s\"\n", input);
    // Use %zu for size_t, which is the return type of strlen.
    printf("Length: %zu\n", strlen(input));
    printf("---------------------------------\n");
}

/**
 * @brief Clears the standard input buffer.
 * 
 * This function reads and discards characters from stdin until a newline
 * or EOF is encountered. It is used to discard leftover characters when
 * user input exceeds the buffer size.
 */
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Main function to demonstrate safe string input in C.
 * 
 * It runs 5 test cases where the user provides input. The input is read
 * using fgets(), which is a buffer-safe function. The code also handles
 * cases where the input is longer than the buffer.
 * 
 * @return int Exit code. EXIT_SUCCESS for success.
 */
int main() {
    // A fixed-size buffer on the stack to hold user input.
    char buffer[BUFFER_SIZE];

    printf("C Safe String Input Demonstration\n");
    printf("Please provide 5 string inputs for processing.\n");
    printf("Max input length is %d characters. Longer inputs will be truncated.\n", BUFFER_SIZE - 1);
    printf("====================================================================\n");

    for (int i = 1; i <= 5; ++i) {
        printf("\nEnter test string #%d: ", i);

        // Use fgets to read safely from standard input.
        // 1. It's buffer-safe: It will not write more than sizeof(buffer) - 1 characters.
        // 2. It always null-terminates the string if any characters are read.
        // 3. We must check its return value for errors or EOF.
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Check if the entire line was read by searching for the newline character.
            char* newline_char = strchr(buffer, '\n');
            if (newline_char != NULL) {
                // Newline was found, so the entire line fit in the buffer.
                // Replace it with a null terminator for cleaner processing.
                *newline_char = '\0';
            } else {
                // Newline was not found. This means the input was too long for the buffer
                // and was truncated. We must clear the rest of the input stream.
                printf("Warning: Input was longer than %d characters and has been truncated.\n", BUFFER_SIZE - 1);
                clear_input_buffer();
            }
            processString(buffer, i);
        } else {
            if (feof(stdin)) {
                 printf("\nEnd of file reached. Exiting.\n");
            } else {
                fprintf(stderr, "Error reading input. Exiting.\n");
            }
            break;
        }
    }
    
    printf("\nProgram finished after 5 tests.\n");

    return EXIT_SUCCESS;
}