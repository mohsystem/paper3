#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h> // For mkdir on POSIX
#include <errno.h>

#ifdef _WIN32
#include <direct.h> // For _mkdir on Windows
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755) // 0755 permissions: rwxr-xr-x
#endif

// Define a safe directory for uploads.
#define UPLOAD_DIRECTORY "uploads_c"
// Define a maximum file size to prevent denial-of-service attacks (e.g., 5MB).
#define MAX_FILE_SIZE (5 * 1024 * 1024)
// Define a maximum path length for buffer safety
#define MAX_PATH_LEN 4096

// Forward declaration
char* uploadFile(const char* originalFileName, const char* fileContent, size_t contentLength);

// In C, a class structure doesn't exist, so we use a plain main function.
typedef struct {
    void (*main)();
} Task50;

/**
 * Sanitizes a filename to prevent path traversal.
 * It removes directory separators.
 * Returns a newly allocated string that must be freed by the caller.
 */
char* sanitize_filename(const char* filename) {
    const char* last_slash = strrchr(filename, '/');
    const char* last_backslash = strrchr(filename, '\\');
    
    const char* start = filename;
    if (last_slash && last_slash >= start) {
        start = last_slash + 1;
    }
    if (last_backslash && last_backslash >= start) {
        start = last_backslash + 1;
    }
    
    char* sanitized = strdup(start);
    return sanitized;
}

/**
 * Creates a directory if it does not exist.
 * Returns 0 on success or if directory already exists, -1 on error.
 */
int create_directory_if_not_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (MKDIR(path) != 0 && errno != EEXIST) {
            return -1;
        }
    }
    return 0;
}

/**
 * Simulates uploading a file to the server.
 *
 * @param originalFileName The name of the file provided by the user.
 * @param fileContent      The binary content of the file.
 * @param contentLength    The length of the file content.
 * @return A newly allocated string with a confirmation or error message.
 *         The caller is responsible for freeing this memory.
 */
char* uploadFile(const char* originalFileName, const char* fileContent, size_t contentLength) {
    char* result_message = (char*)malloc(MAX_PATH_LEN);
    if (!result_message) return strdup("Error: Memory allocation failed.");

    // Security: Check for null or empty inputs
    if (originalFileName == NULL || originalFileName[0] == '\0') {
        snprintf(result_message, MAX_PATH_LEN, "Error: File name cannot be empty.");
        return result_message;
    }
    if (fileContent == NULL || contentLength == 0) {
        snprintf(result_message, MAX_PATH_LEN, "Error: File content cannot be empty.");
        return result_message;
    }

    // Security: Check file size
    if (contentLength > MAX_FILE_SIZE) {
        snprintf(result_message, MAX_PATH_LEN, "Error: File size exceeds the %zuMB limit.", (size_t)(MAX_FILE_SIZE / (1024*1024)));
        return result_message;
    }

    // Security: Prevent path traversal by sanitizing the filename
    char* sanitizedFileName = sanitize_filename(originalFileName);
    if (!sanitizedFileName || sanitizedFileName[0] == '\0') {
        free(sanitizedFileName);
        snprintf(result_message, MAX_PATH_LEN, "Error: Invalid file name provided.");
        return result_message;
    }

    // Security: Generate a unique filename to prevent overwrites
    char uniqueFileName[MAX_PATH_LEN];
    snprintf(uniqueFileName, sizeof(uniqueFileName), "%ld_%s", (long)time(NULL), sanitizedFileName);

    // Create the upload directory if it doesn't exist
    if (create_directory_if_not_exists(UPLOAD_DIRECTORY) != 0) {
        free(sanitizedFileName);
        snprintf(result_message, MAX_PATH_LEN, "Error: Could not create upload directory.");
        return result_message;
    }

    // Construct the final, safe destination path
    char destinationPath[MAX_PATH_LEN];
    int len = snprintf(destinationPath, sizeof(destinationPath), "%s/%s", UPLOAD_DIRECTORY, uniqueFileName);
    if (len >= sizeof(destinationPath)) {
        free(sanitizedFileName);
        snprintf(result_message, MAX_PATH_LEN, "Error: Destination path is too long.");
        return result_message;
    }

    FILE* fp = fopen(destinationPath, "wb");
    if (fp == NULL) {
        free(sanitizedFileName);
        snprintf(result_message, MAX_PATH_LEN, "Error: Could not open file for writing.");
        return result_message;
    }

    size_t written = fwrite(fileContent, 1, contentLength, fp);
    fclose(fp); // Always close the file

    if (written != contentLength) {
        free(sanitizedFileName);
        snprintf(result_message, MAX_PATH_LEN, "Error: Failed to write all content to file.");
        return result_message;
    }

    snprintf(result_message, MAX_PATH_LEN, "Success: File '%s' uploaded as '%s'.", originalFileName, uniqueFileName);
    free(sanitizedFileName);
    return result_message;
}

void run_tests() {
    printf("--- Running C Test Cases ---\n");
    char* response;

    // Test Case 1: Normal successful upload
    const char* content1 = "This is a test file.";
    response = uploadFile("report.txt", content1, strlen(content1));
    printf("Test 1 (Normal): %s\n", response);
    free(response);

    // Sleep for a second to get a new timestamp for the unique name
    #ifdef _WIN32
        _sleep(1000);
    #else
        sleep(1);
    #endif

    // Test Case 2: Uploading a file with the same name (should not overwrite)
    const char* content2 = "This is another test file.";
    response = uploadFile("report.txt", content2, strlen(content2));
    printf("Test 2 (Duplicate Name): %s\n", response);
    free(response);

    // Test Case 3: Path traversal attack attempt
    const char* content3 = "malicious content";
    response = uploadFile("../../etc/passwd", content3, strlen(content3));
    printf("Test 3 (Path Traversal): %s\n", response);
    free(response);

    // Test Case 4: File size too large
    char* largeContent = (char*)malloc(MAX_FILE_SIZE + 1);
    if (largeContent) {
        memset(largeContent, 'A', MAX_FILE_SIZE + 1);
        response = uploadFile("large_file.zip", largeContent, MAX_FILE_SIZE + 1);
        printf("Test 4 (Oversized File): %s\n", response);
        free(response);
        free(largeContent);
    } else {
        printf("Test 4 (Oversized File): Skipped (could not allocate memory).\n");
    }

    // Test Case 5: File with spaces and special characters in name
    const char* content5 = "Final document content.";
    response = uploadFile("My Report (Final Version).docx", content5, strlen(content5));
    printf("Test 5 (Special Chars): %s\n", response);
    free(response);

    printf("--------------------------\n");
}

int main() {
    Task50 task;
    task.main = run_tests;
    task.main();
    return 0;
}