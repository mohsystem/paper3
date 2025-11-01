
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LEN 255

typedef struct {
    char* data;
    size_t size;
} FileContent;

const char* ALLOWED_EXTENSIONS[] = {".txt", ".pdf", ".jpg", ".png", ".doc", ".docx", NULL};

void sanitize_filename(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    const char* basename = strrchr(input, '/');
    if (!basename) basename = strrchr(input, '\\\\');\n    if (basename) basename++; else basename = input;\n    \n    // Copy and sanitize\n    for (size_t i = 0; basename[i] && j < output_size - 1; i++) {\n        char c = basename[i];\n        if (isalnum(c) || c == '.' || c == '-' || c == '_') {\n            output[j++] = c;\n        } else {\n            output[j++] = '_';\n        }\n    }\n    output[j] = '\\0';\n    \n    // Remove leading dots\n    size_t start = 0;\n    while (output[start] == '.') start++;\n    if (start > 0) {\n        memmove(output, output + start, strlen(output + start) + 1);\n    }\n}\n\nint is_allowed_extension(const char* filename) {\n    size_t len = strlen(filename);\n    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {\n        size_t ext_len = strlen(ALLOWED_EXTENSIONS[i]);\n        if (len >= ext_len) {\n            const char* file_ext = filename + len - ext_len;\n            char lower_ext[20];\n            for (size_t j = 0; j < ext_len && j < 19; j++) {\n                lower_ext[j] = tolower(file_ext[j]);\n            }\n            lower_ext[ext_len] = '\\0';\n            \n            if (strcmp(lower_ext, ALLOWED_EXTENSIONS[i]) == 0) {\n                return 1;\n            }\n        }\n    }\n    return 0;\n}\n\nint create_directory(const char* path) {\n#ifdef _WIN32\n    return _mkdir(path) == 0 || errno == EEXIST;\n#else\n    return mkdir(path, 0755) == 0 || errno == EEXIST;\n#endif\n}\n\nchar* upload_file(const FileContent* file_content, const char* filename) {\n    static char result[512];\n    char sanitized[MAX_FILENAME_LEN + 1];\n    char filepath[512];\n    \n    // Validate filename\n    if (!filename || strlen(filename) == 0) {\n        snprintf(result, sizeof(result), "Error: Invalid filename");\n        return result;\n    }\n    \n    // Sanitize filename\n    sanitize_filename(filename, sanitized, sizeof(sanitized));\n    if (strlen(sanitized) == 0) {\n        snprintf(result, sizeof(result), "Error: Invalid filename after sanitization");\n        return result;\n    }\n    \n    // Validate extension\n    if (!is_allowed_extension(sanitized)) {\n        snprintf(result, sizeof(result), "Error: File type not allowed");\n        return result;\n    }\n    \n    // Validate size\n    if (file_content->size > MAX_FILE_SIZE) {\n        snprintf(result, sizeof(result), "Error: File size exceeds limit");\n        return result;\n    }\n    \n    // Create directory\n    create_directory(UPLOAD_DIR);\n    \n    // Create filepath\n    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, sanitized);\n    \n    // Check for path traversal\n    if (strstr(filepath, "..") != NULL) {\n        snprintf(result, sizeof(result), "Error: Invalid file path");\n        return result;\n    }\n    \n    // Save file\n    FILE* f = fopen(filepath, "wb");\n    if (!f) {\n        snprintf(result, sizeof(result), "Error: Cannot create file");\n        return result;\n    }\n    \n    fwrite(file_content->data, 1, file_content->size, f);\n    fclose(f);\n    \n    snprintf(result, sizeof(result), "Success: File uploaded to %s", filepath);\n    return result;\n}\n\nint main() {\n    // Test case 1: Valid file\n    FileContent content1 = {"Test content", 12};\n    printf("Test 1: %s\\n", upload_file(&content1, "document.txt"));\n    \n    // Test case 2: Path traversal attempt\n    FileContent content2 = {"Malicious", 9};\n    printf("Test 2: %s\\n", upload_file(&content2, "../../../etc/passwd"));\n    \n    // Test case 3: Invalid extension\n    FileContent content3 = {"Script", 6};\n    printf("Test 3: %s\\n", upload_file(&content3, "malicious.exe"));\n    \n    // Test case 4: Valid image\n    char buffer[1024];\n    memset(buffer, 'x', sizeof(buffer));
    FileContent content4 = {buffer, 1024};
    printf("Test 4: %s\\n", upload_file(&content4, "photo.jpg"));
    
    // Test case 5: Empty filename
    FileContent content5 = {"Content", 7};
    printf("Test 5: %s\\n", upload_file(&content5, ""));
    
    return 0;
}
