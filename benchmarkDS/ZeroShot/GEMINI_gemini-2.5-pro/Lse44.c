#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#ifdef _WIN32
    #include <direct.h>
    #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
    #define mkdir(path, mode) _mkdir(path)
    #define rmdir _rmdir
    #define PATH_MAX _MAX_PATH
#endif

// The base directory from which files can be read.
const char* SAFE_DIRECTORY = "safe";

/**
 * @brief Securely reads a file from a designated 'safe' directory.
 *
 * It prevents directory traversal by validating the filename and ensuring the
 * canonical path of the requested file is within the safe directory.
 *
 * @param filename The name of the file to read. Must not contain path separators.
 * @return A heap-allocated string with the file content, or an error message.
 *         The caller is responsible for freeing the returned string.
 */
char* readFileFromSafeDirectory(const char* filename) {
    // Validation 1: Basic input checks
    if (filename == NULL || *filename == '\0') {
        return strdup("Error: Filename is empty.");
    }
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return strdup("Error: Filename must not contain path separators.");
    }

    // Construct the full path safely to avoid buffer overflows
    char full_path[PATH_MAX];
    int len = snprintf(full_path, sizeof(full_path), "%s/%s", SAFE_DIRECTORY, filename);
    if (len < 0 || (size_t)len >= sizeof(full_path)) {
        return strdup("Error: Constructed path is too long.");
    }

    // Validation 2: Canonical path check
    char resolved_base[PATH_MAX];
    char resolved_path[PATH_MAX];

    // Get the canonical path of the safe base directory
    if (realpath(SAFE_DIRECTORY, resolved_base) == NULL) {
        return strdup("Error: Safe directory does not exist or cannot be accessed.");
    }

    // Get the canonical path of the requested file.
    // realpath returns NULL if the file does not exist.
    if (realpath(full_path, resolved_path) == NULL) {
        return strdup("Error: File not found or cannot be accessed.");
    }
    
    // The core security check: does the resolved path start with the resolved base path?
    size_t base_len = strlen(resolved_base);
    if (strncmp(resolved_path, resolved_base, base_len) != 0 || 
       (resolved_path[base_len] != '/' && resolved_path[base_len] != '\0')) {
        return strdup("Error: Directory traversal attempt detected.");
    }
    
    // Ensure the path points to a regular file, not a directory
    struct stat path_stat;
    if (stat(resolved_path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) {
        return strdup("Error: Path does not point to a regular file.");
    }

    // It's now safe to read the file
    FILE* file = fopen(resolved_path, "rb");
    if (file == NULL) {
        return strdup("Error: Could not open file.");
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        return strdup("Error: Could not determine file size.");
    }

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return strdup("Error: Could not allocate memory for file content.");
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fclose(file);
        free(buffer);
        return strdup("Error: Failed to read entire file.");
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}


int main() {
    // --- Test Environment Setup ---
    mkdir(SAFE_DIRECTORY, 0755);
    FILE* test_file = fopen("safe/document1.txt", "w");
    if (test_file) {
        fputs("This is a safe document.", test_file);
        fclose(test_file);
        printf("Test environment created.\n");
    } else {
        fprintf(stderr, "Failed to set up test environment.\n");
        return 1;
    }

    // --- Test Cases ---
    const char* test_filenames[] = {
        "document1.txt",               // 1. Valid file
        "nonexistent.txt",             // 2. File not found
        "../main.c",                   // 3. Simple traversal attempt
        "document1.txt/../../main.c",  // 4. Nested traversal attempt
        "/etc/passwd"                  // 5. Absolute path attempt
    };
    int num_tests = sizeof(test_filenames) / sizeof(test_filenames[0]);

    printf("\n--- Running Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* filename = test_filenames[i];
        printf("Test Case %d: Requesting \"%s\"\n", i + 1, filename);
        char* result = readFileFromSafeDirectory(filename);
        if (result) {
            printf("Result: %s\n", result);
            free(result); // Free the heap-allocated string
        } else {
            printf("Result: NULL (Error)\n");
        }
        printf("--------------------\n");
    }

    // --- Test Environment Teardown ---
    remove("safe/document1.txt");
    rmdir(SAFE_DIRECTORY);
    printf("\nTest environment cleaned up.\n");

    return 0;
}