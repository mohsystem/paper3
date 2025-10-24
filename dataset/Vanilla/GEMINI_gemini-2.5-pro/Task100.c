#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * Searches for a regex pattern within a file and prints matching lines.
 * Uses POSIX regular expressions.
 * @param patternStr The regular expression pattern to search for.
 * @param fileName The path to the file to search in.
 */
void search_in_file(const char* patternStr, const char* fileName) {
    printf("Searching for pattern '%s' in file '%s'...\n", patternStr, fileName);

    regex_t regex;
    int reti;
    char msgbuf[100];

    // Compile regular expression
    reti = regcomp(&regex, patternStr, REG_EXTENDED);
    if (reti) {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", msgbuf);
        printf("------------------------------------\n");
        return;
    }

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error opening file");
        regfree(&regex);
        printf("------------------------------------\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        // Execute regular expression
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) {
            // Remove trailing newline character for cleaner printing
            if (line[read - 1] == '\n') {
                line[read - 1] = '\0';
            }
            printf("Found match: %s\n", line);
        } else if (reti != REG_NOMATCH) {
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        }
    }

    // Free allocated resources
    fclose(fp);
    if (line) {
        free(line);
    }
    regfree(&regex);
    printf("------------------------------------\n");
}


/**
 * Sets up test data and runs test cases.
 */
void run_test_cases() {
    const char* test_filename = "test_file_c.txt";
    FILE* fp = fopen(test_filename, "w");
    if (fp == NULL) {
        perror("Failed to create test file");
        return;
    }

    fprintf(fp, "Hello world, this is a test file.\n");
    fprintf(fp, "The year is 2024.\n");
    fprintf(fp, "Contact us at test@example.com for more info.\n");
    fprintf(fp, "Or call 123-456-7890.\n");
    fprintf(fp, "This line contains no special characters or numbers\n");
    fprintf(fp, "Another email: another.email@domain.org.\n");
    fclose(fp);

    // Test Case 1: Match a 4-digit number (POSIX ERE style)
    search_in_file("[0-9]{4}", test_filename);

    // Test Case 2: Match an email address (POSIX ERE style)
    search_in_file("[_A-Za-z0-9\\.\\-]+@[A-Za-z0-9\\-]+\\.[A-Za-z]{2,}", test_filename);

    // Test Case 3: Match a line starting with "Hello"
    search_in_file("^Hello", test_filename);

    // Test Case 4: Match a line ending with "numbers"
    search_in_file("numbers$", test_filename);
    
    // Test Case 5: A pattern that won't match anything
    search_in_file("NonExistentPatternXYZ", test_filename);

    // Cleanup the test file
    remove(test_filename);
}


int main(int argc, char *argv[]) {
    if (argc == 3) {
        // Use command-line arguments
        search_in_file(argv[1], argv[2]);
    } else {
        // Run built-in test cases
        printf("Usage: %s <regex_pattern> <file_name>\n", argv[0]);
        printf("Running test cases instead...\n\n");
        run_test_cases();
    }
    return 0;
}