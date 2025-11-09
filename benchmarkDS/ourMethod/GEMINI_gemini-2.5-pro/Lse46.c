#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#define REALPATH(path, resolved) _fullpath(resolved, path, _MAX_PATH)
#else
#include <errno.h>
#define MKDIR(path, mode) mkdir(path, mode)
#define REALPATH(path, resolved) realpath(path, resolved)
#endif


// --- Constants ---
#define IMAGE_DIR "images"
#define MAX_FILENAME_LEN 255
#define READ_BUFFER_SIZE 4096

/**
 * @brief Validates a filename against a strict allow-list of characters.
 *
 * Rules#1, #2, #5: This function validates the input filename. It checks for
 * length, emptiness, and enforces a strict character set (alphanumeric, '.', '_', '-')
 * to prevent injection of path traversal sequences or other malicious characters.
 * It acts as the first line of defense at the trust boundary.
 *
 * @param filename The filename to validate.
 * @return true if the filename is valid, false otherwise.
 */
static bool is_valid_filename(const char *filename) {
    if (!filename || filename[0] == '\0') {
        return false;
    }

    if (strlen(filename) > MAX_FILENAME_LEN) {
        return false;
    }

    // Disallow special directory names explicitly.
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return false;
    }

    // Allow-list validation for filename characters.
    for (size_t i = 0; filename[i] != '\0'; i++) {
        unsigned char c = filename[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return false;
        }
    }

    return true;
}

/**
 * @brief Simulates serving an image file based on a 'name' parameter.
 *
 * This function models a web handler for a URL like /img?name=<filename>.
 * It securely constructs the path, validates it to prevent directory traversal,
 * and sends the file content to standard output.
 *
 * @param name The filename provided as a URL parameter.
 */
void serve_image(const char *name) {
    // 1. Validate input filename (Rules #1, #2, #5, #54)
    if (!is_valid_filename(name)) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        // In a real server, send an HTTP 400 Bad Request response.
        return;
    }

    // 2. Safely construct the full file path (Rules #18, #60)
    char full_path[PATH_MAX];
    int written = snprintf(full_path, sizeof(full_path), "%s/%s", IMAGE_DIR, name);

    // Check for truncation or encoding errors from snprintf.
    if (written < 0 || (size_t)written >= sizeof(full_path)) {
        fprintf(stderr, "Error: Path construction failed due to length.\n");
        // In a real server, send an HTTP 500 Internal Server Error response.
        return;
    }

    // 3. Normalize paths and verify the result is within the base directory (Rules #3, #39, #41)
    char canonical_base_path[PATH_MAX];
    if (REALPATH(IMAGE_DIR, canonical_base_path) == NULL) {
        fprintf(stderr, "Error: Base image directory '%s' is inaccessible.\n", IMAGE_DIR);
        return;
    }

    char canonical_file_path[PATH_MAX];
    if (REALPATH(full_path, canonical_file_path) == NULL) {
        // This is the expected path for a file that doesn't exist.
        fprintf(stderr, "Error: File not found.\n");
        // In a real server, send an HTTP 404 Not Found response.
        return;
    }

    // Check that the canonical file path starts with the canonical base path.
    size_t base_len = strlen(canonical_base_path);
    if (strncmp(canonical_file_path, canonical_base_path, base_len) != 0 ||
        (canonical_file_path[base_len] != '/' && canonical_file_path[base_len] != '\\' && canonical_file_path[base_len] != '\0')) {
        fprintf(stderr, "Error: Directory traversal attempt detected.\n");
        // In a real server, send an HTTP 403 Forbidden response.
        return;
    }

    // 4. Open file and validate handle to prevent TOCTOU races (Rules #45, #47, #59, #64)
    FILE *file = fopen(canonical_file_path, "rb");
    if (!file) {
        // This might be redundant due to realpath, but serves as defense-in-depth.
        fprintf(stderr, "Error: Could not open file after validation.\n");
        return;
    }

    // Use the file handle to verify file type, avoiding check-then-use on the path.
    int fd = fileno(file);
    struct stat file_stat;
    if (fstat(fd, &file_stat) != 0) {
        fprintf(stderr, "Error: Could not get file stats.\n");
        fclose(file);
        return;
    }

    // Ensure we are only serving regular files.
    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "Error: Requested path is not a regular file.\n");
        fclose(file);
        return;
    }

    // 5. Read file in chunks and send to output (simulates response) (Rules #18, #25)
    printf("--- Simulating HTTP Response ---\n");
    printf("Content-Type: application/octet-stream\n");
    printf("Content-Length: %ld\n\n", (long)file_stat.st_size);

    char buffer[READ_BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        // In a real server, this would write to a socket. We write to stdout.
        fwrite(buffer, 1, bytes_read, stdout);
    }

    // Check for read errors after the loop.
    if (ferror(file)) {
        fprintf(stderr, "\nError: An error occurred while reading the file.\n");
    }

    // 6. Cleanup
    fclose(file);
    printf("\n--- End of Response ---\n");
}


/**
 * @brief Sets up a temporary directory and files for testing.
 */
static void setup_test_environment() {
    // Create the image directory.
    if (MKDIR(IMAGE_DIR, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create 'images' directory");
        exit(EXIT_FAILURE);
    }
    
    // Create a valid image file.
    FILE *f1 = fopen("images/cat.jpg", "w");
    if (f1) {
        fprintf(f1, "This is the content of cat.jpg.");
        fclose(f1);
    } else {
        perror("Failed to create test file 'images/cat.jpg'");
    }
    
    // Create a file outside the allowed directory for traversal tests.
    FILE *f2 = fopen("secret.txt", "w");
     if (f2) {
        fprintf(f2, "This is a secret file.");
        fclose(f2);
    } else {
        perror("Failed to create test file 'secret.txt'");
    }
}

/**
 * @brief Cleans up the files and directory created for testing.
 */
static void cleanup_test_environment() {
    remove("images/cat.jpg");
    remove("secret.txt");
    rmdir(IMAGE_DIR);
}


int main() {
    setup_test_environment();

    printf(">>> Test Case 1: Valid image request\n");
    serve_image("cat.jpg");
    printf("\n\n");

    printf(">>> Test Case 2: Non-existent file request\n");
    serve_image("dog.png");
    printf("\n\n");

    printf(">>> Test Case 3: Path traversal attempt with '..'\n");
    serve_image("../secret.txt"); // Blocked by is_valid_filename()
    printf("\n\n");
    
    printf(">>> Test Case 4: Invalid characters in filename\n");
    serve_image("file<>&.txt"); // Blocked by is_valid_filename()
    printf("\n\n");

    printf(">>> Test Case 5: Requesting a directory instead of a file\n");
    serve_image("."); // Blocked by is_valid_filename()
    printf("\n\n");
    
    cleanup_test_environment();
    return 0;
}