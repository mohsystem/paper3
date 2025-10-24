
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME 256

const char* ALLOWED_EXTENSIONS[] = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};
const int NUM_EXTENSIONS = 7;

int is_valid_filename(const char* filename) {
    if (!filename || strlen(filename) == 0) return 0;
    
    // Check for path traversal attempts
    if (strstr(filename, "..") || strchr(filename, '/') || strchr(filename, '\\\\')) {\n        return 0;\n    }\n    \n    // Check for valid characters\n    for (int i = 0; filename[i]; i++) {\n        char c = filename[i];\n        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {\n            return 0;\n        }\n    }\n    return 1;\n}\n\nvoid get_file_extension(const char* filename, char* extension) {\n    const char* dot = strrchr(filename, '.');\n    if (dot && dot != filename) {\n        strcpy(extension, dot + 1);\n    } else {\n        extension[0] = '\\0';\n    }\n}\n\nint is_allowed_extension(const char* extension) {\n    char lower_ext[32];\n    strcpy(lower_ext, extension);\n    for (int i = 0; lower_ext[i]; i++) {\n        lower_ext[i] = tolower(lower_ext[i]);\n    }\n    \n    for (int i = 0; i < NUM_EXTENSIONS; i++) {\n        if (strcmp(lower_ext, ALLOWED_EXTENSIONS[i]) == 0) {\n            return 1;\n        }\n    }\n    return 0;\n}\n\nvoid sanitize_filename(const char* filename, char* sanitized) {\n    int j = 0;\n    for (int i = 0; filename[i]; i++) {\n        char c = filename[i];\n        if (isalnum(c) || c == '.' || c == '_' || c == '-') {\n            sanitized[j++] = c;\n        } else {\n            sanitized[j++] = '_';\n        }\n    }\n    sanitized[j] = '\\0';\n}\n\nvoid generate_unique_filename(const char* filename, char* unique_name) {\n    char name[MAX_FILENAME];\n    char extension[32];\n    \n    const char* dot = strrchr(filename, '.');\n    if (dot) {\n        strncpy(name, filename, dot - filename);\n        name[dot - filename] = '\\0';\n        strcpy(extension, dot + 1);\n    } else {\n        strcpy(name, filename);\n        extension[0] = '\\0';
    }
    
    long long timestamp = (long long)time(NULL) * 1000;
    sprintf(unique_name, "%s_%lld.%s", name, timestamp, extension);
}

void calculate_checksum(const unsigned char* data, size_t size, char* checksum) {
    // Simple checksum (for production use proper cryptographic hash)
    unsigned long hash = 0;
    for (size_t i = 0; i < size; i++) {
        hash = hash * 31 + data[i];
    }
    sprintf(checksum, "%016lx", hash);
}

void create_directory(const char* dir) {
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
}

char* upload_file(const char* filename, const unsigned char* file_content, size_t content_size) {
    static char result[512];
    
    // Validate filename
    if (!is_valid_filename(filename)) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    // Check file size
    if (content_size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds maximum allowed size (10MB)");
        return result;
    }
    
    // Check file extension
    char extension[32];
    get_file_extension(filename, extension);
    if (!is_allowed_extension(extension)) {
        strcpy(result, "Error: File type not allowed");
        return result;
    }
    
    // Create upload directory
    create_directory(UPLOAD_DIR);
    
    // Sanitize and generate unique filename
    char sanitized[MAX_FILENAME];
    char unique_name[MAX_FILENAME];
    char filepath[MAX_FILENAME * 2];
    
    sanitize_filename(filename, sanitized);
    generate_unique_filename(sanitized, unique_name);
    sprintf(filepath, "%s/%s", UPLOAD_DIR, unique_name);
    
    // Write file
    FILE* f = fopen(filepath, "wb");
    if (!f) {
        strcpy(result, "Error: Failed to create file");
        return result;
    }
    fwrite(file_content, 1, content_size, f);
    fclose(f);
    
    // Calculate checksum
    char checksum[33];
    calculate_checksum(file_content, content_size, checksum);
    
    sprintf(result, "Success: File uploaded successfully. Filename: %s, Size: %zu bytes, Checksum: %s",
            unique_name, content_size, checksum);
    
    return result;
}

int main() {
    printf("File Upload Server - Test Cases\\n\\n");
    
    // Test Case 1: Valid text file
    const char* data1 = "Hello World!";
    char* result1 = upload_file("test1.txt", (unsigned char*)data1, strlen(data1));
    printf("Test 1: %s\\n\\n", result1);
    
    // Test Case 2: Valid PDF file (simulated)
    const char* data2 = "PDF content simulation";
    char* result2 = upload_file("document.pdf", (unsigned char*)data2, strlen(data2));
    printf("Test 2: %s\\n\\n", result2);
    
    // Test Case 3: Invalid filename with path traversal
    const char* data3 = "malicious content";
    char* result3 = upload_file("../../../etc/passwd", (unsigned char*)data3, strlen(data3));
    printf("Test 3: %s\\n\\n", result3);
    
    // Test Case 4: Disallowed file extension
    const char* data4 = "executable content";
    char* result4 = upload_file("script.exe", (unsigned char*)data4, strlen(data4));
    printf("Test 4: %s\\n\\n", result4);
    
    // Test Case 5: Valid image file
    const char* data5 = "JPEG image data";
    char* result5 = upload_file("photo.jpg", (unsigned char*)data5, strlen(data5));
    printf("Test 5: %s\\n\\n", result5);
    
    return 0;
}
