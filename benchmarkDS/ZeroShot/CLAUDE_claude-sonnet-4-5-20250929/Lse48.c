
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#define ALLOWED_DIRECTORY "./images/"
#define MAX_PATH_LEN 512
#define MAX_FILE_SIZE 10485760  // 10MB limit

char* sanitize_filename(const char* filename) {
    if (!filename) return NULL;
    
    size_t len = strlen(filename);
    char* sanitized = (char*)malloc(len + 1);
    if (!sanitized) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = filename[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            sanitized[j++] = c;
        }
    }
    sanitized[j] = '\\0';
    
    return sanitized;
}

int is_safe_path(const char* filepath) {
    // Check for path traversal patterns
    if (strstr(filepath, "..") != NULL || 
        strstr(filepath, "/") != NULL || 
        strstr(filepath, "\\\\") != NULL) {\n        return 0;\n    }\n    return 1;\n}\n\nunsigned char* img(const char* fileParam, size_t* outSize) {\n    *outSize = 0;\n    \n    if (!fileParam || strlen(fileParam) == 0) {\n        fprintf(stderr, "File parameter is required\
");\n        return NULL;\n    }\n    \n    // Sanitize input\n    char* sanitized = sanitize_filename(fileParam);\n    if (!sanitized) {\n        fprintf(stderr, "Memory allocation error\
");\n        return NULL;\n    }\n    \n    if (!is_safe_path(sanitized)) {\n        fprintf(stderr, "Invalid file name\
");\n        free(sanitized);\n        return NULL;\n    }\n    \n    // Construct full path\n    char fullPath[MAX_PATH_LEN];\n    snprintf(fullPath, MAX_PATH_LEN, "%s%s", ALLOWED_DIRECTORY, sanitized);\n    free(sanitized);\n    \n    // Check if file exists and get size\n    struct stat st;\n    if (stat(fullPath, &st) != 0 || !S_ISREG(st.st_mode)) {\n        fprintf(stderr, "File not found or not readable\
");\n        return NULL;\n    }\n    \n    // Check file size limit\n    if (st.st_size > MAX_FILE_SIZE || st.st_size <= 0) {\n        fprintf(stderr, "File size invalid\
");\n        return NULL;\n    }\n    \n    // Open and read file\n    FILE* file = fopen(fullPath, "rb");\n    if (!file) {\n        fprintf(stderr, "Error opening file\
");\n        return NULL;\n    }\n    \n    unsigned char* buffer = (unsigned char*)malloc(st.st_size);\n    if (!buffer) {\n        fprintf(stderr, "Memory allocation error\
");\n        fclose(file);\n        return NULL;\n    }\n    \n    size_t bytesRead = fread(buffer, 1, st.st_size, file);\n    fclose(file);\n    \n    if (bytesRead != (size_t)st.st_size) {\n        fprintf(stderr, "Error reading file\
");\n        free(buffer);\n        return NULL;\n    }\n    \n    *outSize = bytesRead;\n    return buffer;\n}\n\nint main() {\n    // Create test directory\n    #ifdef _WIN32\n    _mkdir(ALLOWED_DIRECTORY);\n    #else\n    mkdir(ALLOWED_DIRECTORY, 0755);\n    #endif\n    \n    // Create test files\n    FILE* f1 = fopen(ALLOWED_DIRECTORY "test1.txt", "wb");\n    if (f1) {\n        fprintf(f1, "Test content 1");\n        fclose(f1);\n    }\n    \n    FILE* f2 = fopen(ALLOWED_DIRECTORY "test2.jpg", "wb");\n    if (f2) {\n        fprintf(f2, "Image content");\n        fclose(f2);\n    }\n    \n    size_t size;\n    unsigned char* result;\n    \n    // Test case 1: Valid file\n    printf("Test 1 - Valid file:\
");\n    result = img("test1.txt", &size);\n    if (result) {\n        printf("Success: %.*s\
", (int)size, result);\n        free(result);\n    } else {\n        printf("Failed\
");\n    }\n    \n    // Test case 2: Another valid file\n    printf("\
Test 2 - Valid image file:\
");\n    result = img("test2.jpg", &size);\n    if (result) {\n        printf("Success: %.*s\
", (int)size, result);\n        free(result);\n    } else {\n        printf("Failed\
");\n    }\n    \n    // Test case 3: Path traversal attempt\n    printf("\
Test 3 - Path traversal attempt:\
");\n    result = img("../etc/passwd", &size);\n    printf("%s\
", result ? "Success" : "Blocked (Expected)");\n    if (result) free(result);\n    \n    // Test case 4: NULL parameter\n    printf("\
Test 4 - NULL parameter:\
");\n    result = img(NULL, &size);\n    printf("%s\
", result ? "Success" : "Blocked (Expected)");\n    if (result) free(result);\n    \n    // Test case 5: Non-existent file\n    printf("\
Test 5 - Non-existent file:\
");\n    result = img("nonexistent.txt", &size);\n    printf("%s\
", result ? "Success" : "Failed (Expected)");
    if (result) free(result);
    
    return 0;
}
