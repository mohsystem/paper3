#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * @brief Reads a file and finds lines that start with a number and end with a dot.
 * @param filePath The path to the file.
 * @param count A pointer to an integer to store the number of matches found.
 * @return A dynamically allocated array of strings containing the matches.
 *         The caller is responsible for freeing the memory using freeMatchedLines().
 *         Returns NULL on error.
 */
char** findMatchingLines(const char* filePath, int* count) {
    *count = 0;
    char** matchedLines = NULL;
    
    // Basic security check for path traversal
    if (strstr(filePath, "..") != NULL) {
        fprintf(stderr, "Error: Invalid or insecure file path provided.\n");
        return NULL;
    }

    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filePath);
        return NULL;
    }

    regex_t regex;
    // The pattern is simple and not vulnerable to ReDoS attacks.
    const char* pattern = "^[0-9].*\\.$";
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        return NULL;
    }

    char line[4096]; // Assume a max line length for simplicity
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline character, if it exists
        line[strcspn(line, "\n")] = 0;

        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            // Found a match, resize the array
            char** temp = realloc(matchedLines, (*count + 1) * sizeof(char*));
            if (temp == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                // Clean up previously allocated memory before exiting
                for (int i = 0; i < *count; ++i) {
                    free(matchedLines[i]);
                }
                free(matchedLines);
                regfree(&regex);
                fclose(file);
                return NULL;
            }
            matchedLines = temp;

            // Allocate memory for the new line and copy it
            matchedLines[*count] = malloc(strlen(line) + 1);
            if (matchedLines[*count] == NULL) {
                 fprintf(stderr, "Memory allocation failed\n");
                 // Clean up and exit
                 for (int i = 0; i < *count; ++i) free(matchedLines[i]);
                 free(matchedLines);
                 regfree(&regex);
                 fclose(file);
                 return NULL;
            }
            strcpy(matchedLines[*count], line);
            (*count)++;
        }
    }

    fclose(file);
    regfree(&regex);
    return matchedLines;
}

/**
 * @brief Frees the memory allocated by findMatchingLines.
 * @param matchedLines The array of strings to free.
 * @param count The number of strings in the array.
 */
void freeMatchedLines(char** matchedLines, int count) {
    if (matchedLines == NULL) return;
    for (int i = 0; i < count; i++) {
        free(matchedLines[i]);
    }
    free(matchedLines);
}

// Helper function to create a test file
void createTestFile(const char* fileName, const char* content) {
    FILE* f = fopen(fileName, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
    }
}

int main() {
    const char* testFileName = "test_c.txt";
    char** matches = NULL;
    int count = 0;

    // --- Test Cases ---

    // Test Case 1: Standard case with mixed content
    printf("--- Test Case 1: Standard File ---\n");
    const char* content1 = "1. This is a matching line.\n"
                           "Not a match.\n"
                           "2) Also not a match.\n"
                           "3. Another match here.\n"
                           "4 This line does not end with a dot\n"
                           "This line does not start with a number.\n"
                           "55. This one is also a match.";
    createTestFile(testFileName, content1);
    matches = findMatchingLines(testFileName, &count);
    printf("Found %d matches:\n", count);
    for (int i = 0; i < count; i++) printf("%s\n", matches[i]);
    freeMatchedLines(matches, count);
    printf("\n");

    // Test Case 2: Empty file
    printf("--- Test Case 2: Empty File ---\n");
    createTestFile(testFileName, "");
    matches = findMatchingLines(testFileName, &count);
    printf("Found %d matches:\n", count);
    for (int i = 0; i < count; i++) printf("%s\n", matches[i]);
    freeMatchedLines(matches, count);
    printf("\n");

    // Test Case 3: File with no matching lines
    printf("--- Test Case 3: No Matches ---\n");
    const char* content3 = "Hello world\n"
                           "Another line without a dot\n"
                           "Ends with a dot but no number.";
    createTestFile(testFileName, content3);
    matches = findMatchingLines(testFileName, &count);
    printf("Found %d matches:\n", count);
    for (int i = 0; i < count; i++) printf("%s\n", matches[i]);
    freeMatchedLines(matches, count);
    printf("\n");

    // Test Case 4: File where all lines match
    printf("--- Test Case 4: All Lines Match ---\n");
    const char* content4 = "1. First line.\n"
                           "2. Second line.\n"
                           "3. Third line.";
    createTestFile(testFileName, content4);
    matches = findMatchingLines(testFileName, &count);
    printf("Found %d matches:\n", count);
    for (int i = 0; i < count; i++) printf("%s\n", matches[i]);
    freeMatchedLines(matches, count);
    printf("\n");

    // Test Case 5: Non-existent file
    printf("--- Test Case 5: Non-existent File ---\n");
    matches = findMatchingLines("nonexistentfile.txt", &count);
    printf("Found %d matches.\n", count);
    freeMatchedLines(matches, count); // Should be safe to call with NULL

    // Cleanup the test file
    remove(testFileName);

    return 0;
}