
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit
#define MAX_PATH_LENGTH 4096
#define MAX_PATTERN_LENGTH 1024
#define BUFFER_SIZE 8192
#define MAX_MATCHES 1000

// Security: Validates that path doesn't contain traversal attempts or dangerous patterns\nbool isValidPath(const char* path) {\n    if (path == NULL) {\n        return false;\n    }\n    \n    size_t len = strnlen(path, MAX_PATH_LENGTH + 1);\n    \n    // Reject empty or too long paths\n    if (len == 0 || len > MAX_PATH_LENGTH) {\n        return false;\n    }\n    \n    // Reject absolute paths\n    if (path[0] == '/') {\n        return false;\n    }\n    \n    // Check for path traversal attempts\n    if (strstr(path, "..") != NULL) {\n        return false;\n    }\n    \n    // Check for double slashes\n    if (strstr(path, "//") != NULL) {\n        return false;\n    }\n    \n    // Check for null bytes within string (not at end)\n    for (size_t i = 0; i < len; i++) {\n        if (path[i] == '\\0' && i < len - 1) {\n            return false;\n        }\n    }\n    \n    return true;\n}\n\n// Security: Validates regex pattern to prevent issues\nbool isValidRegexPattern(const char* pattern) {\n    if (pattern == NULL) {\n        return false;\n    }\n    \n    size_t len = strnlen(pattern, MAX_PATTERN_LENGTH + 1);\n    \n    // Reject empty or too long patterns\n    if (len == 0 || len > MAX_PATTERN_LENGTH) {\n        return false;\n    }\n    \n    // Check for null bytes within string\n    for (size_t i = 0; i < len; i++) {\n        if (pattern[i] == '\\0' && i < len - 1) {\n            return false;\n        }\n    }\n    \n    // Basic check for nested quantifiers (ReDoS prevention)\n    int nestedQuantifiers = 0;\n    for (size_t i = 0; i < len; i++) {\n        if (pattern[i] == '*' || pattern[i] == '+' || pattern[i] == '{') {\n            nestedQuantifiers++;\n            if (nestedQuantifiers > 10) {\n                return false;\n            }\n        }\n    }\n    \n    return true;\n}\n\n// Security: Safe file reading with bounds checking and size limits\nchar* readFileContent(const char* filename, size_t* contentSize) {\n    FILE* file = NULL;\n    char* content = NULL;\n    char* tempBuffer = NULL;\n    size_t totalRead = 0;\n    size_t currentCapacity = 0;\n    \n    // Security: Initialize output parameter\n    if (contentSize != NULL) {\n        *contentSize = 0;\n    } else {\n        return NULL;\n    }\n    \n    // Security: Validate path before opening\n    if (!isValidPath(filename)) {\n        fprintf(stderr, "Error: Invalid file path\\n");\n        return NULL;\n    }\n    \n    // Security: Open file with explicit read mode, use "r" not "rb" for text, but "rb" is safer for regex\n    file = fopen(filename, "rb");\n    if (file == NULL) {\n        fprintf(stderr, "Error: Cannot open file: %s\\n", filename);\n        return NULL;\n    }\n    \n    // Security: Get file size with error checking\n    if (fseek(file, 0, SEEK_END) != 0) {\n        fprintf(stderr, "Error: Cannot seek file\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    long fileSize = ftell(file);\n    if (fileSize < 0 || (size_t)fileSize > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: File size invalid or exceeds limit\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    if (fseek(file, 0, SEEK_SET) != 0) {\n        fprintf(stderr, "Error: Cannot seek to beginning\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    // Security: Allocate initial buffer with size check\n    currentCapacity = (size_t)fileSize + 1; // +1 for null terminator\n    content = (char*)calloc(currentCapacity, sizeof(char));\n    if (content == NULL) {\n        fprintf(stderr, "Error: Memory allocation failed\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    // Security: Read file with bounds checking\n    char buffer[BUFFER_SIZE];\n    size_t bytesRead;\n    \n    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {\n        // Security: Check for overflow before addition\n        if (totalRead > SIZE_MAX - bytesRead || totalRead + bytesRead >= MAX_FILE_SIZE) {\n            fprintf(stderr, "Error: File size limit exceeded\\n");\n            free(content);\n            fclose(file);\n            return NULL;\n        }\n        \n        // Security: Ensure buffer has space (including null terminator)\n        if (totalRead + bytesRead >= currentCapacity) {\n            size_t newCapacity = currentCapacity * 2;\n            if (newCapacity > MAX_FILE_SIZE + 1) {\n                newCapacity = MAX_FILE_SIZE + 1;\n            }\n            \n            // Security: Check realloc return value\n            tempBuffer = (char*)realloc(content, newCapacity);\n            if (tempBuffer == NULL) {\n                fprintf(stderr, "Error: Memory reallocation failed\\n");\n                free(content);\n                fclose(file);\n                return NULL;\n            }\n            content = tempBuffer;\n            currentCapacity = newCapacity;\n        }\n        \n        // Security: Bounds-checked copy\n        memcpy(content + totalRead, buffer, bytesRead);\n        totalRead += bytesRead;\n    }\n    \n    // Security: Check for read errors\n    if (ferror(file)) {\n        fprintf(stderr, "Error: File read error\\n");\n        free(content);\n        fclose(file);\n        return NULL;\n    }\n    \n    fclose(file);\n    \n    // Security: Null terminate the content\n    content[totalRead] = '\\0';
    *contentSize = totalRead;
    
    return content;
}

void searchWithRegex(const char* pattern, const char* filename) {
    regex_t regex;
    regmatch_t match;
    char* content = NULL;
    size_t contentSize = 0;
    int reti;
    
    // Security: Validate inputs
    if (!isValidRegexPattern(pattern)) {
        fprintf(stderr, "Error: Invalid regex pattern\\n");
        return;
    }
    
    // Security: Read file with bounds checking
    content = readFileContent(filename, &contentSize);
    if (content == NULL) {
        return;
    }
    
    // Security: Compile regex with error handling
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti != 0) {
        char errorBuffer[256];
        regerror(reti, &regex, errorBuffer, sizeof(errorBuffer));
        fprintf(stderr, "Error: Could not compile regex: %s\\n", errorBuffer);
        // Security: Free content before returning
        free(content);
        return;
    }
    
    // Search for matches
    int matchCount = 0;
    const char* searchPtr = content;
    size_t remainingSize = contentSize;
    
    while (matchCount < MAX_MATCHES && remainingSize > 0) {
        reti = regexec(&regex, searchPtr, 1, &match, 0);
        
        if (reti == 0) {
            // Match found
            // Security: Validate match offsets
            if (match.rm_so >= 0 && match.rm_eo >= match.rm_so && 
                (size_t)match.rm_eo <= remainingSize) {
                
                int matchLen = match.rm_eo - match.rm_so;
                printf("Match found: %.*s\\n", matchLen, searchPtr + match.rm_so);
                matchCount++;
                
                // Move search pointer past this match
                size_t offset = (size_t)match.rm_eo;
                if (offset == 0) offset = 1; // Prevent infinite loop on zero-length matches
                
                // Security: Check bounds before pointer arithmetic
                if (offset > remainingSize) {
                    break;
                }
                searchPtr += offset;
                remainingSize -= offset;
            } else {
                break;
            }
        } else if (reti == REG_NOMATCH) {
            break;
        } else {
            char errorBuffer[256];
            regerror(reti, &regex, errorBuffer, sizeof(errorBuffer));
            fprintf(stderr, "Error: Regex match failed: %s\\n", errorBuffer);
            break;
        }
    }
    
    if (matchCount == 0) {
        printf("No matches found.\\n");
    } else {
        printf("Total matches: %d\\n", matchCount);
    }
    
    // Security: Clean up resources
    regfree(&regex);
    free(content);
}

int main(int argc, char* argv[]) {
    // Security: Validate argument count
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <regex_pattern> <filename>\\n", 
                argv[0] != NULL ? argv[0] : "program");
        return 1;
    }
    
    // Security: Validate argv pointers
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "Error: Invalid arguments\\n");
        return 1;
    }
    
    // Security: Validate argument lengths
    size_t patternLen = strnlen(argv[1], MAX_PATTERN_LENGTH + 1);
    size_t filenameLen = strnlen(argv[2], MAX_PATH_LENGTH + 1);
    
    if (patternLen == 0 || patternLen > MAX_PATTERN_LENGTH ||
        filenameLen == 0 || filenameLen > MAX_PATH_LENGTH) {
        fprintf(stderr, "Error: Argument length invalid\\n");
        return 1;
    }
    
    printf("=== Test Case: User provided arguments ===\\n");
    searchWithRegex(argv[1], argv[2]);
    
    return 0;
}
