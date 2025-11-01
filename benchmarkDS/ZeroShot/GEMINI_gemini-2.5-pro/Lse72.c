#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define strcasecmp _stricmp
#else
#include <strings.h> // For strcasecmp
#define MKDIR(path) mkdir(path, 0755)
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (5 * 1024 * 1024) // 5 MB
#define MAX_FILENAME_LEN 255
#define MAX_PATH_LEN 4096

// Returns 0 on success, -1 on failure.
int create_dir_if_not_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (MKDIR(path) != 0 && errno != EEXIST) {
            perror("Error creating directory");
            return -1;
        }
        printf("Created directory: %s\n", path);
    }
    return 0;
}

// Simple check for whitelisted characters. Returns true if valid.
bool is_filename_safe(const char* filename) {
    if (filename == NULL || *filename == '\0') return false;
    for (size_t i = 0; filename[i] != '\0'; i++) {
        char c = filename[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

// Returns true if the extension is allowed.
bool is_extension_allowed(const char* filename) {
    const char* allowed_extensions[] = {".txt", ".pdf", ".jpg", ".png", NULL};
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return false;

    for (int i = 0; allowed_extensions[i] != NULL; i++) {
        if (strcasecmp(dot, allowed_extensions[i]) == 0) {
            return true;
        }
    }
    return false;
}

int upload_file(const char* original_filename, const char* content, size_t content_len) {
    if (original_filename == NULL || content == NULL) {
        fprintf(stderr, "Error: Filename or content is null.\n");
        return -1;
    }

    // 1. Check file size
    if (content_len > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the limit of %d bytes.\n", MAX_FILE_SIZE);
        return -2;
    }

    // 2. Sanitize filename to prevent path traversal
    const char* basename_fwd = strrchr(original_filename, '/');
    const char* basename_bwd = strrchr(original_filename, '\\');
    const char* basename = basename_fwd > basename_bwd ? basename_fwd : basename_bwd;
    basename = (basename == NULL) ? original_filename : basename + 1;

    if (strlen(basename) == 0 || strlen(basename) > MAX_FILENAME_LEN) {
        fprintf(stderr, "Error: Invalid filename length.\n");
        return -3;
    }
    
    // Additional check for potentially harmful characters
    if (!is_filename_safe(basename)) {
        fprintf(stderr, "Error: Filename '%s' contains invalid characters.\n", basename);
        return -4;
    }

    // 3. Validate file extension
    if (!is_extension_allowed(basename)) {
        fprintf(stderr, "Error: File type is not allowed for '%s'.\n", basename);
        return -5;
    }

    // 4. Create upload directory
    if (create_dir_if_not_exists(UPLOAD_DIR) != 0) return -6;

    // 5. Construct final path
    char destination_path[MAX_PATH_LEN];
    int written = snprintf(destination_path, sizeof(destination_path), "%s/%s", UPLOAD_DIR, basename);
    if (written < 0 || (size_t)written >= sizeof(destination_path)) {
        fprintf(stderr, "Error: Destination path is too long.\n");
        return -7;
    }

    // 6. Save the file. Use "wb" for binary mode. Prevent overwriting.
    FILE *check_fp = fopen(destination_path, "rb");
    if (check_fp != NULL) {
        fclose(check_fp);
        fprintf(stderr, "Error: File '%s' already exists. Upload aborted.\n", basename);
        return -8;
    }

    FILE *fp = fopen(destination_path, "wb");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -9;
    }

    size_t bytes_written = fwrite(content, 1, content_len, fp);
    fclose(fp);

    if (bytes_written != content_len) {
        fprintf(stderr, "Error: Failed to write the entire file content.\n");
        remove(destination_path); // Clean up partial file
        return -10;
    }

    printf("File saved successfully to: %s\n", destination_path);
    return 0;
}

void run_test(const char* test_name, const char* filename, const char* content) {
    printf("\n[Test] %s\n", test_name);
    upload_file(filename, content, strlen(content));
}

int main() {
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Normal, valid file upload
    run_test("Normal file upload", "my-data.txt", "This is some sample data.");

    // Test Case 2: Path traversal attempt
    run_test("Path traversal attempt", "/etc/shadow", "user:$6$...");

    // Test Case 3: File too large
    printf("\n[Test] File too large\n");
    char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
    if (large_content) {
        memset(large_content, 'A', MAX_FILE_SIZE + 1);
        upload_file("bigfile.jpg", large_content, MAX_FILE_SIZE + 1);
        free(large_content);
    }
    
    // Test Case 4: Disallowed file type
    run_test("Disallowed file type", "runme.bat", "@echo off\ndel C:\\*.* /q");

    // Test Case 5: Filename with invalid characters
    run_test("Filename with invalid characters", "file<with>bad|chars.png", "image data");
    
    // Create test file again to check overwrite prevention
    run_test("Re-upload to test overwrite protection", "my-data.txt", "This is updated data.");

    printf("\n--- Test Cases Finished ---\n");
    return 0;
}