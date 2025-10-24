
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_FILENAME_LEN 255
#define BUFFER_SIZE 4096

/* Validates that the filename contains only safe characters */
bool isValidFilename(const char* filename) {
    size_t len;
    const char* dangerous = "|&;$`\\n<>()[]{}*?!~'\\"";
    
    /* Check NULL pointer */
    if (filename == NULL) {
        return false;
    }
    
    /* Check length: must be non-empty and not exceed limit */
    len = strlen(filename);
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return false;
    }
    
    /* Reject absolute paths and directory traversal */
    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\\\') != NULL ||\n        filename[0] == '.' ||\n        (len > 1 && filename[1] == ':')) { /* Windows drive letter */\n        return false;\n    }\n    \n    /* Reject shell metacharacters and control characters */\n    for (size_t i = 0; i < len; i++) {\n        unsigned char c = (unsigned char)filename[i];\n        if (strchr(dangerous, c) != NULL || c < 32 || c == 127) {\n            return false;\n        }\n    }\n    \n    return true;\n}\n\n/* Safely reads and displays file contents without using system commands */\nbool displayFileContents(const char* filename) {\n    FILE* file = NULL;\n    char* buffer = NULL;\n    size_t bytesRead;\n    bool success = false;\n    \n    /* Allocate buffer */\n    buffer = (char*)malloc(BUFFER_SIZE);\n    if (buffer == NULL) {\n        fprintf(stderr, "Error: Memory allocation failed\\n");\n        return false;\n    }\n    \n    /* Open file in binary read mode */\n    file = fopen(filename, "rb");\n    if (file == NULL) {\n        fprintf(stderr, "Error: Unable to open file '%s'\\n", filename);\n        free(buffer);\n        return false;\n    }\n    \n    /* Read and display contents safely */\n    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {\n        /* Write to stdout with bounds checking */\n        if (fwrite(buffer, 1, bytesRead, stdout) != bytesRead) {\n            fprintf(stderr, "Error: Failed to write output\\n");\n            goto cleanup;\n        }\n    }\n    \n    /* Check for read errors */\n    if (ferror(file)) {\n        fprintf(stderr, "Error: Failed to read file\\n");\n        goto cleanup;\n    }\n    \n    success = true;\n\ncleanup:\n    /* Clean up resources */\n    if (file != NULL) {\n        fclose(file);\n    }\n    if (buffer != NULL) {\n        free(buffer);\n    }\n    \n    return success;\n}\n\nint main(void) {\n    const char* testCases[] = {\n        "test1.txt",\n        "test2.txt",\n        "test3.txt",\n        "test4.txt",\n        "test5.txt"\n    };\n    size_t numTests = sizeof(testCases) / sizeof(testCases[0]);\n    char filename[MAX_FILENAME_LEN + 1];\n    FILE* testFile = NULL;\n    \n    /* Create test files for demonstration */\n    for (size_t i = 0; i < numTests; i++) {\n        testFile = fopen(testCases[i], "w");\n        if (testFile != NULL) {\n            fprintf(testFile, "Test content for file %zu\\n", i + 1);\n            fclose(testFile);\n            testFile = NULL;\n        }\n    }\n    \n    printf("=== Running Test Cases ===\\n");\n    for (size_t i = 0; i < numTests; i++) {\n        printf("\\n--- Testing file: %s ---\\n", testCases[i]);\n        \n        /* Validate filename before processing */\n        if (!isValidFilename(testCases[i])) {\n            fprintf(stderr, "Error: Invalid filename '%s'\\n", testCases[i]);\n            continue;\n        }\n        \n        /* Display contents safely without system commands */\n        if (!displayFileContents(testCases[i])) {\n            fprintf(stderr, "Failed to display file contents\\n");\n        }\n    }\n    \n    printf("\\n\\n=== Interactive Mode ===\\n");\n    printf("Enter filename to display (or 'quit' to exit): ");\n    \n    /* Read filename with safe input handling */\n    while (fgets(filename, sizeof(filename), stdin) != NULL) {\n        size_t len;\n        \n        /* Remove trailing newline */\n        len = strlen(filename);\n        if (len > 0 && filename[len - 1] == '\
') {\n            filename[len - 1] = '\\0';\n            len--;\n        }\n        \n        /* Check for quit command */\n        if (strcmp(filename, "quit") == 0) {\n            break;\n        }\n        \n        /* Validate input filename */\n        if (!isValidFilename(filename)) {\n            fprintf(stderr, "Error: Invalid filename. Filenames must:\\n");\n            fprintf(stderr, "  - Be 1-255 characters\\n");\n            fprintf(stderr, "  - Not contain path separators or '..'\\n");\n            fprintf(stderr, "  - Not contain shell metacharacters\\n");\n            printf("\\nEnter filename to display (or 'quit' to exit): ");\n            continue;\n        }\n        \n        /* Display file contents using safe file I/O */\n        displayFileContents(filename);\n        \n        printf("\\nEnter filename to display (or 'quit' to exit): ");
    }
    
    return 0;
}
