#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the content of a file after validating the filename.
 *
 * @param filename The name of the file to read.
 * @return A dynamically allocated string with the file content.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if an error occurs.
 */
char* readFileContent(const char* filename) {
    if (filename == NULL || *filename == '\0') {
        fprintf(stderr, "Error: Filename cannot be null or empty.\n");
        return NULL;
    }

    // Security: Prevent path traversal attacks.
    if (strstr(filename, "..") != NULL) {
        fprintf(stderr, "Error: Invalid filename. Path traversal is not allowed.\n");
        return NULL;
    }

    FILE* file = fopen(filename, "rb"); // Use "rb" for binary-safe size calculation
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize < 0) {
        perror("Error determining file size");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the content + null terminator
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    // Read file into buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        fprintf(stderr, "Error: Mismatch in bytes read from file.\n");
        fclose(file);
        free(buffer);
        return NULL;
    }

    buffer[fileSize] = '\0'; // Null-terminate the string
    fclose(file);

    return buffer;
}


// Helper to create a test file
void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
    } else {
        fprintf(stderr, "Failed to create test file: %s\n", filename);
    }
}

// Helper to run test cases
void runTests() {
    printf("--- Running Tests ---\n");
    char* result;

    // Test Case 1: Valid file with content
    printf("\n--- Test Case 1: Read a valid file ---\n");
    const char* testFile1 = "test1.txt";
    const char* content1 = "Hello World!";
    createTestFile(testFile1, content1);
    result = readFileContent(testFile1);
    printf("Expected: %s\n", content1);
    printf("Got: %s\n", result ? result : "NULL");
    printf("Test %s\n", (result && strcmp(content1, result) == 0) ? "PASSED" : "FAILED");
    free(result);

    // Test Case 2: Empty file
    printf("\n--- Test Case 2: Read an empty file ---\n");
    const char* testFile2 = "test2.txt";
    createTestFile(testFile2, "");
    result = readFileContent(testFile2);
    printf("Expected: \n");
    printf("Got: %s\n", result ? result : "NULL");
    printf("Test %s\n", (result && strcmp("", result) == 0) ? "PASSED" : "FAILED");
    free(result);

    // Test Case 3: Non-existent file
    printf("\n--- Test Case 3: Read a non-existent file ---\n");
    result = readFileContent("nonexistent.txt");
    printf("Expected: NULL (with error message)\n");
    printf("Got: %s\n", result ? result : "NULL");
    printf("Test %s\n", (result == NULL) ? "PASSED" : "FAILED");
    free(result);

    // Test Case 4: Path traversal attempt
    printf("\n--- Test Case 4: Path traversal attempt ---\n");
    result = readFileContent("../somefile.txt");
    printf("Expected: NULL (with error message)\n");
    printf("Got: %s\n", result ? result : "NULL");
    printf("Test %s\n", (result == NULL) ? "PASSED" : "FAILED");
    free(result);

    // Test Case 5: File with multi-line content
    printf("\n--- Test Case 5: Read a multi-line file ---\n");
    const char* testFile5 = "test5.txt";
    const char* content5 = "Line 1\nLine 2\nLine 3";
    createTestFile(testFile5, content5);
    result = readFileContent(testFile5);
    printf("Expected:\n%s\n", content5);
    printf("Got:\n%s\n", result ? result : "NULL");
    printf("Test %s\n", (result && strcmp(content5, result) == 0) ? "PASSED" : "FAILED");
    free(result);

    // Cleanup
    remove(testFile1);
    remove(testFile2);
    remove(testFile5);

    printf("\n--- Tests Finished ---\n");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* filename = argv[1];
        printf("Attempting to read file from command line: %s\n", filename);
        char* content = readFileContent(filename);
        if (content != NULL) {
            printf("\n--- File Content ---\n");
            printf("%s\n", content);
            printf("--- End of Content ---\n");
            free(content); // Free the allocated memory
        } else {
            printf("Failed to read file.\n");
        }
    } else {
        printf("No filename provided via command line. Running built-in test cases.\n");
        runTests();
    }
    return 0;
}