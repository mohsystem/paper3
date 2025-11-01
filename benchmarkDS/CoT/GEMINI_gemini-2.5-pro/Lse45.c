#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#define PATH_MAX _MAX_PATH
#else
#include <unistd.h>
#include <limits.h> // For PATH_MAX
#endif

// Define the safe directory. In a real application, this should be an absolute path.
const char* SAFE_DIRECTORY = "safe_dir";

/**
 * Reads a file from a predefined safe directory.
 * Prevents path traversal by resolving the canonical path of the file
 * and ensuring it resides within the safe directory.
 * NOTE: The caller is responsible for freeing the memory allocated for 'output_buffer'.
 *
 * @param filename The name of the file to read.
 * @param output_buffer A pointer to a char pointer that will be allocated and filled with file content.
 * @return 0 on success, -1 on security violation, -2 on file error (not found, etc.).
 */
int readFileFromSafeDirectory(const char* filename, char** output_buffer) {
    if (filename == NULL || output_buffer == NULL) {
        return -2;
    }
    *output_buffer = NULL;

    // --- Path Construction ---
    // Construct the full path string: safe_dir/filename
    char full_path[PATH_MAX];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", SAFE_DIRECTORY, filename);
    if (len < 0 || len >= sizeof(full_path)) {
        fprintf(stderr, "Error: Constructed path is too long.\n");
        return -2;
    }
    
    // --- Security Check ---
    // Resolve the canonical path of the safe directory.
    char real_safe_path[PATH_MAX];
    if (realpath(SAFE_DIRECTORY, real_safe_path) == NULL) {
        perror("Error resolving safe directory path");
        return -2;
    }

    // Resolve the canonical path of the target file.
    char real_target_path[PATH_MAX];
    if (realpath(full_path, real_target_path) == NULL) {
        // This is expected for non-existent files, but also for invalid paths.
        // We can treat it as a generic "not found or access denied".
        return -2;
    }
    
    // Check if the target path starts with the safe directory path.
    size_t safe_path_len = strlen(real_safe_path);
    if (strncmp(real_target_path, real_safe_path, safe_path_len) != 0) {
        return -1; // Security violation
    }
    
    // An additional check to ensure it's not just a partial match like /safe_dir_other
    if (real_target_path[safe_path_len] != '/' && real_target_path[safe_path_len] != '\0') {
         // This can happen on Windows with backslashes, but the check is generally good
        #ifndef _WIN32
        return -1; // Security violation
        #endif
    }

    // --- File Reading ---
    FILE* file = fopen(real_target_path, "rb");
    if (file == NULL) {
        return -2; // File error
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        return -2;
    }

    *output_buffer = (char*)malloc(file_size + 1);
    if (*output_buffer == NULL) {
        fclose(file);
        return -2;
    }

    size_t bytes_read = fread(*output_buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fclose(file);
        free(*output_buffer);
        *output_buffer = NULL;
        return -2;
    }

    (*output_buffer)[file_size] = '\0';
    fclose(file);
    return 0; // Success
}

// Simple mkdir cross-platform
void create_dir(const char* path) {
    #ifdef _WIN32
        _mkdir(path);
    #else
        mkdir(path, 0755);
    #endif
}

void setupEnvironment() {
    printf("--- Setting up test environment ---\n");
    // Using system calls for simplicity in setup
    system("rm -rf safe_dir evil.txt"); // Cleanup
    create_dir(SAFE_DIRECTORY);
    create_dir("safe_dir/sub");
    
    FILE* f;
    f = fopen("safe_dir/good_file.txt", "w");
    if (f) { fprintf(f, "This is a safe file."); fclose(f); }

    f = fopen("safe_dir/sub/nested_file.txt", "w");
    if (f) { fprintf(f, "This is a nested safe file."); fclose(f); }
    
    f = fopen("evil.txt", "w");
    if (f) { fprintf(f, "This file is outside the safe directory."); fclose(f); }
    
    printf("Setup complete.\n\n");
}


int main() {
    setupEnvironment();
    
    const char* testCases[] = {
        "good_file.txt",            // 1. Should succeed
        "sub/nested_file.txt",      // 2. Should succeed (subdirectory)
        "non_existent.txt",         // 3. Should fail (file error)
        "../evil.txt",              // 4. Should fail (security violation)
        "sub/../../evil.txt"        // 5. Should fail (security violation)
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Running Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* testCase = testCases[i];
        printf("Attempting to read: '%s'\n", testCase);
        
        char* content = NULL;
        int result = readFileFromSafeDirectory(testCase, &content);

        if (result == 0) {
            printf("  [SUCCESS] Content: \"%s\"\n", content);
        } else if (result == -1) {
            printf("  [FAILURE] SecurityException: Access Denied: Path traversal attempt detected.\n");
        } else { // result == -2
            printf("  [FAILURE] FileException: File not found or could not be read.\n");
        }
        
        if (content != NULL) {
            free(content);
        }
        printf("---------------------------------\n");
    }

    // Cleanup
    remove("evil.txt");
    remove("safe_dir/good_file.txt");
    remove("safe_dir/sub/nested_file.txt");
    rmdir("safe_dir/sub");
    rmdir("safe_dir");

    return 0;
}