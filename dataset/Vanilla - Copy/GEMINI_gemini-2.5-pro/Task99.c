#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * @brief Reads a file and prints lines that start with a number and end with a dot.
 *        Output is printed directly to avoid complex memory management in C.
 * @param filePath The path to the input file.
 */
void findMatchingLines(const char* filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    regex_t regex;
    int reti;
    // POSIX ERE pattern: ^[0-9] is start with a digit, .* is anything, \\.$ is a literal dot at the end.
    const char* pattern = "^[0-9].*\\.$"; 

    // Compile the regular expression
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Read the file line by line
    while ((read = getline(&line, &len, file)) != -1) {
        // Remove trailing newline character, if it exists
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        // Execute the regular expression
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) { // 0 means a match was found
            puts(line);
        }
    }
    
    regfree(&regex);
    if (line) {
        free(line);
    }
    fclose(file);
}

/**
 * @brief Helper function to run a single test case.
 * 
 * @param fileName The name of the temporary file to create.
 * @param content The content to write to the file.
 */
void runTestCase(const char* fileName, const char* content) {
    FILE *f = fopen(fileName, "w");
    if (f == NULL) {
        printf("Failed to create test file: %s\n", fileName);
        return;
    }
    fprintf(f, "%s", content);
    fclose(f);

    printf("\n--- Testing with file: %s ---\n", fileName);
    printf("Content:\n\"");
    const char *p = content;
    while (*p) {
        if (*p == '\n') {
            printf("\\n");
        } else {
            putchar(*p);
        }
        p++;
    }
    printf("\"\n");
    
    printf("Result:\n");
    findMatchingLines(fileName);

    remove(fileName);
}

int main() {
    // Test Case 1: A file with a mix of valid and invalid lines.
    runTestCase("test_c_1.txt",
                "1. This is a valid line.\n"
                "This line is not valid.\n"
                "2. This is also a valid line.\n"
                "3 This line is not valid, no dot at the end\n"
                "4.Valid line.\n"
                "5.\n"
                "Invalid line.\n"
                "6. Another. valid. line.");

    // Test Case 2: An empty file.
    runTestCase("test_c_2.txt", "");

    // Test Case 3: A file with no matching lines.
    runTestCase("test_c_3.txt", "Hello world\nThis is a test\nNo lines match here");

    // Test Case 4: A file where all lines match.
    runTestCase("test_c_4.txt", "1. First.\n2. Second.\n3. Third.");

    // Test Case 5: A file with special characters and multiple digits.
    runTestCase("test_c_5.txt", "123. Special chars !@#$%^&*().\nAnother line\n45.Ends with a dot.");

    return 0;
}