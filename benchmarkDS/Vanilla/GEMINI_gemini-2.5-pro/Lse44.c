#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Cross-platform compatibility for directory creation and path resolution
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define PATH_MAX MAX_PATH
#define REALPATH(path, resolved) _fullpath(resolved, path, PATH_MAX)
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#define REALPATH(path, resolved) realpath(path, resolved)
#endif

// Returns a dynamically allocated string with file content, or a static error string.
// Caller is responsible for freeing the returned buffer on success.
char* readFileFromSafeDirectory(const char* filename) {
    // Basic sanitization
    if (filename == NULL || *filename == '\0' || strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return "Error: Invalid filename.";
    }

    char safe_dir_path[PATH_MAX];
    if (REALPATH("safe", safe_dir_path) == NULL) {
        // This might fail if 'safe' dir doesn't exist, which it should in the test
        return "Error: Could not resolve safe directory path.";
    }

    char requested_path_str[PATH_MAX];
    snprintf(requested_path_str, sizeof(requested_path_str), "safe%c%s",
#ifdef _WIN32
        '\\',
#else
        '/',
#endif
        filename);

    char resolved_path[PATH_MAX];
    if (REALPATH(requested_path_str, resolved_path) == NULL) {
        return "Error: File not found or path cannot be resolved.";
    }

    // Security check: ensure resolved path starts with the safe directory path.
    size_t safe_dir_len = strlen(safe_dir_path);
    if (strncmp(resolved_path, safe_dir_path, safe_dir_len) != 0) {
        return "Error: Directory traversal attempt detected.";
    }

    // Check if it's a regular file
    struct stat path_stat;
    if (stat(resolved_path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) {
        return "Error: Path is not a regular file.";
    }

    FILE *file = fopen(resolved_path, "rb");
    if (file == NULL) {
        return "Error: Could not open file.";
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    if (file_size < 0) {
        fclose(file);
        return "Error: Could not determine file size.";
    }

    char *buffer = (char*) malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return "Error: Memory allocation failed.";
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        free(buffer);
        return "Error: Failed to read entire file.";
    }
    buffer[file_size] = '\0';
    return buffer;
}


void setup_environment() {
    MKDIR("safe");
    FILE *f1 = fopen("safe/test1.txt", "w");
    if (f1) {
        fprintf(f1, "This is test file 1.");
        fclose(f1);
    }
    FILE *f2 = fopen("safe/another_file.log", "w");
    if (f2) {
        fprintf(f2, "Log entry.");
        fclose(f2);
    }
    printf("Test environment set up.\n");
}

void cleanup_environment() {
    remove("safe/test1.txt");
    remove("safe/another_file.log");
    RMDIR("safe");
    printf("\nTest environment cleaned up.\n");
}


int main() {
    setup_environment();

    const char* test_files[] = {
        "test1.txt",              // 1. Valid file
        "another_file.log",       // 2. Another valid file
        "nonexistent.txt",        // 3. Non-existent file
        "../secret.txt",          // 4. Directory traversal attempt
        "../../etc/passwd"        // 5. More complex traversal attempt
    };
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);

    printf("\n--- Running Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Test Case %d: Requesting '%s'\n", i + 1, test_files[i]);
        char* content = readFileFromSafeDirectory(test_files[i]);
        printf("Result: %s\n", content);
        
        // Free memory if it was a successful read (not a static error string)
        if (strncmp(content, "Error:", 6) != 0) {
            free(content);
        }
        printf("----------------------------\n");
    }

    cleanup_environment();
    return 0;
}