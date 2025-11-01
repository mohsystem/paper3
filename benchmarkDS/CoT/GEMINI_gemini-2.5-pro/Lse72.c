#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

// For creating directories
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEPARATOR '\\'
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEPARATOR '/'
#endif


/**
 * Sanitizes a filename by removing directory paths, returning only the base name.
 * This is a crucial security measure to prevent path traversal attacks.
 * Note: The caller is responsible for freeing the returned memory.
 * Returns NULL on error.
 */
char* sanitize_filename(const char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    
    // Find the last occurrence of a path separator
    const char* last_slash = strrchr(filename, '/');
    const char* last_backslash = strrchr(filename, '\\');

    const char* basename_start = filename;
    if (last_slash && last_backslash) {
        basename_start = (last_slash > last_backslash ? last_slash : last_backslash) + 1;
    } else if (last_slash) {
        basename_start = last_slash + 1;
    } else if (last_backslash) {
        basename_start = last_backslash + 1;
    }

    // Check for invalid names like "." or ".."
    if (strcmp(basename_start, ".") == 0 || strcmp(basename_start, "..") == 0 || strlen(basename_start) == 0) {
        return NULL;
    }

    char* sanitized = strdup(basename_start);
    if (sanitized == NULL) {
        perror("strdup failed");
    }
    return sanitized;
}


/**
 * Handles the upload of a file by saving content to a specified filename
 * in an 'uploads' directory. It creates the directory if it doesn't exist
 * and sanitizes the filename to prevent path traversal attacks.
 *
 * @param filename The desired name of the file. Path information will be stripped.
 * @param content The string content to write to the file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool upload_file(const char* filename, const char* content) {
    if (filename == NULL || content == NULL) {
        fprintf(stderr, "Error: Filename or content is NULL.\n");
        return false;
    }

    // --- Security: Path Traversal Prevention ---
    char* sanitized_filename = sanitize_filename(filename);
    if (sanitized_filename == NULL) {
        fprintf(stderr, "Error: Invalid or malicious filename provided: %s\n", filename);
        return false;
    }

    const char* upload_dir = "uploads";

    // Create the directory if it doesn't exist
    if (MKDIR(upload_dir) != 0 && errno != EEXIST) {
        perror("Error creating directory");
        free(sanitized_filename);
        return false;
    }
    
    // Construct destination path safely to prevent buffer overflows
    // path_len = strlen("uploads") + strlen("/") + strlen(sanitized_name) + null_terminator
    size_t path_len = strlen(upload_dir) + 1 + strlen(sanitized_filename) + 1;
    char* destination_path = (char*)malloc(path_len);
    if (destination_path == NULL) {
        perror("Failed to allocate memory for path");
        free(sanitized_filename);
        return false;
    }

    snprintf(destination_path, path_len, "%s%c%s", upload_dir, PATH_SEPARATOR, sanitized_filename);

    // Write the content to the file
    FILE *fp = fopen(destination_path, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        free(sanitized_filename);
        free(destination_path);
        return false;
    }

    if (fputs(content, fp) == EOF) {
        fprintf(stderr, "Error writing to file %s\n", destination_path);
        fclose(fp);
        free(sanitized_filename);
        free(destination_path);
        return false;
    }
    
    printf("Successfully saved file: %s\n", destination_path);

    fclose(fp);
    free(sanitized_filename);
    free(destination_path);
    return true;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Valid simple filename
    upload_file("test1.txt", "This is a valid test file.");

    // Test Case 2: Another valid filename
    upload_file("config.ini", "[settings]\nuser=test");

    // Test Case 3: Path traversal attempt (should fail)
    upload_file("../.profile", "export HACKED=1");

    // Test Case 4: Absolute path attempt (should be sanitized)
    upload_file("/home/user/document.txt", "This should be saved in ./uploads/");

    // Test Case 5: Empty filename (should fail)
    upload_file("", "This content should not be saved.");

    printf("--------------------\n\n");
    return 0;
}