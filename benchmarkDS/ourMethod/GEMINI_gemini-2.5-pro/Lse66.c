#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define UPLOADS_DIR "uploads"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_FILENAME_LEN 255
#define PDF_MAGIC_BYTES "%PDF-"
#define PDF_MAGIC_LEN 5

// Helper function to sanitize a filename.
// It removes path components and allows only a whitelist of characters.
// Returns true on success, false on failure.
static bool sanitize_filename(const char* input, char* output, size_t out_size) {
    if (!input || !output || out_size == 0) return false;

    // Find the last path separator to get the basename.
    const char* last_slash = strrchr(input, '/');
    const char* last_bslash = strrchr(input, '\\');
    const char* basename = input;
    if (last_slash && last_slash > basename) basename = last_slash + 1;
    if (last_bslash && last_bslash > basename) basename = last_bslash + 1;
    
    if (strlen(basename) == 0 || strlen(basename) >= out_size) {
        fprintf(stderr, "Error: Filename is empty or too long after stripping path.\n");
        return false;
    }

    // Copy and validate characters against a whitelist.
    size_t i = 0;
    for (i = 0; basename[i] != '\0' && i < out_size - 1; ++i) {
        char c = basename[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            fprintf(stderr, "Error: Filename contains invalid character '%c'.\n", c);
            return false;
        }
        output[i] = c;
    }
    output[i] = '\0';

    // Check for null bytes in the original basename.
    if (memchr(basename, '\0', strlen(basename)) != basename + strlen(basename)) {
        fprintf(stderr, "Error: Filename contains embedded null bytes.\n");
        return false;
    }

    // Check for ".pdf" extension (case-insensitive)
    const char* ext = strrchr(output, '.');
    if (!ext || (strcasecmp(ext, ".pdf") != 0)) {
        fprintf(stderr, "Error: Filename must have a .pdf extension.\n");
        return false;
    }

    return true;
}


/**
 * Accepts a PDF file's content and saves it securely to an 'uploads' folder.
 *
 * @param file_name The name of the file provided by the user.
 * @param file_content The byte content of the file.
 * @param content_length The length of the file content.
 * @return 0 on success, -1 on failure.
 */
int save_pdf(const char* file_name, const char* file_content, size_t content_length) {
    int dir_fd = -1;
    int temp_fd = -1;
    char temp_path[PATH_MAX] = {0};
    char final_path_sanitized[MAX_FILENAME_LEN] = {0};
    int ret = -1;

    // 1. Input validation
    if (!file_name || !file_content || content_length == 0) {
        fprintf(stderr, "Error: File name or content is empty.\n");
        return -1;
    }

    if (content_length > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the maximum limit.\n");
        return -1;
    }

    if (content_length < PDF_MAGIC_LEN || memcmp(file_content, PDF_MAGIC_BYTES, PDF_MAGIC_LEN) != 0) {
        fprintf(stderr, "Error: File is not a valid PDF.\n");
        return -1;
    }

    // 2. Sanitize filename
    if (!sanitize_filename(file_name, final_path_sanitized, sizeof(final_path_sanitized))) {
        return -1; // Error message printed in function
    }
    
    // 3. Create uploads directory if it doesn't exist
    if (mkdir(UPLOADS_DIR, 0750) != 0 && errno != EEXIST) {
        perror("Error creating uploads directory");
        return -1;
    }

    // 4. Secure write: use openat/renameat for TOCTOU-safe operations
    dir_fd = open(UPLOADS_DIR, O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        perror("Error opening uploads directory");
        return -1;
    }

    // Generate a temporary filename
    snprintf(temp_path, sizeof(temp_path), "%s.tmp.%ld", final_path_sanitized, (long)getpid());

    // Create and open the temp file exclusively and without following symlinks.
    // Use restrictive permissions (owner read/write).
    temp_fd = openat(dir_fd, temp_path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (temp_fd == -1) {
        perror("Error creating temporary file");
        goto cleanup;
    }
    
    // Write content to the temporary file
    ssize_t bytes_written = write(temp_fd, file_content, content_length);
    if (bytes_written < 0 || (size_t)bytes_written != content_length) {
        perror("Error writing to temporary file");
        // Attempt to remove temp file on write failure
        unlinkat(dir_fd, temp_path, 0);
        goto cleanup;
    }

    // Ensure data is written to disk
    if (fsync(temp_fd) != 0) {
        perror("Error fsyncing temporary file");
        unlinkat(dir_fd, temp_path, 0);
        goto cleanup;
    }

    // Atomically rename the temp file to its final name
    if (renameat(dir_fd, temp_path, dir_fd, final_path_sanitized) != 0) {
        perror("Error renaming temporary file");
        unlinkat(dir_fd, temp_path, 0);
        goto cleanup;
    }

    // Sync the directory to ensure rename is persisted
    if (fsync(dir_fd) != 0) {
        perror("Error fsyncing directory");
        // This is not a fatal error for the file itself, but log it.
    }
    
    printf("Successfully saved file: %s/%s\n", UPLOADS_DIR, final_path_sanitized);
    ret = 0; // Success

cleanup:
    if (temp_fd != -1) close(temp_fd);
    if (dir_fd != -1) close(dir_fd);
    return ret;
}

int main() {
    printf("Running test cases...\n");

    // Test Case 1: Valid PDF file
    printf("\n--- Test Case 1: Valid PDF ---\n");
    const char* valid_content = "%PDF-1.4\n... a valid pdf content ...";
    save_pdf("report.pdf", valid_content, strlen(valid_content));

    // Test Case 2: Path traversal attempt
    printf("\n--- Test Case 2: Path Traversal ---\n");
    save_pdf("../../../etc/passwd.pdf", valid_content, strlen(valid_content));

    // Test Case 3: Non-PDF file
    printf("\n--- Test Case 3: Non-PDF File ---\n");
    const char* text_content = "This is just a text file.";
    save_pdf("not_a_pdf.txt", text_content, strlen(text_content));

    // Test Case 4: File too large
    printf("\n--- Test Case 4: File Too Large ---\n");
    char* large_content = malloc(MAX_FILE_SIZE + 1);
    if (large_content) {
        memset(large_content, 'a', MAX_FILE_SIZE + 1);
        memcpy(large_content, valid_content, strlen(valid_content));
        save_pdf("large_file.pdf", large_content, MAX_FILE_SIZE + 1);
        free(large_content);
    }

    // Test Case 5: Invalid filename with null byte
    printf("\n--- Test Case 5: Invalid Filename with Null Byte ---\n");
    char bad_name[] = "bad\0name.pdf";
    save_pdf(bad_name, valid_content, strlen(valid_content));
    
    return 0;
}