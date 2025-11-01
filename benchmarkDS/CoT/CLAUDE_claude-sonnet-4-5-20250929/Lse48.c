
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#define IMAGES_DIR "images"
#define MAX_FILENAME 256

const char* ALLOWED_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};
const int NUM_EXTENSIONS = 6;

int is_valid_extension(const char* filename) {
    int len = strlen(filename);
    for (int i = 0; i < NUM_EXTENSIONS; i++) {
        int ext_len = strlen(ALLOWED_EXTENSIONS[i]);
        if (len >= ext_len) {
            const char* file_ext = filename + len - ext_len;
            if (strcasecmp(file_ext, ALLOWED_EXTENSIONS[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

unsigned char* img(const char* filename, size_t* size) {
    *size = 0;
    
    if (!filename || strlen(filename) == 0) {
        return NULL;
    }
    
    // Security: Validate filename - no path traversal
    if (strstr(filename, "..") || strchr(filename, '/') || 
        strchr(filename, '\\\\') || filename[0] == '.') {
        return NULL;
    }
    
    // Security: Validate file extension
    if (!is_valid_extension(filename)) {
        return NULL;
    }
    
    // Security: Construct safe path
    char filepath[PATH_MAX];
    snprintf(filepath, sizeof(filepath), "%s/%s", IMAGES_DIR, filename);
    
    // Security: Resolve canonical path
    char realpath_buf[PATH_MAX];
    if (!realpath(filepath, realpath_buf)) {
        return NULL;
    }
    
    // Security: Ensure file is within allowed directory
    char base_realpath[PATH_MAX];
    if (!realpath(IMAGES_DIR, base_realpath)) {
        return NULL;
    }
    
    if (strncmp(realpath_buf, base_realpath, strlen(base_realpath)) != 0) {
        return NULL;
    }
    
    // Check if file exists and is a regular file
    struct stat st;
    if (stat(realpath_buf, &st) != 0 || !S_ISREG(st.st_mode)) {
        return NULL;
    }
    
    // Read and return file
    FILE* file = fopen(realpath_buf, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* buffer = (unsigned char*)malloc(file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (read_size != file_size) {
        free(buffer);
        return NULL;
    }
    
    *size = file_size;
    return buffer;
}

int main() {
    // Create images directory
    mkdir(IMAGES_DIR, 0755);
    
    // Test cases
    printf("Test Case 1: Valid image file\\n");
    FILE* f1 = fopen("images/test1.jpg", "wb");
    if (f1) {
        fwrite("JPEG_IMAGE_DATA", 1, 15, f1);
        fclose(f1);
    }
    size_t size1;
    unsigned char* result1 = img("test1.jpg", &size1);
    printf("Result: %s\\n\\n", result1 ? "Success" : "Failed");
    free(result1);
    
    printf("Test Case 2: Path traversal attempt\\n");
    size_t size2;
    unsigned char* result2 = img("../etc/passwd", &size2);
    printf("Result: %s\\n\\n", !result2 ? "Blocked (Success)" : "Failed");
    
    printf("Test Case 3: Invalid extension\\n");
    size_t size3;
    unsigned char* result3 = img("test.txt", &size3);
    printf("Result: %s\\n\\n", !result3 ? "Blocked (Success)" : "Failed");
    
    printf("Test Case 4: Empty filename\\n");
    size_t size4;
    unsigned char* result4 = img("", &size4);
    printf("Result: %s\\n\\n", !result4 ? "Blocked (Success)" : "Failed");
    
    printf("Test Case 5: Valid PNG file\\n");
    FILE* f2 = fopen("images/test2.png", "wb");
    if (f2) {
        fwrite("PNG_IMAGE_DATA", 1, 14, f2);
        fclose(f2);
    }
    size_t size5;
    unsigned char* result5 = img("test2.png", &size5);
    printf("Result: %s\\n", result5 ? "Success" : "Failed");
    free(result5);
    
    return 0;
}
