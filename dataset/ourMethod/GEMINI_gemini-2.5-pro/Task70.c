#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> // For POSIX functions like getcwd, symlink, lstat
#include <limits.h> // For PATH_MAX

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define PATH_MAX MAX_PATH
#define getcwd _getcwd
#define mkdir(path, mode) _mkdir(path)
#endif

/**
 * @brief Reads the content of a file, ensuring the path is safe.
 *        The caller is responsible for freeing the returned buffer.
 *
 * @param filePath The path to the file.
 * @return A dynamically allocated string with the file content, or NULL on error.
 */
char* readFileContent(const char* filePath) {
    // Rule #4: Validate input
    if (filePath == NULL || *filePath == '\0') {
        fprintf(stderr, "Error: File path is empty.\n");
        return NULL;
    }

    struct stat path_stat;
    // Rule #7: Use lstat to check for symlinks without following them
    if (lstat(filePath, &path_stat) != 0) {
        perror("Error checking file status");
        return NULL;
    }

    // Rule #7: Refuse symlinks
    if (S_ISLNK(path_stat.st_mode)) {
        fprintf(stderr, "Error: Symbolic links are not allowed.\n");
        return NULL;
    }

    // Rule #7: Refuse non-regular files (e.g., directories)
    if (!S_ISREG(path_stat.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file.\n");
        return NULL;
    }

    // Rule #7: Prevent path traversal
    char resolved_path[PATH_MAX];
    // Use realpath (POSIX) or _fullpath (Windows) for canonicalization
#if defined(_WIN32) || defined(_WIN64)
    if (_fullpath(resolved_path, filePath, PATH_MAX) == NULL) {
#else
    if (realpath(filePath, resolved_path) == NULL) {
#endif
        perror("Error resolving path");
        return NULL;
    }

    char base_dir[PATH_MAX];
    if (getcwd(base_dir, sizeof(base_dir)) == NULL) {
        perror("Error getting current working directory");
        return NULL;
    }
    
    if (strncmp(resolved_path, base_dir, strlen(base_dir)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected.\n");
        return NULL;
    }
    
    FILE* file = fopen(resolved_path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }
    rewind(file);

    // Rule #1, #2: Allocate buffer with boundary checks
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory.\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file); // Close file as soon as possible
    
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Error reading file.\n");
        free(buffer);
        return NULL;
    }
    
    buffer[file_size] = '\0';
    return buffer;
}


void runTests() {
    printf("Running test cases...\n");
    const char* test_dir = "c_test_files";
    char safe_file[PATH_MAX], sub_dir[PATH_MAX], symlink_file[PATH_MAX];

    snprintf(safe_file, sizeof(safe_file), "%s/safe_file.txt", test_dir);
    snprintf(sub_dir, sizeof(sub_dir), "%s/sub", test_dir);
    snprintf(symlink_file, sizeof(symlink_file), "%s/slink.txt", test_dir);

    // Setup
    mkdir(test_dir, 0755);
    mkdir(sub_dir, 0755);
    FILE* f = fopen(safe_file, "w");
    if (f) {
        fprintf(f, "This is a safe file.");
        fclose(f);
    }
#ifndef _WIN32
    symlink("safe_file.txt", symlink_file);
#endif

    char* content;

    // Test Case 1: Valid file
    printf("\n--- Test Case 1: Valid file ---\n");
    printf("Reading: %s\n", safe_file);
    content = readFileContent(safe_file);
    if (content) {
        printf("Content: %s\n", content);
        free(content);
    }

    // Test Case 2: Non-existent file
    printf("\n--- Test Case 2: Non-existent file ---\n");
    const char* non_existent_path = "c_test_files/nonexistent.txt";
    printf("Reading: %s\n", non_existent_path);
    content = readFileContent(non_existent_path);
    if (content) {
        free(content); // Should not happen
    }

    // Test Case 3: Directory path
    printf("\n--- Test Case 3: Directory path ---\n");
    printf("Reading: %s\n", sub_dir);
    content = readFileContent(sub_dir);
    if (content) {
        free(content); // Should not happen
    }

    // Test Case 4: Symbolic link
#ifndef _WIN32
    printf("\n--- Test Case 4: Symbolic link ---\n");
    printf("Reading: %s\n", symlink_file);
    content = readFileContent(symlink_file);
    if (content) {
        free(content); // Should not happen
    }
#else
    printf("\n--- Test Case 4: Symbolic link (Skipped on Windows) ---\n");
#endif

    // Test Case 5: Path traversal
    printf("\n--- Test Case 5: Path traversal ---\n");
    const char* traversal_path = "../";
    printf("Reading: %s\n", traversal_path);
    content = readFileContent(traversal_path);
    if (content) {
        free(content); // Should not happen
    }

    // Cleanup
#ifndef _WIN32
    remove(symlink_file);
#endif
    remove(safe_file);
    rmdir(sub_dir);
    rmdir(test_dir);
    printf("\nTest cleanup complete.\n");
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* content = readFileContent(argv[1]);
        if (content != NULL) {
            printf("%s\n", content);
            free(content);
        } else {
            // Error message is printed by the function
            return 1;
        }
    } else {
        runTests();
    }
    return 0;
}