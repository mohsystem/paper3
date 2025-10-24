
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/types.h>
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME 256

typedef struct {
    int success;
    char message[512];
    char saved_path[512];
} UploadResult;

const char* ALLOWED_EXTENSIONS[] = {"txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx", NULL};

int is_allowed_extension(const char* ext) {
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void get_file_extension(const char* filename, char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strcpy(extension, dot + 1);
    } else {
        extension[0] = '\\0';
    }
}

void sanitize_filename(const char* filename, char* sanitized) {
    const char* basename = strrchr(filename, '/');
    if (!basename) basename = strrchr(filename, '\\\\');\n    basename = basename ? basename + 1 : filename;\n    \n    int j = 0;\n    for (int i = 0; basename[i] && j < MAX_FILENAME - 1; i++) {\n        char c = basename[i];\n        if (isalnum(c) || c == '.' || c == '_' || c == '-') {\n            sanitized[j++] = c;\n        } else {\n            sanitized[j++] = '_';\n        }\n    }\n    sanitized[j] = '\\0';\n}\n\nvoid generate_unique_filename(const char* original, char* unique) {\n    char name_without_ext[MAX_FILENAME];\n    char extension[64];\n    \n    get_file_extension(original, extension);\n    \n    const char* dot = strrchr(original, '.');\n    if (dot) {\n        int len = dot - original;\n        strncpy(name_without_ext, original, len);\n        name_without_ext[len] = '\\0';\n    } else {\n        strcpy(name_without_ext, original);\n    }\n    \n    long timestamp = (long)time(NULL) * 1000;\n    \n    if (strlen(extension) > 0) {\n        snprintf(unique, MAX_FILENAME, "%s_%ld.%s", name_without_ext, timestamp, extension);\n    } else {\n        snprintf(unique, MAX_FILENAME, "%s_%ld", name_without_ext, timestamp);\n    }\n}\n\nint create_directory(const char* dir) {\n    struct stat st = {0};\n    if (stat(dir, &st) == -1) {\n        return mkdir(dir, 0700) == 0;\n    }\n    return 1;\n}\n\nUploadResult upload_file(const char* filename, const char* file_content, size_t content_size) {\n    UploadResult result = {0, "", ""};\n    \n    // Validate filename\n    if (!filename || strlen(filename) == 0) {\n        strcpy(result.message, "Invalid filename");\n        return result;\n    }\n    \n    // Sanitize filename\n    char sanitized_filename[MAX_FILENAME];\n    sanitize_filename(filename, sanitized_filename);\n    \n    // Validate file extension\n    char extension[64];\n    get_file_extension(sanitized_filename, extension);\n    \n    if (!is_allowed_extension(extension)) {\n        snprintf(result.message, sizeof(result.message), \n                "File type not allowed: %s", extension);\n        return result;\n    }\n    \n    // Validate file size\n    if (!file_content || content_size == 0) {\n        strcpy(result.message, "Empty file content");\n        return result;\n    }\n    if (content_size > MAX_FILE_SIZE) {\n        strcpy(result.message, "File size exceeds maximum limit");\n        return result;\n    }\n    \n    // Create upload directory\n    if (!create_directory(UPLOAD_DIR)) {\n        strcpy(result.message, "Failed to create upload directory");\n        return result;\n    }\n    \n    // Generate unique filename\n    char unique_filename[MAX_FILENAME];\n    generate_unique_filename(sanitized_filename, unique_filename);\n    \n    char target_path[512];\n    snprintf(target_path, sizeof(target_path), "%s/%s", UPLOAD_DIR, unique_filename);\n    \n    // Write file\n    FILE* outfile = fopen(target_path, "wb");\n    if (!outfile) {\n        strcpy(result.message, "Failed to create file");\n        return result;\n    }\n    \n    size_t written = fwrite(file_content, 1, content_size, outfile);\n    fclose(outfile);\n    \n    if (written != content_size) {\n        strcpy(result.message, "Failed to write complete file");\n        return result;\n    }\n    \n    // Set file permissions (Unix-like systems)\n#ifndef _WIN32\n    chmod(target_path, S_IRUSR | S_IWUSR);\n#endif\n    \n    result.success = 1;\n    snprintf(result.message, sizeof(result.message), \n            "File uploaded successfully: %s", unique_filename);\n    strcpy(result.saved_path, target_path);\n    \n    return result;\n}\n\nint main() {\n    printf("=== Secure File Upload Server - Test Cases ===\\n\\n");\n    \n    // Test Case 1: Valid text file upload\n    const char* content1 = "This is a test file content.";\n    UploadResult result1 = upload_file("test_document.txt", content1, strlen(content1));\n    printf("Test 1 - Valid text file:\\n");\n    printf("Success: %s\\n", result1.success ? "true" : "false");\n    printf("Message: %s\\n\\n", result1.message);\n    \n    // Test Case 2: Valid PDF file upload\n    char content2[1024];\n    memset(content2, 'A', sizeof(content2));
    UploadResult result2 = upload_file("report.pdf", content2, sizeof(content2));
    printf("Test 2 - Valid PDF file:\\n");
    printf("Success: %s\\n", result2.success ? "true" : "false");
    printf("Message: %s\\n\\n", result2.message);
    
    // Test Case 3: Invalid file extension
    const char* content3 = "malware";
    UploadResult result3 = upload_file("malicious.exe", content3, strlen(content3));
    printf("Test 3 - Invalid extension (.exe):\\n");
    printf("Success: %s\\n", result3.success ? "true" : "false");
    printf("Message: %s\\n\\n", result3.message);
    
    // Test Case 4: Path traversal attempt
    const char* content4 = "hack";
    UploadResult result4 = upload_file("../../etc/passwd.txt", content4, strlen(content4));
    printf("Test 4 - Path traversal attempt:\\n");
    printf("Success: %s\\n", result4.success ? "true" : "false");
    printf("Message: %s\\n\\n", result4.message);
    
    // Test Case 5: Empty file
    UploadResult result5 = upload_file("empty.txt", "", 0);
    printf("Test 5 - Empty file:\\n");
    printf("Success: %s\\n", result5.success ? "true" : "false");
    printf("Message: %s\\n\\n", result5.message);
    
    return 0;
}
