#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the entire content of a file into a dynamically allocated string.
 * The caller is responsible for freeing the returned memory.
 *
 * @param filePath The path to the file.
 * @return A pointer to a newly allocated string containing the file content,
 *         or a pointer to a newly allocated error message string if an error occurs.
 *         The caller must always free the returned pointer.
 */
char* readFileContent(const char* filePath) {
    FILE* file = fopen(filePath, "rb"); // Open in binary mode to correctly get file size
    if (file == NULL) {
        const char* err_prefix = "Error: Could not open file '";
        const char* err_suffix = "'.";
        char* error_msg = (char*)malloc(strlen(err_prefix) + strlen(filePath) + strlen(err_suffix) + 1);
        if (error_msg == NULL) return NULL;
        strcpy(error_msg, err_prefix);
        strcat(error_msg, filePath);
        strcat(error_msg, err_suffix);
        return error_msg;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(file);
        const char* err_msg = "Error: Could not allocate memory for file content.";
        char* mem_error = (char*)malloc(strlen(err_msg) + 1);
        if (mem_error == NULL) return NULL;
        strcpy(mem_error, err_msg);
        return mem_error;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        fclose(file);
        free(buffer);
        const char* err_msg = "Error: Could not read the entire file.";
        char* read_error = (char*)malloc(strlen(err_msg) + 1);
        if (read_error == NULL) return NULL;
        strcpy(read_error, err_msg);
        return read_error;
    }

    buffer[fileSize] = '\0';
    fclose(file);
    return buffer;
}

// Helper function to create a test file
void createTestFile(const char* fileName, const char* content) {
    FILE* file = fopen(fileName, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    }
}

void runTestCase(const char* testName, const char* fileName, const char* fileContent, int create_file) {
    printf("%s\n", testName);
    if (create_file) {
        createTestFile(fileName, fileContent);
    }
    
    char* content = readFileContent(fileName);
    if (content != NULL) {
        printf("Content: %s\n", content);
        free(content);
    } else {
        printf("Content: (null) - memory allocation for error message failed.\n");
    }
    
    if (create_file) {
        remove(fileName);
    }
    printf("---------------------------------------------------------\n");
}

int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        const char* filePath = argv[1];
        printf("Reading content from command line argument: %s\n", filePath);
        printf("--- File Content ---\n");
        char* content = readFileContent(filePath);
        if (content != NULL) {
            printf("%s\n", content);
            free(content);
        } else {
             printf("Failed to read file or allocate memory.\n");
        }
        printf("--- End of Content ---\n");
        return 0;
    }

    // Run built-in test cases if no command-line argument is provided
    printf("No command line argument provided. Running built-in test cases...\n");
    printf("=========================================================\n");

    // Test Case 1: Simple file
    runTestCase("Test Case 1: Reading a simple file (test1.txt)", "test1.txt", "Hello World from C!", 1);
    
    // Test Case 2: File with multiple lines
    printf("Test Case 2: Reading a multi-line file (test2.txt)\n");
    createTestFile("test2.txt", "Line 1\nLine 2\nLine 3");
    char* multiLineContent = readFileContent("test2.txt");
    if(multiLineContent) {
        printf("Content:\n%s\n", multiLineContent);
        free(multiLineContent);
    }
    remove("test2.txt");
    printf("---------------------------------------------------------\n");

    // Test Case 3: Empty file
    printf("Test Case 3: Reading an empty file (test3.txt)\n");
    createTestFile("test3.txt", "");
    char* emptyContent = readFileContent("test3.txt");
    if (emptyContent != NULL) {
        printf("Content: '%s'\n", emptyContent);
        free(emptyContent);
    }
    remove("test3.txt");
    printf("---------------------------------------------------------\n");

    // Test Case 4: Non-existent file
    runTestCase("Test Case 4: Attempting to read a non-existent file", "nonexistentfile.txt", "", 0);
    
    // Test Case 5: File with special characters
    runTestCase("Test Case 5: Reading a file with special characters (test5.txt)", "test5.txt", "UTF-8 test: áéíóú ñ €", 1);

    printf("=========================================================\n");

    return 0;
}