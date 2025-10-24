#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>

// Note: This code uses POSIX-specific functions (realpath, access, stat, readlink, etc.).
// For Windows, a different implementation using the Windows API would be required.

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define BUFFER_SIZE 4096

const char* ALLOWED_EXTENSIONS[] = {".txt", ".jpg", ".png", ".pdf", NULL};

// Helper function to check for allowed extensions (case-insensitive)
int is_extension_allowed(const char* filename) {
    if (!filename) return 0;
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0; // No extension found

    char ext_lower[32];
    int i = 0;
    for (const char* p = dot; *p && i < sizeof(ext_lower) - 1; ++p, ++i) {
        ext_lower[i] = (*p >= 'A' && *p <= 'Z') ? (*p - 'A' + 'a') : *p;
    }
    ext_lower[i] = '\0';
    
    for (i = 0; ALLOWED_EXTENSIONS[i] != NULL; ++i) {
        if (strcmp(ext_lower, ALLOWED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int secureFileUpload(const char* sourcePath, const char* destDir) {
    struct stat stat_buf;

    // 1. Source File Checks
    if (stat(sourcePath, &stat_buf) != 0 || !S_ISREG(stat_buf.st_mode) || access(sourcePath, R_OK) != 0) {
        fprintf(stderr, "Error: Source file is not a valid, readable file.\n");
        return 0;
    }

    // 2. Destination Directory Checks
    if (stat(destDir, &stat_buf) != 0 || !S_ISDIR(stat_buf.st_mode) || access(destDir, W_OK) != 0) {
        fprintf(stderr, "Error: Destination is not a valid, writable directory.\n");
        return 0;
    }

    // 3. File Size Check
    if (stat(sourcePath, &stat_buf) == 0 && stat_buf.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds limit of %lld MB.\n", (long long)MAX_FILE_SIZE / (1024 * 1024));
        return 0;
    }

    // 4. Filename and Extension Validation
    char* sourcePathCopy = strdup(sourcePath);
    if (!sourcePathCopy) return 0;
    char* base_name = basename(sourcePathCopy);

    if (strchr(base_name, '/') != NULL || strchr(base_name, '\\') != NULL) {
        fprintf(stderr, "Error: Filename contains invalid path characters.\n");
        free(sourcePathCopy);
        return 0;
    }
    
    if (!is_extension_allowed(base_name)) {
        fprintf(stderr, "Error: File extension is not allowed for '%s'.\n", base_name);
        free(sourcePathCopy);
        return 0;
    }

    // 5. Path Traversal Check
    char canonical_dest_dir[PATH_MAX];
    if (realpath(destDir, canonical_dest_dir) == NULL) {
        fprintf(stderr, "Error: Could not resolve real path for destination directory.\n");
        free(sourcePathCopy);
        return 0;
    }
    
    char dest_file_path[PATH_MAX];
    snprintf(dest_file_path, sizeof(dest_file_path), "%s/%s", canonical_dest_dir, base_name);

    char final_path_check[PATH_MAX];
    if (realpath(dest_file_path, final_path_check) != NULL) {
        // File exists, realpath works, check if it's inside the dir
        if(strncmp(final_path_check, canonical_dest_dir, strlen(canonical_dest_dir)) != 0) {
             fprintf(stderr, "Error: Path traversal attempt detected (on existing file).\n");
             free(sourcePathCopy);
             return 0;
        }
    } else {
        // File doesn't exist, check our constructed path
        if (strncmp(dest_file_path, canonical_dest_dir, strlen(canonical_dest_dir)) != 0) {
            fprintf(stderr, "Error: Path traversal attempt detected.\n");
            free(sourcePathCopy);
            return 0;
        }
    }

    // 6. Check for existing file (to prevent overwrite)
    if (access(dest_file_path, F_OK) == 0) {
        fprintf(stderr, "Error: File '%s' already exists in the destination.\n", base_name);
        free(sourcePathCopy);
        return 0;
    }
    
    // 7. Perform the copy
    FILE *source_file = fopen(sourcePath, "rb");
    FILE *dest_file = fopen(dest_file_path, "wb");
    if (!source_file || !dest_file) {
        perror("File open failed");
        if (source_file) fclose(source_file);
        if (dest_file) fclose(dest_file);
        free(sourcePathCopy);
        return 0;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int success = 1;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source_file)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            fprintf(stderr, "Error writing to destination file.\n");
            success = 0;
            break;
        }
    }

    fclose(source_file);
    fclose(dest_file);
    
    if (success) {
        printf("Success: File '%s' uploaded successfully.\n", base_name);
    } else {
        remove(dest_file_path); // Clean up partial file on failure
    }
    
    free(sourcePathCopy);
    return success;
}

void create_test_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

void create_large_test_file(const char* path, long long size) {
    FILE* f = fopen(path, "wb");
    if (f) {
        char buffer[1024] = {0};
        for (long long i = 0; i < size / sizeof(buffer); ++i) {
            fwrite(buffer, 1, sizeof(buffer), f);
        }
        fclose(f);
    }
}

int main() {
    const char* uploadDir = "uploads_c";
    mkdir(uploadDir, 0755);

    create_test_file("test_good.txt", "This is a good file.");
    create_large_test_file("test_large.bin", MAX_FILE_SIZE + 1);
    create_test_file("test_bad.sh", "#!/bin/bash\necho pwned");
    create_test_file("traversal_test.txt", "This file should not escape the upload dir");

    printf("--- Running Test Cases ---\n");

    printf("\n[Test 1] Successful Upload:\n");
    secureFileUpload("test_good.txt", uploadDir);

    printf("\n[Test 2] File Exists:\n");
    secureFileUpload("test_good.txt", uploadDir);

    printf("\n[Test 3] File Too Large:\n");
    secureFileUpload("test_large.bin", uploadDir);

    printf("\n[Test 4] Disallowed Extension:\n");
    secureFileUpload("test_bad.sh", uploadDir);
    
    printf("\n[Test 5] Path Traversal Attempt:\n");
    char malicious_path[50];
    snprintf(malicious_path, sizeof(malicious_path), "%s/../", uploadDir);
    secureFileUpload("traversal_test.txt", malicious_path);

    // Cleanup
    remove("test_good.txt");
    remove("test_large.bin");
    remove("test_bad.sh");
    remove("traversal_test.txt");
    return 0;
}