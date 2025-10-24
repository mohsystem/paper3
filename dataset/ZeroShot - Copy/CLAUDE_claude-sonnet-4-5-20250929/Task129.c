
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#define BASE_DIRECTORY "./secure_files/"
#define MAX_FILE_SIZE 10485760  // 10MB
#define MAX_PATH_LENGTH 4096
#define MAX_CONTENT_SIZE 10485760

const char* ALLOWED_EXTENSIONS[] = {".txt", ".json", ".xml", ".csv", ".log", NULL};

int is_safe_filename(const char* filename) {
    if (!filename || strlen(filename) == 0) return 0;
    
    for (int i = 0; filename[i] != '\\0'; i++) {
        char c = filename[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != '.') {
            return 0;
        }
    }
    return 1;
}

int is_allowed_extension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

char* retrieve_file(const char* file_name) {
    static char result[MAX_CONTENT_SIZE];
    char full_path[MAX_PATH_LENGTH];
    char real_base[MAX_PATH_LENGTH];
    char real_file[MAX_PATH_LENGTH];
    FILE* file;
    struct stat st;
    
    memset(result, 0, sizeof(result));
    
    // Input validation
    if (!file_name || strlen(file_name) == 0) {
        snprintf(result, sizeof(result), "Error: File name cannot be empty");
        return result;
    }
    
    // Check for safe filename pattern
    if (!is_safe_filename(file_name)) {
        snprintf(result, sizeof(result), "Error: Invalid file name format");
        return result;
    }
    
    // Check file extension
    if (!is_allowed_extension(file_name)) {
        snprintf(result, sizeof(result), "Error: File type not allowed");
        return result;
    }
    
    // Build full path
    snprintf(full_path, sizeof(full_path), "%s%s", BASE_DIRECTORY, file_name);
    
    // Get real paths to prevent path traversal
    if (!realpath(BASE_DIRECTORY, real_base)) {
        snprintf(result, sizeof(result), "Error: Cannot resolve base directory");
        return result;
    }
    
    if (!realpath(full_path, real_file)) {
        snprintf(result, sizeof(result), "Error: File not found");
        return result;
    }
    
    // Check if file is within base directory
    if (strncmp(real_file, real_base, strlen(real_base)) != 0) {
        snprintf(result, sizeof(result), "Error: Path traversal detected. Access denied");
        return result;
    }
    
    // Check if file exists and get stats
    if (stat(real_file, &st) != 0) {
        snprintf(result, sizeof(result), "Error: File not found");
        return result;
    }
    
    // Check if it's a regular file\n    if (!S_ISREG(st.st_mode)) {\n        snprintf(result, sizeof(result), "Error: Invalid file type");\n        return result;\n    }\n    \n    // Check file size\n    if (st.st_size > MAX_FILE_SIZE) {\n        snprintf(result, sizeof(result), "Error: File size exceeds maximum allowed size");\n        return result;\n    }\n    \n    // Read file content\n    file = fopen(real_file, "r");\n    if (!file) {\n        snprintf(result, sizeof(result), "Error: Unable to open file");\n        return result;\n    }\n    \n    snprintf(result, sizeof(result), "Success: File retrieved\\nContent:\\n");\n    size_t offset = strlen(result);\n    size_t read_size = fread(result + offset, 1, sizeof(result) - offset - 1, file);\n    result[offset + read_size] = '\\0';
    
    fclose(file);
    return result;
}

void setup_test_environment() {
    struct stat st = {0};
    
    if (stat(BASE_DIRECTORY, &st) == -1) {
        mkdir(BASE_DIRECTORY, 0700);
    }
    
    // Create test files
    FILE* file1 = fopen(BASE_DIRECTORY "test1.txt", "w");
    if (file1) {
        fprintf(file1, "This is a test file 1");
        fclose(file1);
    }
    
    FILE* file2 = fopen(BASE_DIRECTORY "test2.json", "w");
    if (file2) {
        fprintf(file2, "{\\"name\\": \\"test\\", \\"value\\": 123}");
        fclose(file2);
    }
    
    FILE* file3 = fopen(BASE_DIRECTORY "test3.csv", "w");
    if (file3) {
        fprintf(file3, "id,name,age\\n1,John,30\\n2,Jane,25");
        fclose(file3);
    }
}

int main() {
    setup_test_environment();
    
    printf("=== Secure File Retrieval System ===\\n\\n");
    
    // Test case 1: Valid file retrieval
    printf("Test 1: Valid file retrieval\\n");
    printf("%s\\n---\\n\\n", retrieve_file("test1.txt"));
    
    // Test case 2: Another valid file
    printf("Test 2: Valid JSON file\\n");
    printf("%s\\n---\\n\\n", retrieve_file("test2.json"));
    
    // Test case 3: Path traversal attempt
    printf("Test 3: Path traversal attempt\\n");
    printf("%s\\n---\\n\\n", retrieve_file("../../../etc/passwd"));
    
    // Test case 4: Invalid file extension
    printf("Test 4: Invalid file extension\\n");
    printf("%s\\n---\\n\\n", retrieve_file("test.exe"));
    
    // Test case 5: Non-existent file
    printf("Test 5: Non-existent file\\n");
    printf("%s\\n---\\n\\n", retrieve_file("nonexistent.txt"));
    
    return 0;
}
