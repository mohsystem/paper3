/**
 * This C code demonstrates a secure way to read a file from a designated directory,
 * simulating a backend function for a route like `/img?file=...`.
 *
 * It emphasizes security by preventing path traversal, time-of-check-to-time-of-use
 * (TOCTOU) race conditions, and other common file-related vulnerabilities.
 *
 * This code is designed for POSIX-compliant systems (like Linux, macOS)
 * due to its use of functions like openat(), fstat(), and POSIX file flags.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

// POSIX-specific headers for secure file operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BASE_IMAGE_PATH "./images"
#define MAX_FILENAME_LEN 255
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB limit

// A struct to hold the file content and its size.
typedef struct {
    char *data;
    size_t size;
} FileContent;

/**
 * @brief Validates a filename to ensure it is safe to use.
 *
 * This function enforces several rules:
 * - The filename must not be NULL or empty.
 * - The length must be within a reasonable limit.
 * - It must not contain path traversal sequences ('..') or directory separators ('/', '\').
 * - It should not start with a '.' to avoid accessing hidden files.
 * - It should only contain characters from a predefined allow-list.
 *
 * @param filename The filename string to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool is_valid_filename(const char *filename) {
    if (filename == NULL || filename[0] == '\0') {
        return false;
    }

    size_t len = strnlen(filename, MAX_FILENAME_LEN + 1);
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return false;
    }
    
    // Rule #3: Normalize and canonicalize paths... before validation.
    // Here, we disallow path characters entirely, which is a stronger prevention.
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL || strstr(filename, "..") != NULL) {
        return false;
    }

    // Rule #2: Use allow lists over block lists.
    for (size_t i = 0; i < len; ++i) {
        char c = filename[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    
    if (filename[0] == '.') {
        return false;
    }

    return true;
}

/**
 * @brief Simulates serving an image file based on a filename parameter.
 *
 * This function securely reads a file from a predefined base directory. It serves as an
 * example of how to handle file access from untrusted user input.
 *
 * Security measures implemented:
 * - Rule #1, #5: All input is treated as untrusted and validated by is_valid_filename().
 * - Rule #39, #41: Path traversal is prevented by validating the filename and using `openat()`
 *   to anchor file access to a specific base directory.
 * - Rule #44, #45, #47: TOCTOU race conditions are avoided by opening the file first (`openat`)
 *   and then checking its properties (`fstat`) on the resulting file descriptor.
 * - Rule #48: Symlink following is disabled with the `O_NOFOLLOW` flag.
 * - Rule #18, #22, #25: File size is checked against a maximum limit, and reads are bounded.
 *
 * @param filename The name of the file to read.
 * @return A FileContent struct. On success, `data` points to a heap-allocated
 *         buffer with the file contents, and `size` is the file size. The caller
 *         is responsible for freeing `data`. On failure, `data` is NULL and `size` is 0.
 */
FileContent img(const char *filename) {
    FileContent result = { .data = NULL, .size = 0 };
    int dir_fd = -1;
    int file_fd = -1;
    char *buffer = NULL;

    if (!is_valid_filename(filename)) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return result;
    }

    dir_fd = open(BASE_IMAGE_PATH, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd == -1) {
        perror("Error opening base directory");
        return result;
    }

    file_fd = openat(dir_fd, filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (file_fd == -1) {
        // Suppress "No such file or directory" for non-existent files, which is normal.
        if (errno != ENOENT) {
            perror("Error opening file");
        }
        goto cleanup;
    }

    struct stat st;
    if (fstat(file_fd, &st) == -1) {
        perror("Error getting file stats");
        goto cleanup;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Path does not correspond to a regular file.\n");
        goto cleanup;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the maximum limit of %d MB.\n", MAX_FILE_SIZE / (1024 * 1024));
        goto cleanup;
    }

    if (st.st_size == 0) {
        result.size = 0;
        result.data = malloc(1); // Allocate 1 byte to return a valid, free-able pointer
        if (result.data) {
             result.data[0] = '\0';
        }
        goto cleanup;
    }
    
    // Rule #19: Check for potential overflow before allocation. st.st_size is off_t,
    // which might be larger than size_t, but MAX_FILE_SIZE provides a practical upper bound.
    buffer = malloc(st.st_size);
    if (buffer == NULL) {
        perror("Error allocating memory for file content");
        goto cleanup;
    }

    ssize_t bytes_read = read(file_fd, buffer, st.st_size);
    if (bytes_read < 0) {
        perror("Error reading file");
        free(buffer);
        goto cleanup;
    }

    if ((size_t)bytes_read != (size_t)st.st_size) {
        fprintf(stderr, "Error: Incomplete read of the file.\n");
        free(buffer);
        goto cleanup;
    }

    result.data = buffer;
    result.size = st.st_size;

cleanup:
    if (file_fd != -1) close(file_fd);
    if (dir_fd != -1) close(dir_fd);
    return result;
}

/**
 * @brief Sets up a test environment by creating a directory and some test files.
 */
void setup_test_environment() {
    mkdir(BASE_IMAGE_PATH, 0755);

    FILE *f = fopen(BASE_IMAGE_PATH "/test.png", "w");
    if (f) {
        fputs("PNG_DATA_HERE", f);
        fclose(f);
    }
    
    f = fopen("secret.txt", "w");
    if (f) {
        fputs("This is a secret.", f);
        fclose(f);
    }
}

/**
 * @brief Runs a single test case and prints the outcome.
 *
 * @param test_name A descriptive name for the test.
 * @param filename The filename to pass to the img() function.
 */
void run_test_case(const char *test_name, const char *filename) {
    printf("--- Running test case: %s ---\n", test_name);
    printf("Requesting file: \"%s\"\n", filename ? filename : "NULL");

    FileContent content = img(filename);

    if (content.data != NULL) {
        printf("Success! Read %zu bytes.\n", content.size);
        if (content.size > 0) {
            printf("Content preview: '%.*s...'\n", (int)(content.size > 10 ? 10 : content.size), content.data);
        }
        free(content.data);
    } else {
        printf("Failed as expected.\n");
    }
    printf("\n");
}

int main() {
    setup_test_environment();

    // Test case 1: A valid file request.
    run_test_case("Valid file", "test.png");

    // Test case 2: A file that does not exist.
    run_test_case("Non-existent file", "notfound.gif");

    // Test case 3: A path traversal attempt to access a file outside the base directory.
    run_test_case("Path traversal", "../secret.txt");

    // Test case 4: A filename containing a disallowed directory separator.
    run_test_case("Disallowed characters", "images/test.png");
    
    // Test case 5: An empty filename string.
    run_test_case("Empty filename", "");

    return 0;
}