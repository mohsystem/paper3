
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024

int is_safe_filename(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }
    
    for (size_t i = 0; i < strlen(filename); i++) {
        char c = filename[i];
        if (!isalnum(c) && c != '.' && c != '-' && c != '_') {
            return 0;
        }
    }
    return 1;
}

char* sanitize_filename(const char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    
    const char* lastSlash = strrchr(filename, '/');
    const char* lastBackslash = strrchr(filename, '\\\\');\n    const char* basename = filename;\n    \n    if (lastSlash != NULL) {\n        basename = lastSlash + 1;\n    }\n    if (lastBackslash != NULL && lastBackslash > basename) {\n        basename = lastBackslash + 1;\n    }\n    \n    char* sanitized = (char*)malloc(strlen(basename) + 1);\n    if (sanitized != NULL) {\n        strcpy(sanitized, basename);\n    }\n    return sanitized;\n}\n\nchar* read_file_securely(const char* filename) {\n    if (filename == NULL || strlen(filename) == 0) {\n        char* error = (char*)malloc(100);\n        strcpy(error, "Error: Filename cannot be null or empty");\n        return error;\n    }\n    \n    char* sanitized = sanitize_filename(filename);\n    if (sanitized == NULL) {\n        char* error = (char*)malloc(100);\n        strcpy(error, "Error: Memory allocation failed");\n        return error;\n    }\n    \n    if (!is_safe_filename(sanitized)) {\n        char* error = (char*)malloc(150);\n        strcpy(error, "Error: Invalid filename format. Only alphanumeric, dots, hyphens, and underscores allowed");\n        free(sanitized);\n        return error;\n    }\n    \n    struct stat st;\n    if (stat(sanitized, &st) != 0) {\n        char* error = (char*)malloc(100);\n        if (errno == ENOENT) {\n            strcpy(error, "Error: File does not exist");\n        } else {\n            sprintf(error, "Error: Unable to access file - %s", strerror(errno));\n        }\n        free(sanitized);\n        return error;\n    }\n    \n    if (!S_ISREG(st.st_mode)) {\n        char* error = (char*)malloc(100);\n        strcpy(error, "Error: Not a regular file");\n        free(sanitized);\n        return error;\n    }\n    \n    if (st.st_size > MAX_FILE_SIZE) {\n        char* error = (char*)malloc(100);\n        strcpy(error, "Error: File size exceeds maximum allowed size");\n        free(sanitized);\n        return error;\n    }\n    \n    FILE* file = fopen(sanitized, "rb");\n    if (file == NULL) {\n        char* error = (char*)malloc(150);\n        sprintf(error, "Error: Unable to open file - %s", strerror(errno));\n        free(sanitized);\n        return error;\n    }\n    \n    char* content = (char*)malloc(st.st_size + 1);\n    if (content == NULL) {\n        char* error = (char*)malloc(100);\n        strcpy(error, "Error: Memory allocation failed");\n        fclose(file);\n        free(sanitized);\n        return error;\n    }\n    \n    size_t bytesRead = fread(content, 1, st.st_size, file);\n    content[bytesRead] = '\\0';
    
    fclose(file);
    free(sanitized);
    
    return content;
}

int main() {
    char* result;
    
    printf("Test 1 - Valid file:\\n");
    result = read_file_securely("test.txt");
    printf("%s\\n\\n", result);
    free(result);
    
    printf("Test 2 - NULL filename:\\n");
    result = read_file_securely(NULL);
    printf("%s\\n\\n", result);
    free(result);
    
    printf("Test 3 - Empty filename:\\n");
    result = read_file_securely("");
    printf("%s\\n\\n", result);
    free(result);
    
    printf("Test 4 - Path traversal attempt:\\n");
    result = read_file_securely("../etc/passwd");
    printf("%s\\n\\n", result);
    free(result);
    
    printf("Test 5 - Non-existent file:\\n");
    result = read_file_securely("nonexistent.txt");
    printf("%s\\n\\n", result);
    free(result);
    
    return 0;
}
