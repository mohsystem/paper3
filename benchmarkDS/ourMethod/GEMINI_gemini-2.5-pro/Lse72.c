#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#define MKDIR(path, mode) mkdir(path, mode)
#endif


#define UPLOADS_DIR "uploads"
#define MAX_FILENAME_LEN 255
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

/**
 * @brief Validates a filename against security policies.
 *
 * It checks for:
 * - NULL or empty strings.
 * - Excessive length.
 * - Path traversal characters ('/' or '\').
 * - Directory self-references ('.' or '..').
 *
 * @param filename The filename to validate.
 * @return 1 if the filename is valid, 0 otherwise.
 */
static int is_valid_filename(const char *filename) {
    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Error: Filename is empty or NULL.\n");
        return 0;
    }

    if (strlen(filename) > MAX_FILENAME_LEN) {
        fprintf(stderr, "Error: Filename exceeds maximum length of %d.\n", MAX_FILENAME_LEN);
        return 0;
    }

    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        fprintf(stderr, "Error: Filename cannot be '.' or '..'.\n");
        return 0;
    }

    for (const char *p = filename; *p; p++) {
        if (*p == '/' || *p == '\\') {
            fprintf(stderr, "Error: Filename contains path separators.\n");
            return 0;
        }
    }
    
    return 1;
}

/**
 * @brief Creates the 'uploads' directory if it doesn't exist.
 *
 * This function attempts to create the directory. If the directory already
 * exists, it verifies that the existing path is indeed a directory.
 *
 * @return 0 on success (directory exists and is a directory), -1 on failure.
 */
static int create_uploads_dir() {
    if (MKDIR(UPLOADS_DIR, 0755) == 0) {
        printf("Directory '%s' created.\n", UPLOADS_DIR);
        return 0; // Successfully created
    }

    if (errno == EEXIST) {
        struct stat st;
        if (stat(UPLOADS_DIR, &st) != 0) {
            perror("stat");
            return -1;
        }
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: '%s' exists but is not a directory.\n", UPLOADS_DIR);
            return -1;
        }
        return 0; // Already exists and is a directory
    }

    perror("Failed to create directory");
    return -1; // Another error occurred
}

/**
 * @brief Handles the secure upload of a file.
 *
 * This function validates the filename, creates a secure destination directory,
 * and writes the provided content to a new file. It uses an atomic "create and open"
 * operation to prevent race conditions and overwriting existing files.
 *
 * @param filename The name of the file to be created. Must not contain path components.
 * @param file_content A pointer to the buffer containing the file's content.
 * @param content_length The size of the content in the buffer.
 * @return 0 on success, -1 on failure.
 */
int upload_file(const char *filename, const char *file_content, size_t content_length) {
    if (!is_valid_filename(filename)) {
        return -1;
    }

    if (content_length > 0 && file_content == NULL) {
        fprintf(stderr, "Error: File content is NULL but length is greater than 0.\n");
        return -1;
    }

    if (content_length > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size (%zu) exceeds the maximum limit of %d bytes.\n", content_length, MAX_FILE_SIZE);
        return -1;
    }

    if (create_uploads_dir() != 0) {
        return -1;
    }

    char full_path[PATH_MAX];
    int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", UPLOADS_DIR, filename);

    if (path_len < 0 || (size_t)path_len >= sizeof(full_path)) {
        fprintf(stderr, "Error: Constructed file path is too long.\n");
        return -1;
    }

    // "wbx" mode: write, binary, exclusive (fail if file exists).
    // This is atomic on POSIX/C11+ systems and prevents TOCTOU races.
    // Note: 'x' mode might not be supported on older C standards or compilers.
    // For MSVC, you might need to use different file opening functions.
    FILE *fp = fopen(full_path, "wbx");
    if (fp == NULL) {
        perror("Error opening file for writing");
        fprintf(stderr, "Note: File may already exist, or there might be a permission issue.\n");
        return -1;
    }

    int success = 1;
    if (content_length > 0) {
        if (fwrite(file_content, 1, content_length, fp) != content_length) {
            perror("Error writing to file");
            success = 0;
        }
    }

    if (fclose(fp) != 0) {
        perror("Error closing file");
        success = 0;
    }

    if (!success) {
        fprintf(stderr, "An error occurred during file write/close. Deleting partial file.\n");
        remove(full_path);
        return -1;
    }

    printf("File '%s' uploaded successfully.\n", full_path);
    return 0;
}

int main() {
    const char *content1 = "This is a test file.";
    const char *placeholder = "data";

    printf("--- Test Case 1: Valid upload ---\n");
    if (upload_file("test1.txt", content1, strlen(content1)) == 0) {
        printf("Test Case 1 Passed\n");
    } else {
        printf("Test Case 1 Failed\n");
    }
    printf("\n");

    printf("--- Test Case 2: Invalid filename (path traversal) ---\n");
    if (upload_file("../secret.txt", content1, strlen(content1)) != 0) {
        printf("Test Case 2 Passed (rejected as expected)\n");
    } else {
        printf("Test Case 2 Failed\n");
    }
    printf("\n");
    
    printf("--- Test Case 3: Invalid filename (absolute path) ---\n");
    if (upload_file("/etc/passwd", content1, strlen(content1)) != 0) {
        printf("Test Case 3 Passed (rejected as expected)\n");
    } else {
        printf("Test Case 3 Failed\n");
    }
    printf("\n");
    
    printf("--- Test Case 4: File content too large ---\n");
    if (upload_file("large_file.bin", placeholder, MAX_FILE_SIZE + 1) != 0) {
        printf("Test Case 4 Passed (rejected as expected)\n");
    } else {
        printf("Test Case 4 Failed\n");
    }
    printf("\n");
    
    printf("--- Test Case 5: Empty filename ---\n");
    if (upload_file("", content1, strlen(content1)) != 0) {
        printf("Test Case 5 Passed (rejected as expected)\n");
    } else {
        printf("Test Case 5 Failed\n");
    }
    printf("\n");

    printf("--- Test Case 6: Attempt to overwrite existing file ---\n");
    printf("First attempt (should succeed):\n");
    upload_file("test_overwrite.txt", "original", strlen("original"));
    printf("Second attempt (should fail):\n");
    if (upload_file("test_overwrite.txt", "new content", strlen("new content")) != 0) {
        printf("Test Case 6 Passed (overwrite rejected as expected)\n");
    } else {
        printf("Test Case 6 Failed\n");
    }
    printf("\n");

    // Clean up created files for subsequent runs
    remove("uploads/test1.txt");
    remove("uploads/test_overwrite.txt");
    
    return 0;
}