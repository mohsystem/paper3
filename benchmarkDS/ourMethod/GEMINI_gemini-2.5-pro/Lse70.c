#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

// For mkdir/stat and platform-specific definitions
#ifdef _WIN32
#include <direct.h>
// In MSVC, fopen "x" mode is available since Visual Studio 2015
#define MKDIR(path) _mkdir(path)
#define STAT_T _stat
#define STAT _stat
#define IS_DIR(mode) ((mode) & _S_IFDIR)
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0750)
#define STAT_T stat
#define STAT stat
#define IS_DIR(mode) S_ISDIR(mode)
#endif

// Configuration constants
#define UPLOADS_DIR "uploads"
#define MAX_FILENAME_LEN 255
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

// Function prototypes
bool is_filename_valid(const char *filename);
int ensure_uploads_dir_exists(const char *dir_path);
int upload_file(const char *filename, const void *content, size_t content_length);
void run_test_case(int test_num, const char *description, const char *filename, const void *content, size_t content_length);

/**
 * @brief Validates a filename against a strict allow-list.
 *
 * This function checks for:
 * - NULL or empty filename
 * - Excessive length
 * - Invalid characters (allows only alphanumeric, '.', '_', '-')
 * - Path traversal characters ('/' or '\' are blocked by the allow-list)
 * - Filenames that are special directories ('.' or '..')
 *
 * @param filename The filename to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool is_filename_valid(const char *filename) {
    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Error: Filename is NULL or empty.\n");
        return false;
    }

    size_t len = strlen(filename);
    if (len > MAX_FILENAME_LEN) {
        fprintf(stderr, "Error: Filename exceeds maximum length of %d.\n", MAX_FILENAME_LEN);
        return false;
    }

    // Use a strict allow-list for characters to prevent path traversal and other attacks.
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)filename[i]) && filename[i] != '.' && filename[i] != '_' && filename[i] != '-') {
            fprintf(stderr, "Error: Filename '%s' contains invalid characters.\n", filename);
            return false;
        }
    }
    
    // Explicitly disallow filenames that could be special directories.
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        fprintf(stderr, "Error: Filename cannot be '.' or '..'.\n");
        return false;
    }

    return true;
}


/**
 * @brief Ensures the upload directory exists.
 *
 * Tries to create the directory. If it already exists, it verifies that the
 * existing path is indeed a directory.
 *
 * @param dir_path The path to the directory.
 * @return 0 on success (or if directory already exists and is valid), -1 on failure.
 */
int ensure_uploads_dir_exists(const char *dir_path) {
    if (dir_path == NULL) {
        return -1;
    }
    
    if (MKDIR(dir_path) != 0) {
        if (errno != EEXIST) {
            perror("Error creating uploads directory");
            return -1;
        }
        // If it exists, verify it's a directory to prevent attacks
        // where a file named 'uploads' exists.
        struct STAT_T st;
        if (STAT(dir_path, &st) != 0) {
            perror("Error stating uploads path");
            return -1;
        }
        if (!IS_DIR(st.st_mode)) {
            fprintf(stderr, "Error: '%s' exists but is not a directory.\n", dir_path);
            return -1;
        }
    } else {
        printf("Directory '%s' created.\n", dir_path);
    }
    
    return 0;
}

/**
 * @brief Handles a file upload securely.
 *
 * This function validates the filename and content length, then attempts to save
 * the file to the predefined UPLOADS_DIR using an atomic creation operation
 * to prevent TOCTOU (Time-of-Check to Time-of-Use) race conditions.
 *
 * @param filename The name of the file to be saved.
 * @param content A pointer to the file content buffer.
 * @param content_length The size of the content buffer in bytes.
 * @return 0 on success, -1 on failure.
 */
int upload_file(const char *filename, const void *content, size_t content_length) {
    // 1. Validate all inputs before use.
    if (!is_filename_valid(filename)) {
        return -1;
    }

    if (content == NULL) {
        fprintf(stderr, "Error: File content is NULL.\n");
        return -1;
    }

    if (content_length > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size (%zu bytes) exceeds maximum allowed size (%d bytes).\n",
                content_length, MAX_FILE_SIZE);
        return -1;
    }

    // 2. Safely construct the destination path.
    char full_path[PATH_MAX];
    // Use snprintf to prevent buffer overflows.
    int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", UPLOADS_DIR, filename);

    if (path_len < 0 || (size_t)path_len >= sizeof(full_path)) {
        fprintf(stderr, "Error: Constructed file path is too long.\n");
        return -1;
    }

    // 3. Open the file using a race-condition-safe method.
    // "wbx" mode provides atomic, exclusive creation of a binary file.
    // This is a C11 feature and prevents overwriting existing files and TOCTOU attacks.
    FILE *fp = fopen(full_path, "wbx");
    if (fp == NULL) {
        if (errno == EEXIST) {
            fprintf(stderr, "Error: File '%s' already exists.\n", full_path);
        } else {
            perror("Error opening file for writing");
        }
        return -1;
    }
    
    // 4. Write content and handle errors.
    size_t bytes_written = 0;
    if (content_length > 0) {
        bytes_written = fwrite(content, 1, content_length, fp);
    }

    // 5. Close the file and check for errors during close.
    if (fclose(fp) != 0) {
        perror("Error closing file");
        // The file might be corrupted. Attempt to remove it.
        remove(full_path);
        return -1;
    }

    // Check for write errors after closing, as data might still be in buffers.
    if (bytes_written != content_length) {
        fprintf(stderr, "Error: Failed to write all content to file. Wrote %zu of %zu bytes.\n",
                bytes_written, content_length);
        // Clean up the partial file.
        remove(full_path);
        return -1;
    }

    printf("Successfully saved file to '%s'\n", full_path);
    return 0;
}

/**
 * @brief Main function with test cases.
 * 
 * Note: Compile with a C11 compliant compiler (e.g., gcc -std=c11) for "wbx" support.
 */
int main() {
    if (ensure_uploads_dir_exists(UPLOADS_DIR) != 0) {
        fprintf(stderr, "Fatal: Could not create or access uploads directory. Exiting.\n");
        return 1;
    }

    const char *content1 = "This is a test file.";
    const char *long_content = "This is another test file content that is slightly longer.";

    // Test Cases
    run_test_case(1, "Valid file upload", "test1.txt", content1, strlen(content1));
    run_test_case(2, "Path traversal attempt", "../etc/passwd", "hacker", strlen("hacker"));
    run_test_case(3, "Invalid characters in filename", "test<2>.txt", "badchars", strlen("badchars"));
    run_test_case(4, "Filename already exists", "test1.txt", long_content, strlen(long_content));
    run_test_case(5, "File size too large", "largefile.bin", "dummy", MAX_FILE_SIZE + 1);


    // Clean up created files for repeatable tests
    char cleanup_path[PATH_MAX];
    snprintf(cleanup_path, sizeof(cleanup_path), "%s/%s", UPLOADS_DIR, "test1.txt");
    remove(cleanup_path);

    return 0;
}

/**
 * @brief Helper function to run and print a test case.
 */
void run_test_case(int test_num, const char *description, const char *filename, const void *content, size_t content_length) {
    printf("\n--- Test Case %d: %s ---\n", test_num, description);
    printf("Filename: '%s', Content Length: %zu\n", filename, content_length);
    int result = upload_file(filename, content, content_length);
    if (result == 0) {
        printf("Result: SUCCESS\n");
    } else {
        printf("Result: FAILURE (as expected for negative tests)\n");
    }
    printf("--- End Test Case %d ---\n", test_num);
}