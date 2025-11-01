#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#if defined(_WIN32) || defined(_WIN64)
    #define REALPATH(path, resolved) _fullpath((resolved), (path), _MAX_PATH)
    #define PATH_MAX _MAX_PATH
#else
    #define REALPATH(path, resolved) realpath((path), (resolved))
#endif

#define SAFE_DIRECTORY "/safe/"

/**
 * Reads a file from the predefined SAFE_DIRECTORY, preventing path traversal.
 *
 * @param filename The name of the file to read.
 * @return A dynamically allocated string with the file content, or NULL on error.
 *         The caller is responsible for freeing the returned string.
 */
char* readFile(const char* filename) {
    // Security: Basic check for traversal sequences
    if (filename == NULL || *filename == '\0' || strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return NULL;
    }

    char full_path[PATH_MAX];
    int written = snprintf(full_path, sizeof(full_path), "%s%s", SAFE_DIRECTORY, filename);

    if (written < 0 || written >= sizeof(full_path)) {
        fprintf(stderr, "Error: Constructed path is too long.\n");
        return NULL;
    }

    char resolved_path[PATH_MAX];
    // Security: Resolve the path to its canonical form
    if (REALPATH(full_path, resolved_path) == NULL) {
        // This can fail if the file doesn't exist, which is a valid case.
        // We still need to check the constructed path before giving up.
        // If realpath fails for other reasons (e.g., permissions), we should stop.
    }

    char safe_dir_resolved[PATH_MAX];
    if (REALPATH(SAFE_DIRECTORY, safe_dir_resolved) == NULL) {
        fprintf(stderr, "Error: Safe directory does not exist or is inaccessible.\n");
        return NULL;
    }

    // Security Check: Verify the resolved path starts with the safe directory's path.
    if (strncmp(resolved_path, safe_dir_resolved, strlen(safe_dir_resolved)) != 0) {
        fprintf(stderr, "Error: Directory traversal attempt detected.\n");
        return NULL;
    }
    
    // An additional check to ensure it's not a partial match like /safe-dir/
    if (resolved_path[strlen(safe_dir_resolved)] != '/' && resolved_path[strlen(safe_dir_resolved)] != '\\' && resolved_path[strlen(safe_dir_resolved)] != '\0') {
        fprintf(stderr, "Error: Directory traversal attempt detected (path mismatch).\n");
        return NULL;
    }

    FILE* file = fopen(resolved_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: File is not readable or does not exist.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length < 0) {
        fclose(file);
        fprintf(stderr, "Error: Could not determine file size.\n");
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        return NULL;
    }

    if (fread(buffer, 1, length, file) != (size_t)length) {
        fclose(file);
        free(buffer);
        fprintf(stderr, "Error: Could not read the entire file.\n");
        return NULL;
    }
    
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

void run_test(const char* test_name, const char* filename) {
    printf("%s '%s':\n", test_name, filename);
    char* content = readFile(filename);
    if (content != NULL) {
        printf("%s\n", content);
        free(content);
    }
    printf("\n");
}

int main() {
    // Setup test environment
    #if defined(_WIN32) || defined(_WIN64)
        _mkdir(SAFE_DIRECTORY);
    #else
        mkdir(SAFE_DIRECTORY, 0755);
    #endif
    
    FILE *f1 = fopen(SAFE_DIRECTORY "test1.txt", "w");
    if (f1) { fprintf(f1, "This is a safe file."); fclose(f1); }
    FILE *f2 = fopen(SAFE_DIRECTORY "test2.log", "w");
    if (f2) { fprintf(f2, "Another safe log file."); fclose(f2); }
    FILE *f3 = fopen("unsafe.txt", "w");
    if (f3) { fprintf(f3, "This is an unsafe file."); fclose(f3); }
    
    printf("--- C Test Cases ---\n");
    
    // Test Case 1: Valid file
    run_test("1. Reading", "test1.txt");

    // Test Case 2: Another valid file
    run_test("2. Reading", "test2.log");

    // Test Case 3: Non-existent file
    run_test("3. Reading", "nonexistent.txt");
    
    // Test Case 4: Path traversal attempt
    run_test("4. Reading", "../unsafe.txt");
    
    // Test Case 5: Absolute path attempt
    run_test("5. Reading", "/etc/passwd");

    return 0;
}