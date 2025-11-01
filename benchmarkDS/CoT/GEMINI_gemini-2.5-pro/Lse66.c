#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h> // For mkdir on POSIX
#include <errno.h>

// For Windows compatibility for creating directories
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755) // 0755 permissions: rwxr-xr-x
#endif

#define UPLOAD_DIRECTORY "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB
#define MAX_FILENAME_LEN 256
#define MAX_PATH_LEN 512

/**
 * Creates a directory if it doesn't exist.
 * @param path The path of the directory to create.
 * @return 0 on success or if directory exists, -1 on failure.
 */
int create_dir_if_not_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (MKDIR(path) != 0 && errno != EEXIST) {
            perror("Error creating directory");
            return -1;
        }
    }
    return 0;
}

/**
 * Accepts a PDF file's name and content, validates it, and saves it.
 * @param originalFilename The original filename from the user.
 * @param content The file content as a char array.
 * @param contentLength The size of the content.
 * @return A string message (caller must free this memory).
 */
char* savePdfFile(const char* originalFilename, const char* content, size_t contentLength) {
    char* result_msg = (char*)malloc(MAX_PATH_LEN);
    if (!result_msg) return NULL;

    // 1. Security Check: File Size
    if (contentLength > MAX_FILE_SIZE) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: File size exceeds the limit of %d MB.", (int)(MAX_FILE_SIZE / (1024 * 1024)));
        return result_msg;
    }
    if (contentLength == 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: File content cannot be empty.");
        return result_msg;
    }

    // 2. Security Check: Filename Validation
    if (originalFilename == NULL || strlen(originalFilename) == 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: Filename is missing.");
        return result_msg;
    }

    // Strip path information to prevent Path Traversal
    const char* basename = strrchr(originalFilename, '/');
    if (basename == NULL) {
        basename = strrchr(originalFilename, '\\');
    }
    basename = (basename == NULL) ? originalFilename : basename + 1;

    // 3. Security Check: File Type (extension check)
    const char* dot = strrchr(basename, '.');
    if (dot == NULL || strcmp(dot, ".pdf") != 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: Invalid file type. Only PDF files are allowed.");
        return result_msg;
    }

    // 4. Security Check: Sanitize filename characters
    char sanitizedFilename[MAX_FILENAME_LEN] = {0};
    int j = 0;
    for (int i = 0; basename[i] != '\0' && j < MAX_FILENAME_LEN - 1; i++) {
        if (isalnum((unsigned char)basename[i]) || basename[i] == '.' || basename[i] == '_' || basename[i] == '-') {
            sanitizedFilename[j++] = basename[i];
        }
    }
    sanitizedFilename[j] = '\0'; // Null-terminate

    if (strlen(sanitizedFilename) == 0 || strcmp(sanitizedFilename, ".pdf") == 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: Invalid filename after sanitization.");
        return result_msg;
    }

    // Create directory
    if (create_dir_if_not_exists(UPLOAD_DIRECTORY) != 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: Could not create upload directory.");
        return result_msg;
    }

    // Construct final path securely
    char destinationPath[MAX_PATH_LEN];
    int written = snprintf(destinationPath, MAX_PATH_LEN, "%s/%s", UPLOAD_DIRECTORY, sanitizedFilename);
    if (written >= MAX_PATH_LEN || written < 0) {
        snprintf(result_msg, MAX_PATH_LEN, "Error: Destination path is too long.");
        return result_msg;
    }
    
    // Write the file
    FILE *fp = fopen(destinationPath, "wb");
    if (fp == NULL) {
        perror("Error opening file for writing");
        snprintf(result_msg, MAX_PATH_LEN, "Error: Could not open file for writing.");
        return result_msg;
    }
    
    if (fwrite(content, 1, contentLength, fp) != contentLength) {
        fclose(fp);
        snprintf(result_msg, MAX_PATH_LEN, "Error: Failed to write all content to file.");
        return result_msg;
    }
    
    fclose(fp);
    snprintf(result_msg, MAX_PATH_LEN, "Success: File '%s' saved successfully in '%s' folder.", sanitizedFilename, UPLOAD_DIRECTORY);
    return result_msg;
}

void run_test(const char* test_name, const char* filename, const char* content, size_t content_len) {
    char* result = savePdfFile(filename, content, content_len);
    if (result) {
        printf("%s: %s\n", test_name, result);
        free(result);
    } else {
        printf("%s: Failed to get result (memory allocation failed).\n", test_name);
    }
}

int main() {
    printf("--- Running C Test Cases ---\n");
    const char* valid_content = "This is a dummy PDF content.";
    size_t valid_content_len = strlen(valid_content);
    
    char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
    if(large_content) {
        memset(large_content, 'a', MAX_FILE_SIZE + 1);
    }

    // Test Case 1: Valid PDF file
    run_test("Test 1 (Valid)", "MyReport_2023.pdf", valid_content, valid_content_len);

    // Test Case 2: Path Traversal attempt
    run_test("Test 2 (Path Traversal)", "../../etc/passwd.pdf", valid_content, valid_content_len);

    // Test Case 3: Invalid file type
    run_test("Test 3 (Invalid Type)", "image.jpg", valid_content, valid_content_len);

    // Test Case 4: Filename with malicious/invalid characters
    run_test("Test 4 (Invalid Chars)", "a<b>c|d/e?f.pdf", valid_content, valid_content_len);

    // Test Case 5: File too large
    if (large_content) {
        run_test("Test 5 (Too Large)", "large_document.pdf", large_content, MAX_FILE_SIZE + 1);
        free(large_content);
    } else {
        printf("Test 5 (Too Large): Skipped (failed to allocate memory for large content).\n");
    }
    
    printf("----------------------------\n\n");
    return 0;
}