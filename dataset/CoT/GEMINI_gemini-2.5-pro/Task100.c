#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * @brief Searches a file for lines matching a given POSIX regular expression.
 * 
 * @param regexPattern The regular expression pattern string.
 * @param fileName The path to the file to be searched.
 */
void searchFileWithRegex(const char* regexPattern, const char* fileName) {
    regex_t regex;
    int reti;
    char msgbuf[100];

    // Step 1: Compile the regular expression
    reti = regcomp(&regex, regexPattern, REG_EXTENDED);
    if (reti) {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Error: Regex compilation failed: %s\n", msgbuf);
        return;
    }

    // Step 2: Open the file
    FILE* fp = fopen(fileName, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file: %s\n", fileName);
        regfree(&regex); // Free regex memory on error
        return;
    }

    // Step 3: Read file line by line and search for matches
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNumber = 1;
    int found = 0;

    while ((read = getline(&line, &len, fp)) != -1) {
        // Remove trailing newline character if it exists
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) { // 0 means a match was found
            printf("Match found on line %d: %s\n", lineNumber, line);
            found = 1;
        } else if (reti != REG_NOMATCH) {
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed on line %d: %s\n", lineNumber, msgbuf);
        }
        lineNumber++;
    }

    if (!found) {
        printf("No matches found for pattern '%s' in file '%s'.\n", regexPattern, fileName);
    }

    // Step 4: Clean up resources
    fclose(fp);
    if (line) {
        free(line);
    }
    regfree(&regex);
}

void runTests() {
    const char* testFileName = "test_file.txt";
    // Create a temporary test file
    FILE* fp = fopen(testFileName, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create test file.\n");
        return;
    }
    fprintf(fp, "Hello world, this is a test file.\n");
    fprintf(fp, "The quick brown fox jumps over the lazy dog.\n");
    fprintf(fp, "123-456-7890 is a phone number.\n");
    fprintf(fp, "Another line with the word world.\n");
    fprintf(fp, "Email: test@example.com\n");
    fprintf(fp, "invalid-email@.com\n");
    fclose(fp);

    // Note: C POSIX regex uses a slightly different syntax
    const char* testCases[][2] = {
        {"world", "Test Case 1: Simple word match"},
        {"^[A-Z]", "Test Case 2: Match lines starting with a capital letter"},
        {"[0-9]{3}-[0-9]{3}-[0-9]{4}", "Test Case 3: Match a phone number format"},
        {"(fox|dog)", "Test Case 4: Match using alternation"},
        {"[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", "Test Case 5: Match a valid email address"}
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; i++) {
        const char* pattern = testCases[i][0];
        const char* description = testCases[i][1];
        printf("\n--- %s ---\n", description);
        printf("Pattern: %s\n", pattern);
        printf("------------------------------------\n");
        searchFileWithRegex(pattern, testFileName);
    }

    // Clean up the test file
    remove(testFileName);
}

int main(int argc, char* argv[]) {
    // Check if command line arguments are provided
    if (argc == 3) {
        printf("--- Running with Command Line Arguments ---\n");
        const char* regex = argv[1];
        const char* file = argv[2];
        printf("Pattern: %s\n", regex);
        printf("File: %s\n", file);
        printf("-------------------------------------------\n");
        searchFileWithRegex(regex, file);
    } else {
        printf("Usage: %s <regex_pattern> <file_name>\n", argv[0]);
        printf("Running built-in test cases as no arguments were provided.\n");
        runTests();
    }
    return 0;
}