
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1048576  // 1MB per line
#define MAX_LINES 1000000        // Maximum lines to prevent memory exhaustion
#define MAX_PATH_LENGTH 4096     // Maximum file path length

// Security: Validate file path to prevent directory traversal
bool isValidFilePath(const char* filepath) {
    // Security: Check for NULL pointer
    if (filepath == NULL) {
        return false;
    }
    
    // Security: Validate length
    size_t len = strnlen(filepath, MAX_PATH_LENGTH + 1);
    if (len == 0 || len > MAX_PATH_LENGTH) {
        return false;
    }
    
    // Security: Check for directory traversal patterns
    if (strstr(filepath, "..") != NULL) {
        return false;
    }
    
    // Security: Check for null bytes in path (truncation attack)
    if (memchr(filepath, '\\0', len) != filepath + len) {
        return false;
    }
    
    return true;
}

// Structure to hold lines
typedef struct {
    char** lines;
    size_t count;
    size_t capacity;
} LineArray;

// Security: Initialize line array with null checks
LineArray* createLineArray(void) {
    // Security: Check malloc return value
    LineArray* arr = (LineArray*)malloc(sizeof(LineArray));
    if (arr == NULL) {
        return NULL;
    }
    
    arr->capacity = 100;
    // Security: Check malloc return value
    arr->lines = (char**)malloc(arr->capacity * sizeof(char*));
    if (arr->lines == NULL) {
        free(arr);
        return NULL;
    }
    
    arr->count = 0;
    // Security: Initialize all pointers to NULL
    for (size_t i = 0; i < arr->capacity; i++) {
        arr->lines[i] = NULL;
    }
    
    return arr;
}

// Security: Safe memory cleanup
void freeLineArray(LineArray* arr) {
    if (arr == NULL) {
        return;
    }
    
    // Security: Free each line
    if (arr->lines != NULL) {
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->lines[i] != NULL) {
                // Security: Clear sensitive data before freeing (if needed)
                memset(arr->lines[i], 0, strlen(arr->lines[i]));
                free(arr->lines[i]);
                arr->lines[i] = NULL;
            }
        }
        free(arr->lines);
        arr->lines = NULL;
    }
    
    free(arr);
}

// Security: Add line with bounds checking
bool addLine(LineArray* arr, const char* line) {
    // Security: Validate inputs
    if (arr == NULL || line == NULL) {
        return false;
    }
    
    // Security: Prevent memory exhaustion
    if (arr->count >= MAX_LINES) {
        return false;
    }
    
    // Security: Grow array if needed
    if (arr->count >= arr->capacity) {
        // Security: Check for overflow before multiplication
        if (arr->capacity > SIZE_MAX / 2) {
            return false;
        }
        
        size_t new_capacity = arr->capacity * 2;
        // Security: Check realloc return value
        char** new_lines = (char**)realloc(arr->lines, new_capacity * sizeof(char*));
        if (new_lines == NULL) {
            return false;
        }
        
        arr->lines = new_lines;
        // Security: Initialize new pointers
        for (size_t i = arr->capacity; i < new_capacity; i++) {
            arr->lines[i] = NULL;
        }
        arr->capacity = new_capacity;
    }
    
    // Security: Validate line length
    size_t line_len = strnlen(line, MAX_LINE_LENGTH + 1);
    if (line_len > MAX_LINE_LENGTH) {
        return false;
    }
    
    // Security: Allocate memory for line copy, check for overflow
    if (line_len >= SIZE_MAX) {
        return false;
    }
    
    // Security: Check malloc return value
    arr->lines[arr->count] = (char*)malloc(line_len + 1);
    if (arr->lines[arr->count] == NULL) {
        return false;
    }
    
    // Security: Use safe string copy with explicit null termination
    strncpy(arr->lines[arr->count], line, line_len);
    arr->lines[arr->count][line_len] = '\\0';
    arr->count++;
    
    return true;
}

// Security: Safe file reading with bounds checking
LineArray* readFileSafely(const char* filepath) {
    // Security: Validate file path
    if (!isValidFilePath(filepath)) {
        fprintf(stderr, "Invalid file path\\n");
        return NULL;
    }
    
    // Security: Open file in read-only mode
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\\n", filepath);
        return NULL;
    }
    
    LineArray* lines = createLineArray();
    if (lines == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Security: Use stack buffer with bounds checking
    char buffer[8192];
    
    // Security: Read with fgets (safe, prevents overflow)
    while (fgets(buffer, sizeof(buffer), file) != NULL && lines->count < MAX_LINES) {
        // Security: Validate buffer read
        size_t len = strnlen(buffer, sizeof(buffer));
        
        // Remove newline if present
        if (len > 0 && buffer[len - 1] == '\\n') {
            buffer[len - 1] = '\\0';
        }
        
        // Security: Add line with bounds checking
        if (!addLine(lines, buffer)) {
            fprintf(stderr, "Failed to add line\\n");
            break;
        }
    }
    
    fclose(file);
    return lines;
}

// Match lines using regex
LineArray* matchLines(const LineArray* lines) {
    if (lines == NULL) {
        return NULL;
    }
    
    LineArray* matched = createLineArray();
    if (matched == NULL) {
        return NULL;
    }
    
    regex_t regex;
    // Pattern: starts with digits, ends with dot
    // Security: Use safe regex pattern
    int ret = regcomp(&regex, "^[0-9].*\\\\.$", REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        fprintf(stderr, "Regex compilation failed\\n");
        freeLineArray(matched);
        return NULL;
    }
    
    for (size_t i = 0; i < lines->count; i++) {
        // Security: Validate pointer before use
        if (lines->lines[i] == NULL) {
            continue;
        }
        
        ret = regexec(&regex, lines->lines[i], 0, NULL, 0);
        if (ret == 0) {
            // Match found
            if (!addLine(matched, lines->lines[i])) {
                fprintf(stderr, "Failed to add matched line\\n");
                break;
            }
        }
    }
    
    regfree(&regex);
    return matched;
}

int main(void) {
    // Test case 1: Basic file with matching lines
    {
        const char* filename = "test1.txt";
        FILE* out = fopen(filename, "w");
        if (out != NULL) {
            fprintf(out, "123 This line starts with numbers and ends with a dot.\\n");
            fprintf(out, "This line does not start with numbers.\\n");
            fprintf(out, "456 Another matching line.\\n");
            fprintf(out, "789 No dot at the end\\n");
            fprintf(out, "0 Single digit.\\n");
            fclose(out);
            
            LineArray* lines = readFileSafely(filename);
            if (lines != NULL) {
                LineArray* matched = matchLines(lines);
                if (matched != NULL) {
                    printf("Test 1 - Matched lines:\\n");
                    for (size_t i = 0; i < matched->count; i++) {
                        printf("  %s\\n", matched->lines[i]);
                    }
                    freeLineArray(matched);
                }
                freeLineArray(lines);
            }
        }
    }
    
    // Test case 2: Empty file
    {
        const char* filename = "test2.txt";
        FILE* out = fopen(filename, "w");
        if (out != NULL) {
            fclose(out);
            
            LineArray* lines = readFileSafely(filename);
            if (lines != NULL) {
                LineArray* matched = matchLines(lines);
                if (matched != NULL) {
                    printf("Test 2 - Matched lines: %zu\\n", matched->count);
                    freeLineArray(matched);
                }
                freeLineArray(lines);
            }
        }
    }
    
    // Test case 3: No matching lines
    {
        const char* filename = "test3.txt";
        FILE* out = fopen(filename, "w");
        if (out != NULL) {
            fprintf(out, "No numbers at start.\\n");
            fprintf(out, "Also no numbers here\\n");
            fclose(out);
            
            LineArray* lines = readFileSafely(filename);
            if (lines != NULL) {
                LineArray* matched = matchLines(lines);
                if (matched != NULL) {
                    printf("Test 3 - Matched lines: %zu\\n", matched->count);
                    freeLineArray(matched);
                }
                freeLineArray(lines);
            }
        }
    }
    
    // Test case 4: Invalid file path
    {
        LineArray* lines = readFileSafely("../etc/passwd");
        printf("Test 4 - Lines read (should be NULL): %p\\n", (void*)lines);
        if (lines != NULL) {
            freeLineArray(lines);
        }
    }
    
    // Test case 5: Multiple digits and special characters
    {
        const char* filename = "test5.txt";
        FILE* out = fopen(filename, "w");
        if (out != NULL) {
            fprintf(out, "999 Line with special chars !@#$%%.\\n");
            fprintf(out, "42.\\n");
            fprintf(out, "100 Multiword sentence here.\\n");
            fclose(out);
            
            LineArray* lines = readFileSafely(filename);
            if (lines != NULL) {
                LineArray* matched = matchLines(lines);
                if (matched != NULL) {
                    printf("Test 5 - Matched lines:\\n");
                    for (size_t i = 0; i < matched->count; i++) {
                        printf("  %s\\n", matched->lines[i]);
                    }
                    freeLineArray(matched);
                }
                freeLineArray(lines);
            }
        }
    }
    
    return 0;
}
