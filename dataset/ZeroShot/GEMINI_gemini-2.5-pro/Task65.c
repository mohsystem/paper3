#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Validates a filename against a secure whitelist.
 *
 * Allows: a-z, A-Z, 0-9, '.', '_', '-'
 * Disallows: path separators ('/', '\') and traversal sequences ('..')
 *
 * @param filename The filename string to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool isValidFilename(const char* filename) {
    if (filename == NULL || *filename == '\0') {
        return false;
    }

    // Check for disallowed substrings
    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return false;
    }

    // Whitelist check for each character
    for (size_t i = 0; i < strlen(filename); ++i) {
        char c = filename[i];
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Securely reads the content of a file after validating the filename.
 *
 * @param filename The name of the file to read.
 * @param errorMsg A pointer to a char pointer to store a static error message if one occurs.
 * @return A dynamically allocated string containing the file content, or NULL on error.
 *         The caller is responsible for freeing the returned string.
 */
char* getFileContent(const char* filename, const char** errorMsg) {
    // 1. Validate Input
    if (!isValidFilename(filename)) {
        *errorMsg = "Error: Invalid filename format. Path separators and special characters are not allowed.";
        return NULL;
    }

    // 2. Open File
    FILE* file = fopen(filename, "rb"); // Open in binary read mode for safety
    if (file == NULL) {
        *errorMsg = "Error: Could not open file. It may not exist or you may not have permission to read it.";
        return NULL;
    }

    // 3. Determine File Size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        *errorMsg = "Error: Could not determine file size.";
        fclose(file);
        return NULL;
    }
    rewind(file);

    // 4. Allocate Memory (+1 for null terminator)
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        *errorMsg = "Error: Could not allocate memory to read file.";
        fclose(file);
        return NULL;
    }

    // 5. Read File
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        *errorMsg = "Error: Could not read the entire file.";
        fclose(file);
        free(buffer);
        return NULL;
    }
    
    buffer[bytes_read] = '\0'; // Null-terminate the string

    // 6. Cleanup and Return
    fclose(file);
    *errorMsg = NULL; // Success
    return buffer;
}

void setupTestFiles() {
    FILE* f = fopen("test1.txt", "w");
    if (f) {
        fprintf(f, "This is a secure file.\n");
        fclose(f);
    } else {
        perror("Warning: Could not create test file 'test1.txt'");
    }
}

int main() {
    setupTestFiles();

    const char* testCases[] = {
        "test1.txt",              // 1. Valid file
        "non_existent.txt",       // 2. Non-existent file
        "../other_dir/secret.txt",// 3. Path traversal attempt
        "safe_dir/test2.txt",     // 4. Filename with directory separator
        "/etc/passwd"             // 5. Absolute path attempt
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("--- Test Case %d: \"%s\" ---\n", i + 1, testCases[i]);
        
        const char* errorMessage = NULL;
        char* content = getFileContent(testCases[i], &errorMessage);
        
        if (content != NULL) {
            printf("Result:\n%s", content);
            free(content); // IMPORTANT: Free the allocated memory
        } else {
            printf("Result:\n%s\n", errorMessage);
        }
        printf("\n");
    }

    return 0;
}