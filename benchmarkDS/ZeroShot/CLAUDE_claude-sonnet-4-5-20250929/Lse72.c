
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LENGTH 256
#define UPLOAD_DIR "uploads"

typedef struct {
    char* data;
    size_t size;
} FileContent;

int is_allowed_extension(const char* filename) {
    const char* allowed[] = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};
    int num_allowed = 7;
    
    const char* dot = strrchr(filename, '.');
    if (dot == NULL) {
        return 0;
    }
    
    const char* ext = dot + 1;
    for (int i = 0; i < num_allowed; i++) {
        if (strcasecmp(ext, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void sanitize_filename(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    
    for (size_t i = 0; i < strlen(input) && j < output_size - 1; i++) {
        char c = input[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

void generate_unique_filename(const char* filename, char* output, size_t output_size) {
    char sanitized[MAX_FILENAME_LENGTH];
    sanitize_filename(filename, sanitized, MAX_FILENAME_LENGTH);
    
    time_t timestamp = time(NULL);
    const char* dot = strrchr(sanitized, '.');
    
    if (dot != NULL) {
        size_t name_len = dot - sanitized;
        snprintf(output, output_size, "%.*s_%ld%s", (int)name_len, sanitized, timestamp, dot);
    } else {
        snprintf(output, output_size, "%s_%ld", sanitized, timestamp);
    }
}

char* upload_file(const FileContent* file_content, const char* filename) {
    static char result[512];
    
    // Validate file size
    if (file_content->size > MAX_FILE_SIZE) {
        strcpy(result, "Error: File size exceeds maximum allowed size");
        return result;
    }
    
    // Sanitize filename
    char sanitized[MAX_FILENAME_LENGTH];
    sanitize_filename(filename, sanitized, MAX_FILENAME_LENGTH);
    
    if (strlen(sanitized) == 0) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    // Validate file extension
    if (!is_allowed_extension(sanitized)) {
        strcpy(result, "Error: File type not allowed");
        return result;
    }
    
    // Create uploads directory
    #ifdef _WIN32
        _mkdir(UPLOAD_DIR);
    #else
        mkdir(UPLOAD_DIR, 0755);
    #endif
    
    // Generate unique filename
    char unique_filename[MAX_FILENAME_LENGTH];
    generate_unique_filename(sanitized, unique_filename, MAX_FILENAME_LENGTH);
    
    // Create file path
    char file_path[MAX_FILENAME_LENGTH + 20];
    snprintf(file_path, sizeof(file_path), "%s/%s", UPLOAD_DIR, unique_filename);
    
    // Save file
    FILE* fp = fopen(file_path, "wb");
    if (fp == NULL) {
        strcpy(result, "Error: Could not create file");
        return result;
    }
    
    fwrite(file_content->data, 1, file_content->size, fp);
    fclose(fp);
    
    snprintf(result, sizeof(result), "File uploaded successfully: %s", unique_filename);
    return result;
}

int main() {
    // Test Case 1: Valid text file
    char content1[] = "This is a test file content";
    FileContent file1 = {content1, strlen(content1)};
    printf("%s\\n", upload_file(&file1, "test1.txt"));
    
    // Test Case 2: Valid PDF file
    char* content2 = (char*)calloc(1024, 1);
    FileContent file2 = {content2, 1024};
    printf("%s\\n", upload_file(&file2, "document.pdf"));
    free(content2);
    
    // Test Case 3: Invalid extension
    FileContent file3 = {content1, strlen(content1)};
    printf("%s\\n", upload_file(&file3, "malicious.exe"));
    
    // Test Case 4: Path traversal attempt
    printf("%s\\n", upload_file(&file3, "../../../etc/passwd"));
    
    // Test Case 5: Valid image file
    char* content5 = (char*)calloc(2048, 1);
    FileContent file5 = {content5, 2048};
    printf("%s\\n", upload_file(&file5, "photo.jpg"));
    free(content5);
    
    return 0;
}
