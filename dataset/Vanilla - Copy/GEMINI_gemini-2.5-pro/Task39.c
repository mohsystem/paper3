#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Displays the content of a file using a system command.
 * This function is cross-platform, using 'type' on Windows and 'cat' on others.
 * @param filename The name of the file to display.
 */
void displayFileContent(const char* filename) {
    // Buffer to hold the command. 512 is a generous size.
    char command[512]; 

    // Use preprocessor directives for OS-specific commands
#if defined(_WIN32) || defined(_WIN64)
    // snprintf is a safe way to format strings in C
    snprintf(command, sizeof(command), "type \"%s\"", filename);
#else
    snprintf(command, sizeof(command), "cat \"%s\"", filename);
#endif

    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Command failed or returned a non-zero exit code for file: %s\n", filename);
    }
}

int main() {
    const int NUM_FILES = 5;
    char testFiles[NUM_FILES][50];
    for (int i = 0; i < NUM_FILES; ++i) {
        sprintf(testFiles[i], "c_test_file_%d.txt", i + 1);
    }

    printf("--- Creating test files ---\n");
    for (int i = 0; i < NUM_FILES; ++i) {
        FILE *fp = fopen(testFiles[i], "w");
        if (fp != NULL) {
            fprintf(fp, "Hello from C test file %d.\n", i + 1);
            fprintf(fp, "This is the content of %s.\n", testFiles[i]);
            fclose(fp);
            printf("Created: %s\n", testFiles[i]);
        } else {
            perror("Error creating file");
            return 1;
        }
    }

    printf("\n--- Running C Test Cases ---\n");
    for (int i = 0; i < NUM_FILES; ++i) {
        printf("\n--- Test Case %d: Displaying %s ---\n", i + 1, testFiles[i]);
        displayFileContent(testFiles[i]);
        printf("--- End of %s ---\n", testFiles[i]);
    }

    printf("\n--- Cleaning up test files ---\n");
    for (int i = 0; i < NUM_FILES; ++i) {
        if (remove(testFiles[i]) != 0) {
            perror("Error deleting file");
        } else {
            printf("Deleted: %s\n", testFiles[i]);
        }
    }

    return 0;
}