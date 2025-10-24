#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#define PATH_MAX _MAX_PATH
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#include <limits.h>
#endif

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

/**
 * Securely reads the content of a file specified by a path.
 * The path is validated to prevent directory traversal attacks. The file must
 * exist within the current working directory.
 * The caller is responsible for freeing the returned buffer.
 *
 * @param userPath The path to the file provided by the user.
 * @return A dynamically allocated string with the file content, or NULL on error.
 */
char* readFileContent(const char* userPath) {
    if (userPath == NULL || *userPath == '\0') {
        fprintf(stderr, "Error: File path cannot be null or empty.\n");
        return NULL;
    }

    // Get canonical path of the allowed base directory (current working directory)
    char base_path_real[PATH_MAX];
    if (realpath(".", base_path_real) == NULL) {
        perror("Error resolving base path");
        return NULL;
    }

    // Get canonical path of the requested file.
    // realpath resolves ".." and symlinks, and also checks if the file exists.
    char user_path_real[PATH_MAX];
    if (realpath(userPath, user_path_real) == NULL) {
        perror("Error resolving user path (check if file exists)");
        return NULL;
    }

    // Security Check 1: Ensure the requested path is within the base directory.
    if (strncmp(base_path_real, user_path_real, strlen(base_path_real)) != 0) {
        fprintf(stderr, "Error: Directory traversal attempt detected. Access denied.\n");
        return NULL;
    }

    // Security Check 2: Use stat to ensure it's a regular file.
    struct stat statbuf;
    if (stat(user_path_real, &statbuf) != 0) {
        perror("Error getting file stats");
        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file.\n");
        return NULL;
    }
    
    // Security Check 3: Check file size to prevent DoS
    if (statbuf.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File is too large.\n");
        return NULL;
    }
    
    if (statbuf.st_size == 0) {
        // Return an empty, but valid, string for empty files
        char* empty_buffer = (char*)malloc(1);
        if (empty_buffer) {
            empty_buffer[0] = '\0';
        }
        return empty_buffer;
    }

    // Open and read the file
    FILE* file = fopen(user_path_real, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char* buffer = (char*)malloc(statbuf.st_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, statbuf.st_size, file);
    if (bytes_read != (size_t)statbuf.st_size) {
        fprintf(stderr, "Error: Failed to read the entire file.\n");
        fclose(file);
        free(buffer);
        return NULL;
    }
    
    buffer[bytes_read] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

void run_tests() {
    printf("Running integrated test cases...\n");

    const char* test_sub_dir = "test_dir";
    const char* safe_file = "safe.txt";
    const char* nested_file = "test_dir/nested.txt"; // Use forward slashes for portability

    // Setup
#ifdef _WIN32
    mkdir(test_sub_dir);
#else
    mkdir(test_sub_dir, 0755);
#endif
    FILE *f_safe = fopen(safe_file, "w");
    if (f_safe) {
        fprintf(f_safe, "This is a safe file.");
        fclose(f_safe);
    }
    FILE *f_nested = fopen(nested_file, "w");
    if (f_nested) {
        fprintf(f_nested, "This is a nested file.");
        fclose(f_nested);
    }
    
    char* content = NULL;

    // --- Test Cases ---
    printf("\n--- Test Case 1: Valid file in CWD ---\n");
    printf("Input: %s\n", safe_file);
    content = readFileContent(safe_file);
    if (content) {
        printf("Output: \n%s\n", content);
        free(content);
    }

    printf("\n--- Test Case 2: Valid nested file ---\n");
    printf("Input: %s\n", nested_file);
    content = readFileContent(nested_file);
    if (content) {
        printf("Output: \n%s\n", content);
        free(content);
    }
    
    printf("\n--- Test Case 3: Path Traversal attempt ---\n");
    const char* traversal_path = "../safe.txt";
    printf("Input: %s\n", traversal_path);
    content = readFileContent(traversal_path);
    if (content) {
        printf("Output: \n%s\n", content); // Should not print
        free(content);
    }

    printf("\n--- Test Case 4: Non-existent file ---\n");
    const char* non_existent_file = "nonexistent.txt";
    printf("Input: %s\n", non_existent_file);
    content = readFileContent(non_existent_file);
    if (content) {
        printf("Output: \n%s\n", content); // Should not print
        free(content);
    }

    printf("\n--- Test Case 5: Attempt to read a directory ---\n");
    printf("Input: %s\n", test_sub_dir);
    content = readFileContent(test_sub_dir);
    if (content) {
        printf("Output: \n%s\n", content); // Should not print
        free(content);
    }

    // Cleanup
    remove(nested_file);
    remove(safe_file);
    rmdir(test_sub_dir);
}


int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        printf("Reading file from command line argument: %s\n", argv[1]);
        char* content = readFileContent(argv[1]);
        if (content != NULL) {
            printf("--- File Content ---\n");
            printf("%s\n", content);
            printf("--- End of Content ---\n");
            free(content);
        } else {
            fprintf(stderr, "Failed to read file content.\n");
            return 1;
        }
    } else {
        run_tests();
    }

    return 0;
}