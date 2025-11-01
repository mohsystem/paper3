#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// POSIX headers for file operations
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_FILE_SIZE (1024 * 1024) // 1 MB

/**
 * Reads a file from a safe directory using file descriptors to prevent TOCTOU
 * and path traversal vulnerabilities.
 *
 * @param safeDir The path to the trusted directory.
 * @param filename The name of the file to read.
 * @param out_size Pointer to a size_t to store the size of the returned buffer.
 * @return A dynamically allocated buffer with the file content, or NULL on failure.
 *         The caller is responsible for freeing this buffer.
 */
char* readFileFromSafeDirectory(const char* safeDir, const char* filename, size_t* out_size) {
    char* buffer = NULL;
    int dir_fd = -1;
    int fd = -1;
    
    if (out_size) {
        *out_size = 0;
    }

    // 1. Validate filename to ensure it is a simple name, not a path.
    if (filename == NULL || strchr(filename, '/') != NULL || strcmp(filename, "..") == 0 || strcmp(filename, ".") == 0) {
        fprintf(stderr, "Error: Filename must not contain path components.\n");
        return NULL;
    }
    
    // 2. Get a file descriptor for the safe directory.
    dir_fd = open(safeDir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd < 0) {
        fprintf(stderr, "Error: Could not open safe directory '%s': %s\n", safeDir, strerror(errno));
        return NULL;
    }

    // 3. Open the file relative to the directory descriptor.
    //    O_NOFOLLOW prevents following symbolic links.
    fd = openat(dir_fd, filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Error: Could not open file '%s': %s\n", filename, strerror(errno));
        goto cleanup;
    }
    
    // 4. Validate the opened file handle using fstat.
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: fstat failed for '%s': %s\n", filename, strerror(errno));
        goto cleanup;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        goto cleanup;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File '%s' exceeds size limit.\n", filename);
        goto cleanup;
    }
    if (st.st_size < 0) {
        fprintf(stderr, "Error: Invalid file size for '%s'.\n", filename);
        goto cleanup;
    }

    // 5. Allocate memory and read from the validated file descriptor.
    buffer = (char*)malloc(st.st_size > 0 ? st.st_size : 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        goto cleanup;
    }

    ssize_t bytes_read = read(fd, buffer, st.st_size);
    if (bytes_read < 0) {
        fprintf(stderr, "Error: read failed for '%s': %s\n", filename, strerror(errno));
        free(buffer);
        buffer = NULL;
        goto cleanup;
    }
    if (bytes_read != st.st_size) {
        fprintf(stderr, "Error: Incomplete read for '%s'.\n", filename);
        free(buffer);
        buffer = NULL;
        goto cleanup;
    }
    
    if (out_size) {
        *out_size = bytes_read;
    }

cleanup:
    if (fd >= 0) {
        close(fd);
    }
    if (dir_fd >= 0) {
        close(dir_fd);
    }
    return buffer;
}


void setup_test_environment(const char* dir, const char* secret) {
    mkdir(dir, 0755);
    char path[256];
    
    snprintf(path, sizeof(path), "%s/good.txt", dir);
    int fd = creat(path, 0644);
    if(fd != -1) { write(fd, "This is a safe file.", 20); close(fd); }
    
    fd = creat(secret, 0644);
    if(fd != -1) { write(fd, "This is a secret.", 17); close(fd); }

    snprintf(path, sizeof(path), "%s/large.txt", dir);
    fd = creat(path, 0644);
    if(fd != -1) { 
        char* large_content = (char*)malloc(MAX_FILE_SIZE + 1);
        if (large_content) {
            memset(large_content, 0, MAX_FILE_SIZE + 1);
            write(fd, large_content, MAX_FILE_SIZE + 1);
            free(large_content);
        }
        close(fd); 
    }

    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "%s/symlink.txt", dir);
    symlink(secret, symlink_path);
}

void cleanup_test_environment(const char* dir, const char* secret) {
    char path[256];
    snprintf(path, sizeof(path), "%s/good.txt", dir); unlink(path);
    snprintf(path, sizeof(path), "%s/large.txt", dir); unlink(path);
    snprintf(path, sizeof(path), "%s/symlink.txt", dir); unlink(path);
    unlink(secret);
    rmdir(dir);
}


int main() {
    const char* safeDirName = "c_safe_dir";
    const char* secretFileName = "c_secret_file.txt";

    setup_test_environment(safeDirName, secretFileName);
    
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Read a valid file
    printf("\n1. Reading a valid file (good.txt):\n");
    size_t content_size;
    char* content = readFileFromSafeDirectory(safeDirName, "good.txt", &content_size);
    if (content) {
        printf("   Success. Content: %.*s\n", (int)content_size, content);
        free(content);
    } else {
        printf("   Failed.\n");
    }

    // Test Case 2: Read a non-existent file
    printf("\n2. Reading a non-existent file (nonexistent.txt):\n");
    content = readFileFromSafeDirectory(safeDirName, "nonexistent.txt", &content_size);
    if (!content) {
        printf("   Success: Read operation failed as expected.\n");
    } else {
        printf("   Failed: Code read a non-existent file.\n");
        free(content);
    }

    // Test Case 3: Read a file that is too large
    printf("\n3. Reading a large file (large.txt):\n");
    content = readFileFromSafeDirectory(safeDirName, "large.txt", &content_size);
    if (!content) {
        printf("   Success: Read operation failed as expected.\n");
    } else {
        printf("   Failed: Code read an oversized file.\n");
        free(content);
    }

    // Test Case 4: Attempt path traversal
    char traversal_path[256];
    snprintf(traversal_path, sizeof(traversal_path), "../%s", secretFileName);
    printf("\n4. Attempting path traversal with a checked-for string (%s):\n", traversal_path);
    content = readFileFromSafeDirectory(safeDirName, traversal_path, &content_size);
    if (!content) {
        printf("   Success: Path traversal was blocked.\n");
    } else {
        printf("   Failed: Path traversal was successful.\n");
        free(content);
    }

    // Test Case 5: Attempt to read a symbolic link
    printf("\n5. Reading a symbolic link (symlink.txt):\n");
    content = readFileFromSafeDirectory(safeDirName, "symlink.txt", &content_size);
    if (!content) {
        printf("   Success: Reading symbolic link was blocked.\n");
    } else {
        printf("   Failed: Code followed a symbolic link.\n");
        free(content);
    }

    cleanup_test_environment(safeDirName, secretFileName);
    return 0;
}