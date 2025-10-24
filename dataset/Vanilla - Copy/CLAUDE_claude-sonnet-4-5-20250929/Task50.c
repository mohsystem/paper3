
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILENAME 256
#define MAX_MESSAGE 512

typedef struct {
    char message[MAX_MESSAGE];
} UploadResult;

typedef struct {
    int exists;
} FileCheckResult;

// Function to handle file upload
UploadResult handle_file_upload(const char* file_name, const unsigned char* file_data, size_t data_size) {
    UploadResult result;
    
    // Create upload directory if it doesn't exist\n    MKDIR(UPLOAD_DIR);\n    \n    // Create full file path\n    char file_path[MAX_FILENAME + 50];\n    snprintf(file_path, sizeof(file_path), "%s/%s", UPLOAD_DIR, file_name);\n    \n    // Open file for writing in binary mode\n    FILE* file = fopen(file_path, "wb");\n    if (file == NULL) {\n        snprintf(result.message, MAX_MESSAGE, "ERROR: Failed to create file");\n        return result;\n    }\n    \n    // Write data to file\n    size_t written = fwrite(file_data, 1, data_size, file);\n    fclose(file);\n    \n    if (written != data_size) {\n        snprintf(result.message, MAX_MESSAGE, "ERROR: Failed to write complete file");\n        return result;\n    }\n    \n    snprintf(result.message, MAX_MESSAGE, \n             "SUCCESS: File '%s' uploaded successfully. Size: %zu bytes", \n             file_name, data_size);\n    \n    return result;\n}\n\n// Function to check if file exists\nint file_exists(const char* file_name) {\n    char file_path[MAX_FILENAME + 50];\n    snprintf(file_path, sizeof(file_path), "%s/%s", UPLOAD_DIR, file_name);\n    \n    FILE* file = fopen(file_path, "r");\n    if (file) {\n        fclose(file);\n        return 1;\n    }\n    return 0;\n}\n\n// Client function to upload file\nUploadResult upload_file(const char* file_name, const unsigned char* file_data, size_t data_size) {\n    return handle_file_upload(file_name, file_data, data_size);\n}\n\nint main() {\n    printf("File Upload System - Test Cases\
\
");\n    \n    // Test Case 1: Upload a text file\n    printf("Test Case 1: Upload text file\
");\n    const char* content1 = "Hello, this is a test file!";\n    UploadResult result1 = upload_file("test1.txt", (const unsigned char*)content1, strlen(content1));\n    printf("%s\
", result1.message);\n    printf("File exists: %s\
\
", file_exists("test1.txt") ? "true" : "false");\n    \n    // Test Case 2: Upload a binary file\n    printf("Test Case 2: Upload binary file\
");\n    unsigned char binary_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};\n    UploadResult result2 = upload_file("test2.bin", binary_data, sizeof(binary_data));\n    printf("%s\
", result2.message);\n    printf("File exists: %s\
\
", file_exists("test2.bin") ? "true" : "false");\n    \n    // Test Case 3: Upload a larger file\n    printf("Test Case 3: Upload larger file\
");\n    char* large_content = (char*)malloc(50000);\n    int offset = 0;\n    for (int i = 0; i < 1000; i++) {\n        offset += snprintf(large_content + offset, 50000 - offset, "Line %d: This is test data.\
", i);\n    }\n    UploadResult result3 = upload_file("test3.log", (const unsigned char*)large_content, offset);\n    printf("%s\
", result3.message);\n    printf("File exists: %s\
\
", file_exists("test3.log") ? "true" : "false");\n    free(large_content);\n    \n    // Test Case 4: Upload file with special characters in name\n    printf("Test Case 4: Upload file with special characters\
");\n    const char* content4 = "Special file content";\n    UploadResult result4 = upload_file("test_file-4.txt", (const unsigned char*)content4, strlen(content4));\n    printf("%s\
", result4.message);\n    printf("File exists: %s\
\
", file_exists("test_file-4.txt") ? "true" : "false");\n    \n    // Test Case 5: Upload empty file\n    printf("Test Case 5: Upload empty file\
");\n    UploadResult result5 = upload_file("empty.txt", (const unsigned char*)"", 0);\n    printf("%s\
", result5.message);\n    printf("File exists: %s\
\
", file_exists("empty.txt") ? "true" : "false");
    
    return 0;
}
