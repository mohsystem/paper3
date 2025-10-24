#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_MAX MAX_PATH
#define realpath(N,R) _fullpath((R),(N),PATH_MAX)
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// The base directory from which files can be served.
const char* BASE_DIRECTORY = "public";

/**
 * @brief Securely retrieves the content of a file from a predefined base directory.
 *
 * It prevents path traversal attacks by ensuring the requested file's canonical
 * path is within the base directory. The caller is responsible for freeing the
 * returned buffer.
 * @param userProvidedFilename The name of the file provided by the user.
 * @return A dynamically allocated string with file content, or NULL on failure.
 */
char* retrieveFileContent(const char* userProvidedFilename) {
    if (userProvidedFilename == NULL || *userProvidedFilename == '\0') {
        fprintf(stderr, "Error: Filename cannot be null or empty.\n");
        return NULL;
    }

    // --- Sanitize Input ---
    // Find the last path separator to get the basename
    const char* last_slash = strrchr(userProvidedFilename, '/');
    const char* last_backslash = strrchr(userProvidedFilename, '\\');
    const char* safe_filename = userProvidedFilename;
    if (last_slash && last_backslash) {
        safe_filename = (last_slash > last_backslash ? last_slash + 1 : last_backslash + 1);
    } else if (last_slash) {
        safe_filename = last_slash + 1;
    } else if (last_backslash) {
        safe_filename = last_backslash + 1;
    }
    
    if (*safe_filename == '\0') {
        fprintf(stderr, "Error: Invalid filename format.\n");
        return NULL;
    }

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", BASE_DIRECTORY, safe_filename);

    char canonical_base_path[PATH_MAX];
    char canonical_full_path[PATH_MAX];

    // --- Security Check ---
    // Resolve the canonical path for the base directory
    if (realpath(BASE_DIRECTORY, canonical_base_path) == NULL) {
        perror("Error resolving base path");
        return NULL;
    }

    // Resolve the canonical path for the requested file
    if (realpath(full_path, canonical_full_path) == NULL) {
        perror("Error resolving full path (file may not exist)");
        return NULL;
    }
    
    // The canonical path of the file must start with the canonical path of the base directory.
    size_t base_len = strlen(canonical_base_path);
    if (strncmp(canonical_full_path, canonical_base_path, base_len) != 0) {
        fprintf(stderr, "Security Alert: Path traversal attempt detected for file: %s\n", userProvidedFilename);
        return NULL;
    }
    // Also check for directory mismatch, e.g., /base/dir vs /base/dir-something
    if (canonical_full_path[base_len] != '/' && canonical_full_path[base_len] != '\\' && canonical_full_path[base_len] != '\0') {
        fprintf(stderr, "Security Alert: Path traversal attempt detected for file: %s\n", userProvidedFilename);
        return NULL;
    }


    // --- Read File ---
    FILE* file = fopen(canonical_full_path, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fprintf(stderr, "Error getting file size.\n");
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Error reading file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

void setupTestEnvironment() {
    // Create directories
    MKDIR("public");
    MKDIR("public/safe_dir");
    MKDIR("secret");

    // Create files
    FILE* f;
    f = fopen("public/file1.txt", "w");
    if (f) {
        fputs("This is file 1.", f);
        fclose(f);
    }
    f = fopen("public/safe_dir/file2.txt", "w");
     if (f) {
        fputs("This is file 2.", f);
        fclose(f);
    }
    f = fopen("secret/secret.txt", "w");
    if (f) {
        fputs("This is a secret.", f);
        fclose(f);
    }
    printf("Test environment created successfully.\n");
}

int main() {
    setupTestEnvironment();

    const char* test_cases[] = {
        "file1.txt",                          // 1. Valid file
        "safe_dir/file2.txt",                 // 2. Valid file in subdir (should fail due to basename sanitization)
        "../secret/secret.txt",               // 3. Path Traversal
    #ifdef _WIN32
        "C:\\Windows\\System32\\drivers\\etc\\hosts", // 4. Absolute Path (Windows)
    #else
        "/etc/passwd",                      // 4. Absolute Path (Unix-like)
    #endif
        "nonexistent.txt"                     // 5. Non-existent file
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Test Case %d: Requesting '%s' ---\n", i + 1, test_cases[i]);
        char* content = retrieveFileContent(test_cases[i]);
        if (content) {
            printf("Success! File content:\n%s\n", content);
            free(content);
        } else {
            printf("Failed as expected.\n");
        }
    }
    return 0;
}