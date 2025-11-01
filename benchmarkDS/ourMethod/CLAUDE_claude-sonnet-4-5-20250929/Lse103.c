
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

/* Secure file open with O_CREAT|O_WRONLY|O_TRUNC flags */
/* Returns file descriptor or -1 on error */
/* Security: validates paths, prevents traversal, uses restrictive permissions */
int open_file_create(const char* base_dir, const char* filename) {
    /* Validate inputs - reject NULL or empty */
    if (base_dir == NULL || filename == NULL || base_dir[0] == '\\0' || filename[0] == '\\0') {
        fprintf(stderr, "Error: Base directory and filename must not be NULL or empty\\n");
        return -1;
    }
    
    /* Security: reject if filename contains path separators */
    if (strchr(filename, '/') != NULL) {
        fprintf(stderr, "Error: Filename must not contain path separators\\n");
        return -1;
    }
    
    /* Security: check filename length to prevent buffer overflow */
    if (strlen(filename) > 255) {
        fprintf(stderr, "Error: Filename too long\\n");
        return -1;
    }
    
    /* Construct full path with bounds checking */
    char full_path[PATH_MAX];
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, filename);
    if (ret < 0 || ret >= (int)sizeof(full_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        return -1;
    }
    
    /* Normalize path and check it's within base directory */\n    char resolved_base[PATH_MAX];\n    char resolved_path[PATH_MAX];\n    \n    if (realpath(base_dir, resolved_base) == NULL) {\n        fprintf(stderr, "Error: Cannot resolve base directory\\n");\n        return -1;\n    }\n    \n    /* O_CREAT|O_WRONLY|O_TRUNC|O_NOFOLLOW|O_CLOEXEC */\n    /* Security: use restrictive permissions 0600 at creation time */\n    /* O_NOFOLLOW prevents following symlinks */\n    int flags = O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC;\n    \n#ifdef O_NOFOLLOW\n    flags |= O_NOFOLLOW;\n#endif\n    \n    /* Open with restrictive permissions (0600 - owner read/write only) */\n    int fd = open(full_path, flags, S_IRUSR | S_IWUSR);\n    if (fd < 0) {\n        fprintf(stderr, "Error: Failed to open file: %s\\n", strerror(errno));\n        return -1;\n    }\n    \n    /* Validate the opened file descriptor with fstat (TOCTOU-safe) */\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Error: Failed to stat file descriptor\\n");\n        close(fd);\n        return -1;\n    }\n    \n    /* Security: ensure it's a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return -1;
    }
    
    /* Verify path is within base directory after opening */
    if (realpath(full_path, resolved_path) != NULL) {
        size_t base_len = strlen(resolved_base);
        if (strncmp(resolved_path, resolved_base, base_len) != 0 ||
            (resolved_path[base_len] != '/' && resolved_path[base_len] != '\\0')) {
            fprintf(stderr, "Error: Path traversal attempt detected\\n");
            close(fd);
            unlink(full_path); /* Remove the created file */
            return -1;
        }
    }
    
    return fd;
}

/* Secure file open with O_CREAT|O_WRONLY|O_EXCL flags */
/* Returns file descriptor, fails if file already exists */
int open_file_create_exclusive(const char* base_dir, const char* filename) {
    /* Validate inputs */
    if (base_dir == NULL || filename == NULL || base_dir[0] == '\\0' || filename[0] == '\\0') {
        fprintf(stderr, "Error: Base directory and filename must not be NULL or empty\\n");
        return -1;
    }
    
    /* Security: reject if filename contains path separators */
    if (strchr(filename, '/') != NULL) {
        fprintf(stderr, "Error: Filename must not contain path separators\\n");
        return -1;
    }
    
    /* Security: check filename length */
    if (strlen(filename) > 255) {
        fprintf(stderr, "Error: Filename too long\\n");
        return -1;
    }
    
    /* Construct full path with bounds checking */
    char full_path[PATH_MAX];
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, filename);
    if (ret < 0 || ret >= (int)sizeof(full_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        return -1;
    }
    
    /* Normalize and validate base directory */
    char resolved_base[PATH_MAX];
    if (realpath(base_dir, resolved_base) == NULL) {
        fprintf(stderr, "Error: Cannot resolve base directory\\n");
        return -1;
    }
    
    /* O_CREAT|O_WRONLY|O_EXCL|O_NOFOLLOW|O_CLOEXEC */
    /* Security: O_EXCL fails if file exists, O_NOFOLLOW prevents symlink following */
    int flags = O_CREAT | O_WRONLY | O_EXCL | O_CLOEXEC;
    
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    
    /* Open with exclusive creation and restrictive permissions */
    int fd = open(full_path, flags, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to create file (may already exist): %s\\n", strerror(errno));
        return -1;
    }
    
    /* Validate the opened file descriptor */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file descriptor\\n");
        close(fd);
        return -1;
    }
    
    /* Security: ensure it's a regular file */\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Not a regular file\\n");\n        close(fd);\n        unlink(full_path);\n        return -1;\n    }\n    \n    /* Verify path is within base directory */\n    char resolved_path[PATH_MAX];\n    if (realpath(full_path, resolved_path) != NULL) {\n        size_t base_len = strlen(resolved_base);\n        if (strncmp(resolved_path, resolved_base, base_len) != 0 ||\n            (resolved_path[base_len] != '/' && resolved_path[base_len] != '\\0')) {
            fprintf(stderr, "Error: Path traversal attempt detected\\n");
            close(fd);
            unlink(full_path);
            return -1;
        }
    }
    
    return fd;
}

int main(void) {
    const char* test_dir = "/tmp";
    
    /* Test case 1: Create or truncate file */
    printf("Test 1: Create/truncate file\\n");
    int fd1 = open_file_create(test_dir, "test1.txt");
    if (fd1 >= 0) {
        write(fd1, "Test data\\n", 10);
        close(fd1);
        printf("Success: File created/truncated\\n");
    }
    
    /* Test case 2: Create exclusive file (should succeed first time) */
    printf("\\nTest 2: Create exclusive file\\n");
    int fd2 = open_file_create_exclusive(test_dir, "test2.txt");
    if (fd2 >= 0) {
        write(fd2, "Exclusive data\\n", 15);
        close(fd2);
        printf("Success: Exclusive file created\\n");
    }
    
    /* Test case 3: Create exclusive file again (should fail) */
    printf("\\nTest 3: Create exclusive file that exists (should fail)\\n");
    int fd3 = open_file_create_exclusive(test_dir, "test2.txt");
    if (fd3 >= 0) {
        close(fd3);
        printf("Success: Exclusive file created\\n");
    } else {
        printf("Expected error occurred\\n");
    }
    
    /* Test case 4: Path traversal attempt (should fail) */
    printf("\\nTest 4: Path traversal attempt (should fail)\\n");
    int fd4 = open_file_create(test_dir, "../etc/passwd");
    if (fd4 >= 0) {
        close(fd4);
        printf("Success: File created\\n");
    } else {
        printf("Expected error occurred\\n");
    }
    
    /* Test case 5: Invalid input (should fail) */
    printf("\\nTest 5: Invalid empty filename (should fail)\\n");
    int fd5 = open_file_create(test_dir, "");
    if (fd5 >= 0) {
        close(fd5);
        printf("Success: File created\\n");
    } else {
        printf("Expected error occurred\\n");
    }
    
    /* Cleanup */
    unlink("/tmp/test1.txt");
    unlink("/tmp/test2.txt");
    
    return 0;
}
