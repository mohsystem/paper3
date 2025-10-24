
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 1024
#define MAX_MATCHES 100

typedef struct {
    char lines[MAX_MATCHES][MAX_LINE_LENGTH];
    int count;
} MatchedLines;

MatchedLines matchLines(const char* filename) {
    MatchedLines result;
    result.count = 0;
    
    regex_t regex;
    int reti;
    
    // Compile regex: starts with digit(s), ends with dot
    reti = regcomp(&regex, "^[0-9]+.*\\\\.$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\\n");
        return result;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error reading file: %s\\n", filename);
        regfree(&regex);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && result.count < MAX_MATCHES) {
        // Remove newline
        line[strcspn(line, "\\n")] = 0;
        
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) {
            strcpy(result.lines[result.count], line);
            result.count++;
        }
    }
    
    fclose(file);
    regfree(&regex);
    return result;
}

int main() {
    // Test cases - creating test files
    const char* testFiles[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    
    // Test case 1: Simple numbered list
    FILE* f1 = fopen(testFiles[0], "w");
    fprintf(f1, "1 This is a sentence.\\n");
    fprintf(f1, "2 Another line.\\n");
    fprintf(f1, "Not matching\\n");
    fprintf(f1, "3 Third line.\\n");
    fclose(f1);
    
    // Test case 2: Multi-digit numbers
    FILE* f2 = fopen(testFiles[1], "w");
    fprintf(f2, "123 Large number line.\\n");
    fprintf(f2, "456 Another large number.\\n");
    fprintf(f2, "No number here.\\n");
    fclose(f2);
    
    // Test case 3: Lines without dots
    FILE* f3 = fopen(testFiles[2], "w");
    fprintf(f3, "1 This has no dot\\n");
    fprintf(f3, "2 This has a dot.\\n");
    fprintf(f3, "3 No dot here\\n");
    fclose(f3);
    
    // Test case 4: Mixed content
    FILE* f4 = fopen(testFiles[3], "w");
    fprintf(f4, "99 Special characters !@#$.\\n");
    fprintf(f4, "Start with text 100.\\n");
    fprintf(f4, "0 Zero starts this.\\n");
    fclose(f4);
    
    // Test case 5: Empty and edge cases
    FILE* f5 = fopen(testFiles[4], "w");
    fprintf(f5, "1.\\n");
    fprintf(f5, "2\\n");
    fprintf(f5, ".\\n");
    fprintf(f5, "12345 Multiple words here.\\n");
    fclose(f5);
    
    // Execute test cases
    for (int i = 0; i < 5; i++) {
        printf("Test case %d - File: %s\\n", i + 1, testFiles[i]);
        MatchedLines results = matchLines(testFiles[i]);
        printf("Matched lines: ");
        for (int j = 0; j < results.count; j++) {
            printf("\\"%s\\" ", results.lines[j]);
        }
        printf("\\n\\n");
    }
    
    return 0;
}
