
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME 256
#define UPLOAD_DIR "uploads"

const char* ALLOWED_EXTENSIONS[] = {"txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx", NULL};

int is_allowed_extension(const char* ext) {
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void sanitize_filename(const char* input, char* output, size_t size) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < size - 1; i++) {
        if (isalnum(input[i]) || input[i] == '.' || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
    
    // Remove ".." patterns
    char* ptr;
    while ((ptr = strstr(output, "..")) != NULL) {
        memmove(ptr, ptr + 2, strlen(ptr + 2) + 1);
    }
}

void get_file_extension(const char* filename, char* ext, size_t size) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strncpy(ext, dot + 1, size - 1);
        ext[size - 1] = '\\0';
    } else {
        ext[0] = '\\0';
    }
}

void generate_secure_filename(const char* original, char* secure, size_t size) {
    char ext[32];
    get_file_extension(original, ext, sizeof(ext));
    
    time_t t = time(NULL);
    unsigned int hash = 0;
    for (size_t i = 0; original[i]; i++) {
        hash = hash * 31 + original[i];
    }
    hash = hash * 31 + (unsigned int)t;
    
    snprintf(secure, size, "%08x_%ld.%s", hash, (long)t, ext);
}

int create_directory(const char* path) {
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

char* handle_file_upload(const char* filename, const char* content, size_t content_size) {
    static char result[512];
    char sanitized[MAX_FILENAME];
    char extension[32];
    char secure_filename[MAX_FILENAME];
    char filepath[MAX_FILENAME + 32];
    
    // Validate file size
    if (content_size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds limit");
        return result;
    }
    
    // Sanitize filename
    sanitize_filename(filename, sanitized, sizeof(sanitized));
    if (strlen(sanitized) == 0) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    // Validate extension
    get_file_extension(sanitized, extension, sizeof(extension));
    if (!is_allowed_extension(extension)) {
        strcpy(result, "Error: File type not allowed");
        return result;
    }
    
    // Create upload directory
    create_directory(UPLOAD_DIR);
    
    // Generate secure filename
    generate_secure_filename(sanitized, secure_filename, sizeof(secure_filename));
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, secure_filename);
    
    // Check for path traversal
    if (strstr(filepath, "..") != NULL) {
        strcpy(result, "Error: Invalid file path");
        return result;
    }
    
    // Write file
    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        strcpy(result, "Error: Could not write file");
        return result;
    }
    
    fwrite(content, 1, content_size, fp);
    fclose(fp);
    
    snprintf(result, sizeof(result), "Success: File uploaded as %s", secure_filename);
    return result;
}

int main() {
    printf("=== File Upload Security Test Cases ===\\n\\n");
    
    // Test Case 1: Valid text file
    const char* content1 = "Hello, World!";
    printf("Test 1 - Valid txt file: %s\\n", 
           handle_file_upload("document.txt", content1, strlen(content1)));
    
    // Test Case 2: Valid PDF file
    const char* content2 = "%PDF-1.4";
    printf("Test 2 - Valid pdf file: %s\\n", 
           handle_file_upload("report.pdf", content2, strlen(content2)));
    
    // Test Case 3: Invalid file extension
    const char* content3 = "malicious code";
    printf("Test 3 - Invalid extension: %s\\n", 
           handle_file_upload("script.exe", content3, strlen(content3)));
    
    // Test Case 4: Path traversal attempt
    const char* content4 = "test";
    printf("Test 4 - Path traversal: %s\\n", 
           handle_file_upload("../../etc/passwd", content4, strlen(content4)));
    
    // Test Case 5: Large file (simulated with size check)
    printf("Test 5 - Oversized file: %s\\n", 
           handle_file_upload("large.txt", "data", MAX_FILE_SIZE + 1));
    
    return 0;
}
