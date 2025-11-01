#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

// Platform-specific directory creation
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755)
#endif

// Define security constants
#define UPLOAD_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define PDF_MAGIC_BYTES "%PDF-"
#define PDF_MAGIC_BYTES_LEN 5
#define MAX_FILENAME_LEN 255
#define MAX_PATH_LEN 1024

/**
 * Securely saves a PDF file to the 'uploads' directory.
 *
 * @param original_filename The original filename from the user.
 * @param file_content      The byte content of the file.
 * @param content_length    The length of the file content.
 * @return 0 on success, -1 on failure.
 */
int save_pdf(const char* original_filename, const char* file_content, size_t content_length) {
    // 1. Check for invalid inputs
    if (original_filename == NULL || file_content == NULL || content_length == 0) {
        fprintf(stderr, "Error: Invalid filename or empty file content.\n");
        return -1;
    }

    // 2. Security: Check file size
    if (content_length > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the maximum limit of %d bytes.\n", MAX_FILE_SIZE);
        return -1;
    }

    // 3. Security: Validate file type by checking magic bytes
    if (content_length < PDF_MAGIC_BYTES_LEN || memcmp(file_content, PDF_MAGIC_BYTES, PDF_MAGIC_BYTES_LEN) != 0) {
        fprintf(stderr, "Error: File is not a valid PDF.\n");
        return -1;
    }

    // 4. Security: Prevent path traversal by extracting the base filename
    const char *base_filename = strrchr(original_filename, '/');
    const char *win_base_filename = strrchr(original_filename, '\\');
    if (win_base_filename > base_filename) {
        base_filename = win_base_filename;
    }
    
    if (base_filename == NULL) {
        base_filename = original_filename;
    } else {
        base_filename++; // Move past the separator
    }

    if (strlen(base_filename) == 0) {
        fprintf(stderr, "Error: Filename is empty after path removal.\n");
        return -1;
    }

    // 5. Security: Sanitize the filename using a whitelist
    char sanitized_filename[MAX_FILENAME_LEN] = {0};
    int j = 0;
    for (int i = 0; base_filename[i] != '\0' && j < MAX_FILENAME_LEN - 1; i++) {
        if (isalnum((unsigned char)base_filename[i]) || base_filename[i] == '.' || base_filename[i] == '_' || base_filename[i] == '-') {
            sanitized_filename[j++] = base_filename[i];
        } else {
            sanitized_filename[j++] = '_';
        }
    }
    sanitized_filename[j] = '\0'; // Null-terminate

    // Ensure directory exists
    if (MKDIR(UPLOAD_DIR) != 0 && errno != EEXIST) {
        perror("Error creating uploads directory");
        return -1;
    }
    
    // Construct the final, secure destination path
    char destination_path[MAX_PATH_LEN];
    int written = snprintf(destination_path, sizeof(destination_path), "%s/%s", UPLOAD_DIR, sanitized_filename);
    if (written < 0 || written >= sizeof(destination_path)) {
        fprintf(stderr, "Error: Destination path is too long.\n");
        return -1;
    }

    // Write the file
    FILE *fp = fopen(destination_path, "wb");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -1;
    }

    size_t written_bytes = fwrite(file_content, 1, content_length, fp);
    fclose(fp);

    if (written_bytes != content_length) {
        fprintf(stderr, "Error: Failed to write the entire file.\n");
        // Attempt to remove the partial file
        remove(destination_path);
        return -1;
    }
    
    printf("Successfully saved file to: %s\n", destination_path);
    return 0;
}

void run_test(const char* test_name, const char* filename, const char* content, size_t content_len) {
    printf("\n[Test %s]\n", test_name);
    save_pdf(filename, content, content_len);
}

int main() {
    printf("--- Running C Test Cases ---\n");

    const char* valid_pdf_content = "%PDF-1.7... some pdf content...";
    size_t valid_len = strlen(valid_pdf_content);

    // Test Case 1: Valid PDF file
    run_test("1] Valid PDF file", "my_document.pdf", valid_pdf_content, valid_len);

    // Test Case 2: Path traversal attempt
    run_test("2] Path traversal attempt", "../../../boot.ini", valid_pdf_content, valid_len);

    // Test Case 3: Invalid file type (not a PDF)
    const char* non_pdf_content = "some other file format";
    run_test("3] Invalid file type", "image.jpg", non_pdf_content, strlen(non_pdf_content));

    // Test Case 4: Filename with special characters
    run_test("4] Filename with special characters", "my file<name>|is*bad.pdf", valid_pdf_content, valid_len);

    // Test Case 5: File too large
    char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
    if (large_content) {
        memcpy(large_content, PDF_MAGIC_BYTES, PDF_MAGIC_BYTES_LEN);
        // Fill the rest with some data
        memset(large_content + PDF_MAGIC_BYTES_LEN, 'A', MAX_FILE_SIZE + 1 - PDF_MAGIC_BYTES_LEN);
        run_test("5] File too large", "oversized.pdf", large_content, MAX_FILE_SIZE + 1);
        free(large_content);
    } else {
        printf("\n[Test 5] Skipped: Could not allocate memory for large file test.\n");
    }

    printf("\n--- C Test Cases Finished ---\n");
    return 0;
}