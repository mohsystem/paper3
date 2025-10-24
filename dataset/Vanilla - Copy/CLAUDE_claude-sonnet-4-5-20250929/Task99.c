
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 1024
#define MAX_MATCHES 100

typedef struct {
    char** lines;
    int count;
} MatchResult;

MatchResult matchLines(const char* filename) {
    MatchResult result;
    result.lines = (char**)malloc(MAX_MATCHES * sizeof(char*));
    result.count = 0;
    
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]+.*\\\\.$", REG_EXTENDED);
    
    if (reti) {
        fprintf(stderr, "Could not compile regex\\n");
        return result;
    }
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        regfree(&regex);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\\n') {
            line[len - 1] = '\\0';
        }
        
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti && result.count < MAX_MATCHES) {
            result.lines[result.count] = (char*)malloc((strlen(line) + 1) * sizeof(char));
            strcpy(result.lines[result.count], line);
            result.count++;
        }
    }
    
    fclose(file);
    regfree(&regex);
    return result;
}

void freeMatchResult(MatchResult* result) {
    for (int i = 0; i < result->count; i++) {
        free(result->lines[i]);
    }
    free(result->lines);
}

int main() {
    // Test file names
    const char* testFiles[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    const char* testData[][4] = {
        {"123 This is a test.", "456 Another line.", "Not matching", "789 End with dot."},
        {"1 First line.", "Second line", "2 Third line.", "3 Fourth."},
        {"99 Single digit.", "100 Triple digits.", "No number here.", "5."},
        {"0 Zero start.", "abc", "42 The answer.", ""},
        {"7777 Multiple digits here.", "8888.", "Not starting with number.", "9 Final."}
    };
    
    // Create test files
    for (int i = 0; i < 5; i++) {
        FILE* file = fopen(testFiles[i], "w");
        if (file != NULL) {
            for (int j = 0; j < 4; j++) {
                fprintf(file, "%s\\n", testData[i][j]);
            }
            fclose(file);
        }
    }
    
    // Test cases
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d - File: %s\\n", i + 1, testFiles[i]);
        MatchResult results = matchLines(testFiles[i]);
        printf("Matched lines:\\n");
        for (int j = 0; j < results.count; j++) {
            printf("  %s\\n", results.lines[j]);
        }
        printf("\\n");
        freeMatchResult(&results);
    }
    
    return 0;
}
