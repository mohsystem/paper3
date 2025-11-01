#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // For mkdir
#include <unistd.h>   // For rmdir, unlink
#include <limits.h>   // For PATH_MAX

// Note: This implementation uses POSIX-specific functions like realpath, mkdir,
// and system calls for file management. It will not be portable to Windows
// without modification (e.g., using _mkdir, _fullpath).

#define BUFFER_SIZE 4096

// Define the base directory. For testing, we use a temporary directory.
// In a real application, this would be a fixed path like "/safe/".
const char* SAFE_DIR = "test_root/safe/";

/**
 * Reads a file from a designated "safe" directory into a buffer.
 *
 * @param filename The name of the file to read. Must not contain path separators.
 * @param buffer The buffer to store the file content.
 * @param buffer_size The size of the buffer.
 * @return 0 on success, -1 on failure. A detailed error message is printed to stderr.
 */
int readFileFromSafeDirectory(const char* filename, char* buffer, size_t buffer_size) {
    // 1. Initial validation: Filename should not contain path separators.
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid characters in filename. Path traversal attempt rejected.\n");
        return -1;
    }
    
    // 2. Path validation using realpath.
    char real_safe_path[PATH_MAX];
    char real_target_path[PATH_MAX];
    char temp_target_path[PATH_MAX];

    // Construct the initial target path string
    snprintf(temp_target_path, sizeof(temp_target_path), "%s%s", SAFE_DIR, filename);

    // Resolve the canonical paths. realpath returns NULL if path doesn't exist.
    if (realpath(SAFE_DIR, real_safe_path) == NULL) {
        perror("Error resolving safe directory path");
        return -1;
    }

    if (realpath(temp_target_path, real_target_path) == NULL) {
        perror("Error resolving target file path (file may not exist)");
        return -1;
    }
    
    // Ensure the target path starts with the safe path.
    size_t safe_path_len = strlen(real_safe_path);
    if (strncmp(real_target_path, real_safe_path, safe_path_len) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected.\n");
        return -1;
    }
    // Also check that the next character is a separator or null terminator,
    // to prevent cases like /safe-dir from matching /safe.
    if (real_target_path[safe_path_len] != '/' && real_target_path[safe_path_len] != '\0') {
         fprintf(stderr, "Error: Path traversal attempt detected (ambiguous path).\n");
         return -1;
    }

    // 3. Read the file if validation passes.
    FILE* file = fopen(real_target_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Clear buffer and read file content
    memset(buffer, 0, buffer_size);
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);
    if (ferror(file)) {
        perror("Error reading from file");
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0; // Success
}

// Helper functions for test setup and cleanup
void setup_test_environment() {
    printf("--- Setting up test environment ---\n");
    // Using POSIX mkdir; S_IRWXU gives read/write/execute permissions to user.
    mkdir("test_root", S_IRWXU);
    mkdir(SAFE_DIR, S_IRWXU);

    FILE* f1 = fopen("test_root/safe/good.txt", "w");
    if (f1) { fprintf(f1, "This is a safe file."); fclose(f1); }
    
    FILE* f2 = fopen("test_root/safe/another.txt", "w");
    if (f2) { fprintf(f2, "This is another safe file."); fclose(f2); }
    
    FILE* f3 = fopen("test_root/outside.txt", "w");
    if (f3) { fprintf(f3, "This file is outside the safe directory."); fclose(f3); }
    printf("Setup complete.\n");
    printf("-------------------------------------\n\n");
}

void cleanup_test_environment() {
    // POSIX unlink to delete files, rmdir to delete directories
    unlink("test_root/safe/good.txt");
    unlink("test_root/safe/another.txt");
    unlink("test_root/outside.txt");
    rmdir("test_root/safe");
    rmdir("test_root");
    // printf("Test environment cleaned up.\n");
}


int main() {
    setup_test_environment();

    const char* test_filenames[] = {
        "good.txt",                      // Test Case 1: Valid file
        "another.txt",                   // Test Case 2: Another valid file
        "nonexistent.txt",               // Test Case 3: File does not exist
        "../outside.txt",                // Test Case 4: Path traversal attempt
        "good.txt/../../outside.txt"     // Test Case 5: Complex path traversal attempt
    };
    int num_tests = sizeof(test_filenames) / sizeof(test_filenames[0]);

    char file_content_buffer[BUFFER_SIZE];

    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d: Attempting to read '%s'\n", i + 1, test_filenames[i]);
        if (readFileFromSafeDirectory(test_filenames[i], file_content_buffer, BUFFER_SIZE) == 0) {
            printf("Result: %s\n\n", file_content_buffer);
        } else {
            // Error message is printed inside the function to stderr
            printf("Result: FAILED\n\n");
        }
    }
    
    cleanup_test_environment();

    return 0;
}