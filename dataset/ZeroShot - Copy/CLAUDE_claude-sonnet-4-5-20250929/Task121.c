
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME 256
#define UPLOAD_DIRECTORY "uploads"

typedef struct {
    int success;
    char message[256];
    char savedPath[512];
} UploadResult;

const char* allowed_extensions[] = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx", NULL};

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int isAllowedExtension(const char* extension) {
    char ext_lower[32];
    strncpy(ext_lower, extension, sizeof(ext_lower) - 1);
    ext_lower[sizeof(ext_lower) - 1] = '\\0';
    toLowerCase(ext_lower);
    
    for (int i = 0; allowed_extensions[i] != NULL; i++) {
        if (strcmp(ext_lower, allowed_extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void getFileExtension(const char* filename, char* extension, size_t ext_size) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strncpy(extension, dot + 1, ext_size - 1);
        extension[ext_size - 1] = '\\0';
    } else {
        extension[0] = '\\0';
    }
}

void getBasename(const char* path, char* basename, size_t base_size) {
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackslash = strrchr(path, '\\\\');\n    const char* start = path;\n    \n    if (lastSlash && (!lastBackslash || lastSlash > lastBackslash)) {\n        start = lastSlash + 1;\n    } else if (lastBackslash) {\n        start = lastBackslash + 1;\n    }\n    \n    strncpy(basename, start, base_size - 1);\n    basename[base_size - 1] = '\\0';\n}\n\nint isSafeFilename(const char* filename) {\n    for (int i = 0; filename[i]; i++) {\n        char c = filename[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||\n              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {\n            return 0;\n        }\n    }\n    return 1;\n}\n\nint sanitizeFilename(const char* filename, char* sanitized, size_t san_size) {\n    char basename[MAX_FILENAME];\n    getBasename(filename, basename, sizeof(basename));\n    \n    if (!isSafeFilename(basename)) {\n        return 0;\n    }\n    \n    strncpy(sanitized, basename, san_size - 1);\n    sanitized[san_size - 1] = '\\0';\n    return 1;\n}\n\nvoid generateUniqueFilename(const char* original, char* unique, size_t unique_size) {\n    char nameWithoutExt[MAX_FILENAME];\n    char extension[32];\n    \n    strncpy(nameWithoutExt, original, sizeof(nameWithoutExt) - 1);\n    nameWithoutExt[sizeof(nameWithoutExt) - 1] = '\\0';\n    \n    char* dot = strrchr(nameWithoutExt, '.');\n    if (dot) {\n        *dot = '\\0';
    }
    
    getFileExtension(original, extension, sizeof(extension));
    
    time_t now = time(NULL);
    snprintf(unique, unique_size, "%s_%ld.%s", nameWithoutExt, (long)now, extension);
}

int createDirectory(const char* path) {
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

UploadResult uploadFile(const char* filename, const unsigned char* fileContent, size_t contentSize) {
    UploadResult result = {0, "", ""};
    
    if (!filename || strlen(filename) == 0) {
        result.success = 0;
        strcpy(result.message, "Invalid filename");
        return result;
    }
    
    char sanitized[MAX_FILENAME];
    if (!sanitizeFilename(filename, sanitized, sizeof(sanitized))) {
        result.success = 0;
        strcpy(result.message, "Filename contains invalid characters");
        return result;
    }
    
    char extension[32];
    getFileExtension(sanitized, extension, sizeof(extension));
    if (!isAllowedExtension(extension)) {
        result.success = 0;
        strcpy(result.message, "File type not allowed");
        return result;
    }
    
    if (!fileContent || contentSize == 0) {
        result.success = 0;
        strcpy(result.message, "Empty file");
        return result;
    }
    
    if (contentSize > MAX_FILE_SIZE) {
        result.success = 0;
        strcpy(result.message, "File size exceeds maximum limit");
        return result;
    }
    
    createDirectory(UPLOAD_DIRECTORY);
    
    char uniqueFilename[MAX_FILENAME];
    generateUniqueFilename(sanitized, uniqueFilename, sizeof(uniqueFilename));
    
    char filePath[512];
    snprintf(filePath, sizeof(filePath), "%s/%s", UPLOAD_DIRECTORY, uniqueFilename);
    
    FILE* fp = fopen(filePath, "wb");
    if (!fp) {
        result.success = 0;
        strcpy(result.message, "Error creating file");
        return result;
    }
    
    size_t written = fwrite(fileContent, 1, contentSize, fp);
    fclose(fp);
    
    if (written != contentSize) {
        result.success = 0;
        strcpy(result.message, "Error writing file");
        return result;
    }
    
    result.success = 1;
    strcpy(result.message, "File uploaded successfully");
    strcpy(result.savedPath, filePath);
    
    return result;
}

int main() {
    printf("=== File Upload Program - Test Cases ===\\n\\n");
    
    // Test Case 1: Valid text file
    printf("Test Case 1: Valid text file\\n");
    const char* content1 = "Hello, this is a test file.";
    UploadResult result1 = uploadFile("test.txt", (const unsigned char*)content1, strlen(content1));
    printf("Success: %s\\n", result1.success ? "true" : "false");
    printf("Message: %s\\n", result1.message);
    printf("Path: %s\\n\\n", result1.savedPath);
    
    // Test Case 2: Valid PDF file
    printf("Test Case 2: Valid PDF file\\n");
    unsigned char* data2 = (unsigned char*)malloc(1024);
    memset(data2, 0x50, 1024);
    UploadResult result2 = uploadFile("document.pdf", data2, 1024);
    printf("Success: %s\\n", result2.success ? "true" : "false");
    printf("Message: %s\\n", result2.message);
    printf("Path: %s\\n\\n", result2.savedPath);
    free(data2);
    
    // Test Case 3: Invalid file extension
    printf("Test Case 3: Invalid file extension\\n");
    const char* content3 = "Executable content";
    UploadResult result3 = uploadFile("malicious.exe", (const unsigned char*)content3, strlen(content3));
    printf("Success: %s\\n", result3.success ? "true" : "false");
    printf("Message: %s\\n\\n", result3.message);
    
    // Test Case 4: File too large
    printf("Test Case 4: File too large\\n");
    unsigned char* data4 = (unsigned char*)malloc(MAX_FILE_SIZE + 1);
    memset(data4, 0, MAX_FILE_SIZE + 1);
    UploadResult result4 = uploadFile("large_file.txt", data4, MAX_FILE_SIZE + 1);
    printf("Success: %s\\n", result4.success ? "true" : "false");
    printf("Message: %s\\n\\n", result4.message);
    free(data4);
    
    // Test Case 5: Path traversal attempt
    printf("Test Case 5: Path traversal attempt\\n");
    const char* content5 = "Malicious content";
    UploadResult result5 = uploadFile("../../etc/passwd.txt", (const unsigned char*)content5, strlen(content5));
    printf("Success: %s\\n", result5.success ? "true" : "false");
    printf("Message: %s\\n\\n", result5.message);
    
    return 0;
}
