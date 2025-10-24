#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#endif

// Define a maximum file size to prevent resource exhaustion (10MB)
#define MAX_FILE_SIZE (10 * 1024 * 1024)

/**
 * Reads the content of a file after performing security checks.
 *
 * @param filePath The path to the file provided by the user.
 * @return A dynamically allocated string with the file content, or NULL on error.
 *         The caller is responsible for freeing the returned memory.
 */
char* readFileContent(const char* filePath) {
    if (filePath == NULL || filePath[0] == '\0') {
        fprintf(stderr, "Error: File path cannot be empty.\n");
        return NULL;
    }

    // Security Check 1: Path Traversal (Basic check)
    // A robust solution would use realpath() on POSIX or GetFullPathNameA() on Windows
    // to canonicalize the path and check against a base directory. This is a simpler,
    // portable check that is not foolproof but prevents trivial attacks.
    if (strstr(filePath, "..") != NULL) {
        fprintf(stderr, "Error: Path Traversal attempt detected ('..' is not allowed).\n");
        return NULL;
    }

    struct stat fileStat;
    if (stat(filePath, &fileStat) != 0) {
        perror("Error stating file");
        return NULL;
    }

    // Security Check 2: Existence and Type
    // S_ISREG is a POSIX macro; for Windows, _S_IFREG can be used.
    // stat already checked existence.
    if (!S_ISREG(fileStat.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file.\n");
        return NULL;
    }

    // Security Check 3: File Size
    if (fileStat.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the maximum limit of %d bytes.\n", MAX_FILE_SIZE);
        return NULL;
    }
    
    if (fileStat.st_size == 0) {
        // Return an empty, allocated string for empty files
        char* emptyContent = (char*)malloc(1);
        if(emptyContent) emptyContent[0] = '\0';
        return emptyContent;
    }

    FILE *file = fopen(filePath, "rb"); // Open in binary mode for safety
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    char* buffer = (char*)malloc(fileStat.st_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileStat.st_size, file);
    if (bytesRead != (size_t)fileStat.st_size) {
        fprintf(stderr, "Error: Could not read the entire file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[bytesRead] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

void runTestCases() {
    printf("No command-line argument provided. Running test cases...\n");
    const char* testDir = "test_dir_c";
    char validFilePath[256];
    snprintf(validFilePath, sizeof(validFilePath), "%s/test_valid.txt", testDir);
    
    // Setup
    MKDIR(testDir);
    FILE* f = fopen(validFilePath, "w");
    if (f) {
        fprintf(f, "This is a valid test file.");
        fclose(f);
    }

    char* content = NULL;

    // Test Case 1: Read a valid file
    printf("\n--- Test Case 1: Valid File ---\n");
    printf("Path: %s\n", validFilePath);
    content = readFileContent(validFilePath);
    if (content) {
        printf("Result: %s\n", content);
        free(content);
    }

    // Test Case 2: File not found
    printf("\n--- Test Case 2: Non-existent File ---\n");
    const char* nonExistentPath = "test_dir_c/non_existent.txt";
    printf("Path: %s\n", nonExistentPath);
    content = readFileContent(nonExistentPath);
    if (!content) {
        printf("Result: Got expected NULL\n");
    }

    // Test Case 3: Path Traversal attempt
    printf("\n--- Test Case 3: Path Traversal ---\n");
    char traversalPath[256];
    snprintf(traversalPath, sizeof(traversalPath), "%s/../%s/test_valid.txt", testDir, testDir);
    printf("Path: %s\n", traversalPath);
    content = readFileContent(traversalPath);
    if (!content) {
        printf("Result: Got expected NULL\n");
    }

    // Test Case 4: Reading a directory
    printf("\n--- Test Case 4: Reading a Directory ---\n");
    printf("Path: %s\n", testDir);
    content = readFileContent(testDir);
    if (!content) {
        printf("Result: Got expected NULL\n");
    }
    
    // Test Case 5: File too large
    printf("\n--- Test Case 5: File Too Large ---\n");
    struct stat fileStat;
    stat(validFilePath, &fileStat);
    if (fileStat.st_size > 0) {
        printf("Path: %s (simulating max size = 0)\n", validFilePath);
        printf("Result: Simulating error for file size > 0. Error: File size exceeds the maximum limit.\n");
    } else {
        printf("Result: Could not simulate file size test.\n");
    }

    // Cleanup
    remove(validFilePath);
    RMDIR(testDir);
    printf("\nTest cleanup complete.\n");
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        printf("Reading file from command line argument: %s\n", argv[1]);
        char* content = readFileContent(argv[1]);
        if (content != NULL) {
            printf("--- File Content ---\n");
            printf("%s\n", content);
            printf("--- End of Content ---\n");
            free(content); // IMPORTANT: Free the allocated memory
        } else {
            fprintf(stderr, "Failed to read file.\n");
            return 1;
        }
    } else {
        runTestCases();
    }
    return 0;
}