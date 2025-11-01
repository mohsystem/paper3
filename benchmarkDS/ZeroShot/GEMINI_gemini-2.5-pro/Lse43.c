#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // For PATH_MAX

// On Windows, use _fullpath instead of realpath
#ifdef _WIN32
#include <direct.h>
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
#else
#include <unistd.h> // For realpath on POSIX
#endif

/**
 * Securely reads a file from the /safe/ directory.
 * Prevents path traversal attacks by resolving the canonical path and
 * ensuring it starts with the safe base directory.
 *
 * @param filename The name of the file to read.
 * @return A dynamically allocated string with the file's content,
 *         or NULL on failure. The caller is responsible for freeing
 *         the returned string.
 */
char* readFileFromSafeDirectory(const char* filename) {
    const char* base_dir = "/safe/";

    // Security Check 1: Basic validation on the filename input.
    if (filename == NULL || strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid filename or path traversal detected in input.\n");
        return NULL;
    }

    // Construct the full path safely to avoid buffer overflows.
    char full_path[PATH_MAX];
    int len = snprintf(full_path, sizeof(full_path), "%s%s", base_dir, filename);
    if (len < 0 || (size_t)len >= sizeof(full_path)) {
        fprintf(stderr, "Error: Path construction failed or path is too long.\n");
        return NULL;
    }
    
    // Security Check 2: Canonicalize the path and verify it's within the base directory.
    char resolved_path[PATH_MAX];
    if (realpath(full_path, resolved_path) == NULL) {
        // This can fail if the file doesn't exist, which is a normal case.
        // We log it but don't treat it as a security failure.
        perror("realpath failed (file may not exist)");
        return NULL;
    }
    
    if (strncmp(base_dir, resolved_path, strlen(base_dir)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected. Final path '%s' is outside '%s'.\n", resolved_path, base_dir);
        return NULL;
    }

    FILE* file = fopen(resolved_path, "rb"); // Open in binary read mode for safety
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the content
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    // Read file into buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if ((long)bytes_read != file_size) {
        fprintf(stderr, "Error: Could not read the entire file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0'; // Null-terminate the string

    fclose(file);
    return buffer;
}


int main() {
    printf("--- Running C Test Cases ---\n");
    printf("NOTE: For these tests to pass, create the /safe directory and files:\n");
    printf("  sudo mkdir -p /safe\n");
    printf("  sudo chown -R $(whoami) /safe\n");
    printf("  echo \"This is a safe report.\" > /safe/report.txt\n\n");


    const char* testFilenames[] = {
        "report.txt",             // 1. Valid file
        "nonexistent.txt",        // 2. Non-existent file
        "../../etc/passwd",       // 3. Path traversal attempt (blocked by initial check)
        "../safe/report.txt",     // 4. Another traversal (blocked by initial check)
        "/etc/passwd"             // 5. Absolute path (blocked by initial check)
    };
    int num_tests = sizeof(testFilenames) / sizeof(testFilenames[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Testing with filename: '%s' ---\n", testFilenames[i]);
        char* content = readFileFromSafeDirectory(testFilenames[i]);
        if (content != NULL) {
            printf("Successfully read file. Content:\n%s\n", content);
            free(content); // IMPORTANT: Free the allocated memory
        } else {
            printf("Failed to read file as expected.\n");
        }
        printf("----------------------------------------\n\n");
    }

    return 0;
}