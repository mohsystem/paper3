#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * Reads a file and finds lines that start with numbers and end with a dot.
 * Note: The caller is responsible for freeing the returned array and its contents.
 *
 * @param filePath The path to the file.
 * @param count    A pointer to an integer to store the number of matches found.
 * @return A dynamically allocated array of strings containing the matching lines, or NULL on error.
 */
char** findMatchingLines(const char* filePath, int* count) {
    *count = 0;
    FILE* file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Regex: ^[0-9]+ -> starts with one or more digits
    //        .*      -> followed by any character, zero or more times
    //        \.      -> a literal dot
    //        $       -> end of the line
    const char* pattern = "^[0-9]+.*\\.$";
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(file);
        return NULL;
    }

    char** matches = NULL;
    size_t capacity = 0;
    
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // Use getline for safe reading of lines of any length
    while ((read = getline(&line, &len, file)) != -1) {
        // Remove trailing newline character if it exists
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            // Reallocate memory for the matches array if needed
            if (*count >= capacity) {
                capacity = (capacity == 0) ? 8 : capacity * 2;
                char** new_matches = realloc(matches, capacity * sizeof(char*));
                if (!new_matches) {
                    perror("Failed to reallocate memory");
                    // Free previously allocated resources
                    for (int i = 0; i < *count; ++i) free(matches[i]);
                    free(matches);
                    free(line);
                    regfree(&regex);
                    fclose(file);
                    return NULL;
                }
                matches = new_matches;
            }
            // strdup allocates memory for the string copy
            matches[*count] = strdup(line);
            if (!matches[*count]) {
                 perror("Failed to duplicate string");
            }
            (*count)++;
        }
    }

    // Clean up
    free(line);
    regfree(&regex);
    fclose(file);

    return matches;
}

void free_matches(char** matches, int count) {
    if (matches) {
        for (int i = 0; i < count; ++i) {
            free(matches[i]);
        }
        free(matches);
    }
}

int main() {
    const char* testFileName = "test_Task99.txt";

    // Create a test file with some content
    FILE* writer = fopen(testFileName, "w");
    if (!writer) {
        perror("Error creating test file");
        return 1;
    }
    fprintf(writer, "1. This is the first test case.\n");
    fprintf(writer, "This line does not start with a number.\n");
    fprintf(writer, "2. Second test case.\n");
    fprintf(writer, "3 A line without a dot at the end\n");
    fprintf(writer, "4. Test case three is here.\n");
    fprintf(writer, "Another non-matching line.\n");
    fprintf(writer, "55. Test case four.\n");
    fprintf(writer, "99999. Final test case.\n");
    fprintf(writer, "The end.\n");
    fprintf(writer, "6 is not a match either\n");
    fclose(writer);

    printf("--- Reading from file: %s ---\n", testFileName);
    int match_count = 0;
    char** matched = findMatchingLines(testFileName, &match_count);

    if (matched) {
        printf("Found %d matching lines:\n", match_count);
        for (int i = 0; i < match_count; ++i) {
            printf("%s\n", matched[i]);
        }
        // Free the allocated memory
        free_matches(matched, match_count);
    }
    
    // Clean up the test file
    remove(testFileName);

    return 0;
}