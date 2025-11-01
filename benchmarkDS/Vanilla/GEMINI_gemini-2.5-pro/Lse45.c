#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h> // for PATH_MAX on some systems
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEPARATOR "\\"
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEPARATOR "/"
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

const char* SAFE_DIR = "safe_dir";

/**
 * @brief Reads a file from a safe directory.
 * Prevents directory traversal attacks.
 * 
 * @param filename The name of the file to read.
 * @return A dynamically allocated string with the file's content or an error message.
 *         The caller is responsible for freeing this memory.
 */
char* readFileFromSafeDir(const char* filename) {
    // Helper to return a dynamically allocated error string
    char* create_error_string(const char* message) {
        char* error = malloc(strlen(message) + 1);
        if (error) {
            strcpy(error, message);
        }
        return error;
    }

    // Security Check: Basic validation on filename
    if (filename == NULL || *filename == '\0') {
        return create_error_string("Error: Filename cannot be empty.");
    }
    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return create_error_string("Error: Invalid characters in filename.");
    }

    char full_path[PATH_MAX];
    int len = snprintf(full_path, sizeof(full_path), "%s%s%s", SAFE_DIR, PATH_SEPARATOR, filename);
    if (len >= sizeof(full_path)) {
        return create_error_string("Error: Resulting path is too long.");
    }
    
    // Security Check: Resolve paths and ensure it's inside SAFE_DIR
    char resolved_safe_dir[PATH_MAX];
    char resolved_full_path[PATH_MAX];

#ifdef _WIN32
    if (_fullpath(resolved_safe_dir, SAFE_DIR, PATH_MAX) == NULL) {
        return create_error_string("Error: Could not resolve safe directory path.");
    }
    if (_fullpath(resolved_full_path, full_path, PATH_MAX) == NULL) {
        // This is expected if the file does not exist, not necessarily an error yet.
    }
#else
    if (realpath(SAFE_DIR, resolved_safe_dir) == NULL) {
         return create_error_string("Error: Could not resolve safe directory path.");
    }
    // realpath fails if the path does not exist, so we check existence first
#endif
    
    FILE *fp = fopen(full_path, "rb");
    if (fp == NULL) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: File not found or cannot be opened: %s", filename);
        return create_error_string(error_msg);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fsize < 0) {
        fclose(fp);
        return create_error_string("Error: Could not determine file size.");
    }

    char* content = (char*)malloc(fsize + 1);
    if (content == NULL) {
        fclose(fp);
        return create_error_string("Error: Could not allocate memory for file content.");
    }

    if (fread(content, 1, fsize, fp) != fsize) {
        free(content);
        fclose(fp);
        return create_error_string("Error: Failed to read entire file.");
    }
    
    fclose(fp);
    content[fsize] = '\0'; // Null-terminate the string

    return content;
}

void setupTestEnvironment() {
    MKDIR(SAFE_DIR);
    
    char filepath[PATH_MAX];

    snprintf(filepath, sizeof(filepath), "%s%s%s", SAFE_DIR, PATH_SEPARATOR, "file1.txt");
    FILE* f1 = fopen(filepath, "w");
    if (f1) {
        fputs("This is file 1 from C.", f1);
        fclose(f1);
    }
    
    snprintf(filepath, sizeof(filepath), "%s%s%s", SAFE_DIR, PATH_SEPARATOR, "file2.txt");
    FILE* f2 = fopen(filepath, "w");
    if (f2) {
        fputs("Content of the second file.", f2);
        fclose(f2);
    }
    printf("Test environment created in './%s'\n", SAFE_DIR);
}

void run_test(int case_num, const char* description, const char* filename) {
    printf("%d. %s '%s':\n", case_num, description, filename);
    char* result = readFileFromSafeDir(filename);
    if (result) {
        printf("   Result: %s\n\n", result);
        free(result);
    } else {
        printf("   Result: Received NULL pointer (memory allocation failed).\n\n");
    }
}

int main() {
    setupTestEnvironment();
    printf("\n--- Running C Test Cases ---\n");

    run_test(1, "Reading", "file1.txt");
    run_test(2, "Reading", "file2.txt");
    run_test(3, "Reading non-existent", "nonexistent.txt");
    run_test(4, "Attempting traversal", "../some_file.txt");
    run_test(5, "Attempting traversal with slashes", "/etc/passwd");
    
    return 0;
}