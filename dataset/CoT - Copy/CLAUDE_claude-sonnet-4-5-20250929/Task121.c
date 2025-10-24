
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME 256
#define UPLOAD_DIR "uploads"

typedef struct {
    int success;
    char message[512];
    char savedPath[512];
} UploadResult;

const char* allowed_extensions[] = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx", NULL};

void to_lowercase(char* str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

const char* get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if(!dot || dot == filename || *(dot + 1) == '\\0') return "";
    return dot + 1;
}

const char* get_basename(const char* path) {
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackslash = strrchr(path, '\\\\');\n    const char* base = path;\n    \n    if(lastSlash && lastSlash > base) base = lastSlash + 1;\n    if(lastBackslash && lastBackslash > base) base = lastBackslash + 1;\n    \n    return base;\n}\n\nint is_valid_filename(const char* filename) {\n    for(int i = 0; filename[i]; i++) {\n        char c = filename[i];\n        if(!isalnum((unsigned char)c) && c != '_' && c != '-' && c != '.') {\n            return 0;\n        }\n    }\n    return 1;\n}\n\nint is_allowed_extension(const char* ext) {\n    char lower_ext[32];\n    strncpy(lower_ext, ext, sizeof(lower_ext) - 1);\n    lower_ext[sizeof(lower_ext) - 1] = '\\0';\n    to_lowercase(lower_ext);\n    \n    for(int i = 0; allowed_extensions[i] != NULL; i++) {\n        if(strcmp(lower_ext, allowed_extensions[i]) == 0) {\n            return 1;\n        }\n    }\n    return 0;\n}\n\nvoid generate_unique_filename(const char* original, char* output, size_t output_size) {\n    char base[MAX_FILENAME];\n    const char* ext = get_file_extension(original);\n    size_t base_len = strlen(original) - strlen(ext) - 1;\n    \n    if(base_len >= sizeof(base)) base_len = sizeof(base) - 1;\n    strncpy(base, original, base_len);\n    base[base_len] = '\\0';\n    \n    long long timestamp = (long long)time(NULL) * 1000;\n    snprintf(output, output_size, "%s_%lld.%s", base, timestamp, ext);\n}\n\nUploadResult upload_file(const char* original_filename, const char* file_content, size_t content_size) {\n    UploadResult result = {0, "", ""};\n    \n    // Validate filename\n    if(original_filename == NULL || strlen(original_filename) == 0) {\n        result.success = 0;\n        strcpy(result.message, "Filename cannot be empty");\n        return result;\n    }\n    \n    // Validate file content\n    if(file_content == NULL || content_size == 0) {\n        result.success = 0;\n        strcpy(result.message, "File content cannot be empty");\n        return result;\n    }\n    \n    // Check file size\n    if(content_size > MAX_FILE_SIZE) {\n        result.success = 0;\n        strcpy(result.message, "File size exceeds maximum limit of 10MB");\n        return result;\n    }\n    \n    // Sanitize filename\n    const char* sanitized = get_basename(original_filename);\n    \n    // Validate filename pattern\n    if(!is_valid_filename(sanitized)) {\n        result.success = 0;\n        strcpy(result.message, "Invalid filename. Use only alphanumeric characters, hyphens, underscores, and dots");\n        return result;\n    }\n    \n    // Validate file extension\n    const char* extension = get_file_extension(sanitized);\n    if(!is_allowed_extension(extension)) {\n        result.success = 0;\n        strcpy(result.message, "File type not allowed");\n        return result;\n    }\n    \n    // Create upload directory\n    #ifdef _WIN32\n    mkdir(UPLOAD_DIR);\n    #else\n    mkdir(UPLOAD_DIR, 0700);\n    #endif\n    \n    // Generate unique filename\n    char unique_filename[MAX_FILENAME];\n    generate_unique_filename(sanitized, unique_filename, sizeof(unique_filename));\n    \n    char upload_path[512];\n    snprintf(upload_path, sizeof(upload_path), "%s/%s", UPLOAD_DIR, unique_filename);\n    \n    // Check if file already exists\n    FILE* check = fopen(upload_path, "rb");\n    if(check != NULL) {\n        fclose(check);\n        result.success = 0;\n        strcpy(result.message, "File already exists");\n        return result;\n    }\n    \n    // Write file\n    FILE* file = fopen(upload_path, "wb");\n    if(file == NULL) {\n        result.success = 0;\n        strcpy(result.message, "Error opening file for writing");\n        return result;\n    }\n    \n    size_t written = fwrite(file_content, 1, content_size, file);\n    fclose(file);\n    \n    if(written != content_size) {\n        remove(upload_path);\n        result.success = 0;\n        strcpy(result.message, "Error writing file");\n        return result;\n    }\n    \n    // Set file permissions\n    #ifndef _WIN32\n    chmod(upload_path, S_IRUSR | S_IWUSR);\n    #endif\n    \n    result.success = 1;\n    strcpy(result.message, "File uploaded successfully");\n    strcpy(result.savedPath, upload_path);\n    \n    return result;\n}\n\nint main() {\n    printf("=== File Upload Program Test Cases ===\\n\\n");\n    \n    // Test Case 1: Valid text file upload\n    printf("Test 1: Valid text file upload\\n");\n    const char* content1 = "Hello, this is a test file content.";\n    UploadResult result1 = upload_file("test_document.txt", content1, strlen(content1));\n    printf("Success: %s\\n", result1.success ? "true" : "false");\n    printf("Message: %s\\n", result1.message);\n    printf("Path: %s\\n\\n", result1.savedPath);\n    \n    // Test Case 2: Invalid file extension\n    printf("Test 2: Invalid file extension (.exe)\\n");\n    const char* content2 = "Malicious content";\n    UploadResult result2 = upload_file("malware.exe", content2, strlen(content2));\n    printf("Success: %s\\n", result2.success ? "true" : "false");\n    printf("Message: %s\\n\\n", result2.message);\n    \n    // Test Case 3: Path traversal attempt\n    printf("Test 3: Path traversal attempt\\n");\n    const char* content3 = "Attack content";\n    UploadResult result3 = upload_file("../../etc/passwd.txt", content3, strlen(content3));\n    printf("Success: %s\\n", result3.success ? "true" : "false");\n    printf("Message: %s\\n\\n", result3.message);\n    \n    // Test Case 4: Empty filename\n    printf("Test 4: Empty filename\\n");\n    const char* content4 = "Some content";\n    UploadResult result4 = upload_file("", content4, strlen(content4));\n    printf("Success: %s\\n", result4.success ? "true" : "false");\n    printf("Message: %s\\n\\n", result4.message);\n    \n    // Test Case 5: File size exceeds limit\n    printf("Test 5: File size exceeds limit\\n");\n    size_t large_size = MAX_FILE_SIZE + 1;\n    char* content5 = (char*)malloc(large_size);\n    if(content5) {\n        memset(content5, 'A', large_size);
        UploadResult result5 = upload_file("large_file.txt", content5, large_size);
        printf("Success: %s\\n", result5.success ? "true" : "false");
        printf("Message: %s\\n\\n", result5.message);
        free(content5);
    }
    
    return 0;
}
