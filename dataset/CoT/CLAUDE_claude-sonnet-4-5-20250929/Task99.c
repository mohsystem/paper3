
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 4096
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit
#define MAX_LINES 10000

typedef struct {
    char** lines;
    int count;
} MatchResult;

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int matchesPattern(const char* line) {
    if (line == NULL || strlen(line) == 0) {
        return 0;
    }
    
    // Check if starts with digit
    if (!isDigit(line[0])) {
        return 0;
    }
    
    // Check if ends with dot
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '.') {
        return 1;
    }
    
    return 0;
}

MatchResult matchLines(const char* filePath) {
    MatchResult result;
    result.lines = NULL;
    result.count = 0;
    
    if (filePath == NULL || strlen(filePath) == 0) {
        fprintf(stderr, "Error: File path cannot be null or empty\\n");
        return result;
    }
    
    // Check file size
    struct stat st;
    if (stat(filePath, &st) != 0) {
        fprintf(stderr, "Error: Cannot access file: %s\\n", filePath);
        return result;
    }
    
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        return result;
    }
    
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\\n", filePath);
        return result;
    }
    
    result.lines = (char**)malloc(MAX_LINES * sizeof(char*));
    if (result.lines == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(file);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL && result.count < MAX_LINES) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && (line[len - 1] == '\\n' || line[len - 1] == '\\r')) {
            line[len - 1] = '\\0';
            if (len > 1 && line[len - 2] == '\\r') {
                line[len - 2] = '\\0';
            }
        }
        
        if (matchesPattern(line)) {
            result.lines[result.count] = (char*)malloc(strlen(line) + 1);
            if (result.lines[result.count] != NULL) {
                strcpy(result.lines[result.count], line);
                result.count++;
            }
        }
    }
    
    fclose(file);
    return result;
}

void freeMatchResult(MatchResult* result) {
    if (result != NULL && result->lines != NULL) {
        for (int i = 0; i < result->count; i++) {
            free(result->lines[i]);
        }
        free(result->lines);
        result->lines = NULL;
        result->count = 0;
    }
}

int main() {
    // Test case 1: File with numbered lines
    const char* testFile1 = "test1.txt";
    {
        FILE* f = fopen(testFile1, "w");
        if (f) {
            fprintf(f, "1 This is line one.\\n2 Second line.\\n3 Third line.");
            fclose(f);
            
            printf("Test 1 - File with numbered lines:\\n");
            MatchResult result1 = matchLines(testFile1);
            for (int i = 0; i < result1.count; i++) {
                printf("%s\\n", result1.lines[i]);
            }
            freeMatchResult(&result1);
            remove(testFile1);
        }
    }
    
    // Test case 2: Mixed content
    const char* testFile2 = "test2.txt";
    {
        FILE* f = fopen(testFile2, "w");
        if (f) {
            fprintf(f, "123 Start with number.\\nNo number start.\\n456 Another one.");
            fclose(f);
            
            printf("\\nTest 2 - Mixed content:\\n");
            MatchResult result2 = matchLines(testFile2);
            for (int i = 0; i < result2.count; i++) {
                printf("%s\\n", result2.lines[i]);
            }
            freeMatchResult(&result2);
            remove(testFile2);
        }
    }
    
    // Test case 3: No matching lines
    const char* testFile3 = "test3.txt";
    {
        FILE* f = fopen(testFile3, "w");
        if (f) {
            fprintf(f, "No numbers here\\nAnother line without numbers");
            fclose(f);
            
            printf("\\nTest 3 - No matching lines:\\n");
            MatchResult result3 = matchLines(testFile3);
            printf("Matched lines: %d\\n", result3.count);
            freeMatchResult(&result3);
            remove(testFile3);
        }
    }
    
    // Test case 4: Lines ending without dot
    const char* testFile4 = "test4.txt";
    {
        FILE* f = fopen(testFile4, "w");
        if (f) {
            fprintf(f, "1 Line with dot.\\n2 Line without dot\\n3 Another with dot.");
            fclose(f);
            
            printf("\\nTest 4 - Lines with and without ending dot:\\n");
            MatchResult result4 = matchLines(testFile4);
            for (int i = 0; i < result4.count; i++) {
                printf("%s\\n", result4.lines[i]);
            }
            freeMatchResult(&result4);
            remove(testFile4);
        }
    }
    
    // Test case 5: Invalid file path
    printf("\\nTest 5 - Invalid file path:\\n");
    MatchResult result5 = matchLines("nonexistent_file.txt");
    printf("Matched lines: %d\\n", result5.count);
    freeMatchResult(&result5);
    
    return 0;
}
