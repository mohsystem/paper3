#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// For directory creation
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// Configuration for file uploads
const char* UPLOAD_DIRECTORY = "uploads";
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
const char* ALLOWED_EXTENSIONS[] = {".txt", ".pdf", ".jpg", ".png", NULL};

/**
 * Sanitizes a filename to prevent path traversal.
 * Modifies the buffer in place.
 * @param filename The buffer containing the original filename.
 */
void sanitize_filename(char* filename) {
    char* last_slash = strrchr(filename, '/');
    char* last_backslash = strrchr(filename, '\\');
    char* p = last_slash > last_backslash ? last_slash : last_backslash;
    
    if (p != NULL) {
        // Move the substring after the slash to the beginning of the buffer
        memmove(filename, p + 1, strlen(p));
    }
}

/**
 * Checks if a file extension is in the whitelist.
 * @param filename The filename to check.
 * @return true if the extension is allowed, false otherwise.
 */
bool is_extension_allowed(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return false; // No extension or hidden file

    // Create a lowercase version of the extension for case-insensitive comparison
    char lower_ext[32];
    int i = 0;
    for (i = 0; dot[i] != '\0' && i < 31; ++i) {
        lower_ext[i] = tolower((unsigned char)dot[i]);
    }
    lower_ext[i] = '\0';
    
    for (int j = 0; ALLOWED_EXTENSIONS[j] != NULL; ++j) {
        if (strcmp(lower_ext, ALLOWED_EXTENSIONS[j]) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Simulates securely saving an uploaded file.
 * @param original_filename The filename from the user.
 * @param file_content The byte content of the file.
 * @param content_size The size of the file content.
 * @return true if saved successfully, false otherwise.
 */
bool save_uploaded_file(const char* original_filename, const char* file_content, size_t content_size) {
    // 1. Security Check: File Size
    if (content_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the %zuMB limit.\n", MAX_FILE_SIZE / 1024 / 1024);
        return false;
    }

    // Create a mutable copy for sanitization
    char sanitized_filename[256];
    strncpy(sanitize_filename, original_filename, sizeof(sanitized_filename) - 1);
    sanitized_filename[sizeof(sanitized_filename) - 1] = '\0';

    // 2. Security Check: Path Traversal
    sanitize_filename(sanitized_filename);
    if (strlen(sanitized_filename) == 0 || strcmp(sanitized_filename, ".") == 0 || strcmp(sanitized_filename, "..") == 0) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return false;
    }
    
    // 3. Security Check: File Extension Whitelist
    if (!is_extension_allowed(sanitized_filename)) {
        fprintf(stderr, "Error: File type is not allowed.\n");
        return false;
    }

    // 4. Construct the final safe path
    MKDIR(UPLOAD_DIRECTORY); // Create directory, ignore error if it exists
    char destination_path[512];
    snprintf(destination_path, sizeof(destination_path), "%s/%s", UPLOAD_DIRECTORY, sanitized_filename);

    // Prevent overwriting existing files
    FILE* f_check = fopen(destination_path, "rb");
    if (f_check) {
        fprintf(stderr, "Error: A file with this name already exists.\n");
        fclose(f_check);
        return false;
    }

    // 5. Write the file to the secure location
    FILE* outfile = fopen(destination_path, "wb");
    if (!outfile) {
        fprintf(stderr, "Error: Could not open file for writing.\n");
        return false;
    }
    size_t written = fwrite(file_content, 1, content_size, outfile);
    fclose(outfile);

    if (written != content_size) {
        fprintf(stderr, "Error: Failed to write complete file content.\n");
        remove(destination_path); // Clean up partial file
        return false;
    }
    
    printf("Success: File '%s' uploaded to '%s' directory.\n", sanitized_filename, UPLOAD_DIRECTORY);
    return true;
}

void run_tests() {
    printf("--- Running C File Upload Tests ---\n");
    const char* small_content = "This is a test file content.";
    size_t small_size = strlen(small_content);
    
    char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
    if(large_content) memset(large_content, 'A', MAX_FILE_SIZE + 1);

    // Test Case 1: Valid file upload
    printf("\n[Test Case 1: Valid file]\n");
    save_uploaded_file("image.png", small_content, small_size);

    // Test Case 2: Invalid file extension
    printf("\n[Test Case 2: Invalid extension]\n");
    save_uploaded_file("program.c", small_content, small_size);

    // Test Case 3: Path Traversal attempt
    printf("\n[Test Case 3: Path Traversal]\n");
    save_uploaded_file("/etc/shadow", small_content, small_size);
    
    // Test Case 4: File too large
    printf("\n[Test Case 4: File too large]\n");
    if (large_content) {
        save_uploaded_file("big.txt", large_content, MAX_FILE_SIZE + 1);
    } else {
        fprintf(stderr, "Could not allocate memory for large file test.\n");
    }

    // Test Case 5: No extension
    printf("\n[Test Case 5: No extension]\n");
    save_uploaded_file("my-special-file", small_content, small_size);

    // Cleanup
    free(large_content);
    char file_to_remove[512];
    snprintf(file_to_remove, sizeof(file_to_remove), "%s/%s", UPLOAD_DIRECTORY, "image.png");
    remove(file_to_remove);
}

int main() {
    run_tests();
    return 0;
}