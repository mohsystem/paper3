#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Parses a CSV string and prints its contents.
 * This function creates a mutable copy of the input string to use with strtok_r,
 * which modifies the string it parses. It is a simple parser and does not handle
 * quoted fields containing commas or newlines.
 * 
 * @param csvData The constant string containing CSV data.
 */
void parseAndPrintCSV(const char* csvData) {
    if (csvData == NULL || *csvData == '\0') {
        printf("[]\n");
        return;
    }

    // strtok_r modifies the string, so we need a mutable copy.
    char* csvCopy = strdup(csvData);
    if (csvCopy == NULL) {
        perror("Failed to allocate memory");
        return;
    }

    char* rowSavePtr;
    char* row = strtok_r(csvCopy, "\n", &rowSavePtr);

    printf("[\n");
    while (row != NULL) {
        printf("  [");
        char* fieldSavePtr;
        char* field = strtok_r(row, ",", &fieldSavePtr);
        int isFirstField = 1;

        while (field != NULL) {
            if (!isFirstField) {
                printf(", ");
            }
            printf("\"%s\"", field);
            isFirstField = 0;
            field = strtok_r(NULL, ",", &fieldSavePtr);
        }
        printf("]\n");
        row = strtok_r(NULL, "\n", &rowSavePtr);
    }
    printf("]\n");

    free(csvCopy); // Free the memory allocated by strdup
}

void runTestCase(const char* testName, const char* input) {
    printf("--- %s ---\n", testName);
    printf("Input:\n\"%s\"\n", input);
    printf("Output:\n");
    parseAndPrintCSV(input);
    printf("\n");
}

int main() {
    runTestCase("Test Case 1: Standard CSV", "a,b,c\n1,2,3\nx,y,z");
    runTestCase("Test Case 2: Empty fields", "a,,c\n1,2,\n,y,z");
    runTestCase("Test Case 3: Single row", "one,two,three,four");
    runTestCase("Test Case 4: Single column", "one\ntwo\nthree");
    runTestCase("Test Case 5: Empty input", "");

    return 0;
}