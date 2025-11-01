#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Platform-specific includes and definitions for realpath and path limits
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_MAX MAX_PATH
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define SEP '\\'
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#define SEP '/'
#endif

/**
 * Securely retrieves an image file from a designated 'images' directory.
 * This function simulates a web endpoint handler for a URL like /img?name=...
 * It prevents path traversal attacks by validating the filename and ensuring
 * the resolved file path is within the intended directory.
 *
 * @param name The name of the file requested from the 'images' directory.
 * @param file_size A pointer to a long where the size of the file will be stored.
 * @return A dynamically allocated buffer containing the file's content, or NULL
 *         if the file is not found or a security violation is detected. The caller
 *         is responsible for freeing this buffer.
 */
char* img(const char* name, long* file_size) {
    *file_size = 0;

    // 1. Basic validation: reject NULL, empty, or names containing path separators.
    if (name == NULL || *name == '\0' || strstr(name, "/") != NULL || strstr(name, "\\") != NULL || strstr(name, "..") != NULL) {
        fprintf(stderr, "Error: Invalid characters or path traversal attempt in filename '%s'.\n", name);
        return NULL;
    }

    // 2. Define the secure base directory and get its canonical path.
    char base_dir_path[PATH_MAX];
    if (realpath("images", base_dir_path) == NULL) {
        fprintf(stderr, "Error: Could not resolve 'images' directory path. Error: %s\n", strerror(errno));
        return NULL;
    }

    // 3. Construct the full requested path.
    char requested_path_str[PATH_MAX];
    int len = snprintf(requested_path_str, sizeof(requested_path_str), "images%c%s", SEP, name);
    if (len >= sizeof(requested_path_str)) {
        fprintf(stderr, "Error: Constructed path is too long.\n");
        return NULL;
    }

    // Get the canonical path of the requested file.
    char canonical_path[PATH_MAX];
    if (realpath(requested_path_str, canonical_path) == NULL) {
        // This can happen if the file doesn't exist, which is not a security error.
        fprintf(stderr, "Error: File not found: %s\n", requested_path_str);
        return NULL;
    }

    // 4. Security Check: Ensure the resolved path is still within the base directory.
    size_t base_len = strlen(base_dir_path);
    if (strncmp(canonical_path, base_dir_path, base_len) != 0 || 
        (canonical_path[base_len] != SEP && canonical_path[base_len] != '\0')) {
        fprintf(stderr, "Security Error: Path traversal detected for filename '%s'.\n", name);
        return NULL;
    }
    
    // 5. Check if it's a regular file and read it.
    struct stat statbuf;
    if (stat(canonical_path, &statbuf) != 0 || !S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path is not a regular file: %s\n", canonical_path);
        return NULL;
    }

    FILE* file = fopen(canonical_path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file: %s. Error: %s\n", canonical_path, strerror(errno));
        return NULL;
    }

    *file_size = statbuf.st_size;
    char* buffer = (char*)malloc(*file_size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, *file_size, file) != *file_size) {
        fprintf(stderr, "Error: Could not read entire file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return buffer;
}

// Main function with test cases
int main() {
    // Setup: Create a dummy directory and files for testing
    const char* image_dir = "images";
    const char valid_image_path[] = "images/test.jpg";
    const char* secret_file_path = "secret.txt";

    MKDIR(image_dir);
    FILE* f_valid = fopen(valid_image_path, "w");
    if (f_valid) { fputs("jpeg_data", f_valid); fclose(f_valid); }
    FILE* f_secret = fopen(secret_file_path, "w");
    if (f_secret) { fputs("secret_data", f_secret); fclose(f_secret); }
    printf("Test environment created.\n");

    printf("\n--- Running Test Cases ---\n");

    const char* test_cases[] = {
        "test.jpg",                     // 1. Valid case
        "../secret.txt",                // 2. Path traversal
        "nonexistent.png",              // 3. File not found
        "test.jpg/../test.jpg",         // 4. More complex invalid name
        "../../../../etc/passwd"        // 5. Classic path traversal attack
    };
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_cases; ++i) {
        printf("\nTest Case %d: Requesting '%s'\n", i + 1, test_cases[i]);
        long size = 0;
        char* result = img(test_cases[i], &size);
        if (result) {
            printf("Success: Returned %ld bytes. Content: %.*s\n", size, (int)size, result);
            free(result);
        } else {
            printf("Failure: Request denied as expected.\n");
        }
    }

    // Teardown: Clean up the created files and directory
    remove(valid_image_path);
    remove(secret_file_path);
    RMDIR(image_dir);
    printf("\n--- Test environment cleaned up. ---\n");

    return 0;
}