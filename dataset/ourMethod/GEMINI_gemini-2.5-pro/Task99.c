#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LEN 4096

/**
 * Reads a file and returns an array of lines that start with numbers and end with a dot.
 *
 * @param filePath The path to the file to read.
 * @param count    A pointer to an integer that will be set to the number of matching lines.
 * @return A dynamically allocated array of strings. The caller must free each string
 *         and then the array itself. Returns NULL on failure.
 */
char** findMatchingLines(const char* filePath, int* count) {
    *count = 0;
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL) {
        return NULL;
    }

    regex_t regex;
    // Using POSIX ERE: ^[0-9]+.*[.]$ is an alternative.
    // We escape '.' as it is a special character in regex.
    int reti = regcomp(&regex, "^[0-9]+.*\\.$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        fclose(fp);
        return NULL;
    }

    int capacity = 10;
    char** matches = malloc(capacity * sizeof(char*));
    if (matches == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        regfree(&regex);
        fclose(fp);
        return NULL;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Remove trailing newline character, if present
        line[strcspn(line, "\r\n")] = 0;

        reti = regexec(&regex, line, 0, NULL, 0);
        if (reti == 0) { // Match found
            if (*count >= capacity) {
                capacity *= 2;
                char** new_matches = realloc(matches, capacity * sizeof(char*));
                if (new_matches == NULL) {
                    fprintf(stderr, "Memory reallocation failed\n");
                    // Free existing memory before returning
                    for (int i = 0; i < *count; i++) {
                        free(matches[i]);
                    }
                    free(matches);
                    regfree(&regex);
                    fclose(fp);
                    return NULL;
                }
                matches = new_matches;
            }
            matches[*count] = strdup(line);
            if(matches[*count] == NULL){
                 fprintf(stderr, "strdup failed\n");
                 // clean up and exit
                 for(int i = 0; i < *count; i++){
                    free(matches[i]);
                 }
                 free(matches);
                 regfree(&regex);
                 fclose(fp);
                 return NULL;
            }
            (*count)++;
        }
    }

    fclose(fp);
    regfree(&regex);
    return matches;
}

void runTestCase(int testNum, const char* fileName, const char* fileContent) {
    printf("--- Test Case %d ---\n", testNum);
    
    FILE* outFile = fopen(fileName, "w");
    if (!outFile) {
        fprintf(stderr, "Failed to create test file: %s\n", fileName);
        return;
    }
    fprintf(outFile, "%s", fileContent);
    fclose(outFile);

    printf("File content:\n\"\"\"\n%s\"\"\"\n", fileContent);
    
    int match_count = 0;
    char** results = findMatchingLines(fileName, &match_count);

    if (results != NULL || match_count == 0) {
        printf("Matching lines found: %d\n", match_count);
        for (int i = 0; i < match_count; i++) {
            printf(" > %s\n", results[i]);
            free(results[i]); // Free each string
        }
        free(results); // Free the array
    } else {
         printf("Function failed or file could not be opened.\n");
    }

    remove(fileName);
    printf("\n");
}

int main() {
    const char* fileName = "test_file_c.txt";

    // Test Case 1: Mixed content
    const char* content1 = "123 this is a test.\n"
                           "456 another line that matches.\n"
                           "not a match\n"
                           "789 this line does not end with a dot\n"
                           "and another line that does not start with a number.\n"
                           "999.\n"
                           "1.";
    runTestCase(1, fileName, content1);

    // Test Case 2: All lines match
    const char* content2 = "1. First line.\n"
                           "20. Second line.\n"
                           "300. Third line.";
    runTestCase(2, fileName, content2);
    
    // Test Case 3: No lines match
    const char* content3 = "This starts with text.\n"
                           "123 this ends with no dot\n"
                           "Ends with a dot but no number.";
    runTestCase(3, fileName, content3);
    
    // Test Case 4: Empty file
    const char* content4 = "";
    runTestCase(4, fileName, content4);
    
    // Test Case 5: Lines with special regex characters
    const char* content5 = "123.*.\n"
                           "456?[]\\d+.\n"
                           "abc.";
    runTestCase(5, fileName, content5);

    return 0;
}