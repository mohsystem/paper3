
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

#define IMAGES_DIR "images"
#define MAX_PATH 512
#define MAX_FILE_SIZE 10485760  // 10MB

const char* ALLOWED_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", NULL};

int is_valid_extension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(ext, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

unsigned char* serve_image(const char* filename, size_t* size) {
    *size = 0;
    
    if (!filename || strlen(filename) == 0) {
        return NULL;
    }
    
    // Create a copy for basename extraction
    char filename_copy[MAX_PATH];
    strncpy(filename_copy, filename, MAX_PATH - 1);
    filename_copy[MAX_PATH - 1] = '\\0';
    
    // Sanitize filename - get only basename
    char* sanitized = basename(filename_copy);
    if (!sanitized || strlen(sanitized) == 0) {
        return NULL;
    }
    
    // Validate extension
    if (!is_valid_extension(sanitized)) {
        return NULL;
    }
    
    // Construct path
    char filepath[MAX_PATH];
    snprintf(filepath, MAX_PATH, "%s/%s", IMAGES_DIR, sanitized);
    
    // Check if path contains traversal attempts
    if (strstr(filepath, "..") != NULL) {
        return NULL;
    }
    
    // Check file exists and get size
    struct stat st;
    if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return NULL;
    }
    
    if (st.st_size > MAX_FILE_SIZE || st.st_size <= 0) {
        return NULL;
    }
    
    // Read file
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return NULL;
    }
    
    unsigned char* buffer = (unsigned char*)malloc(st.st_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, st.st_size, file);
    fclose(file);
    
    if (read_size != (size_t)st.st_size) {
        free(buffer);
        return NULL;
    }
    
    *size = read_size;
    return buffer;
}

int main() {
    // Create images directory
    #ifdef _WIN32
    mkdir(IMAGES_DIR);
    #else
    mkdir(IMAGES_DIR, 0755);
    #endif
    
    // Create test images
    const char* test_files[] = {"test1.jpg", "test2.png", "test3.gif", "test4.webp", "test5.bmp"};
    for (int i = 0; i < 5; i++) {
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "%s/%s", IMAGES_DIR, test_files[i]);
        FILE* f = fopen(path, "wb");
        if (f) {
            fprintf(f, "Test image content for %s", test_files[i]);
            fclose(f);
        }
    }
    
    // Test cases
    size_t size;
    unsigned char* result;
    
    result = serve_image("test1.jpg", &size);
    printf("Test 1 - Valid image (test1.jpg): %s\\n", result ? "SUCCESS" : "FAILED");
    free(result);
    
    result = serve_image("test2.png", &size);
    printf("Test 2 - Valid image (test2.png): %s\\n", result ? "SUCCESS" : "FAILED");
    free(result);
    
    result = serve_image("../test.jpg", &size);
    printf("Test 3 - Path traversal attempt (../test.jpg): %s\\n", !result ? "SUCCESS (Blocked)" : "FAILED");
    free(result);
    
    result = serve_image("test.txt", &size);
    printf("Test 4 - Invalid extension (test.txt): %s\\n", !result ? "SUCCESS (Blocked)" : "FAILED");
    free(result);
    
    result = serve_image("nonexistent.jpg", &size);
    printf("Test 5 - Non-existent file (nonexistent.jpg): %s\\n", !result ? "SUCCESS (Not Found)" : "FAILED");
    free(result);
    
    return 0;
}
