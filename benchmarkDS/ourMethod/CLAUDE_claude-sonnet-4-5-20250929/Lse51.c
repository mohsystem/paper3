
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <archive.h>
#include <archive_entry.h>
#include <limits.h>
#include <errno.h>

/* Maximum security limits to prevent resource exhaustion */
#define MAX_FILE_SIZE (100 * 1024 * 1024)  /* 100MB per file */
#define MAX_TOTAL_SIZE (500 * 1024 * 1024) /* 500MB total */
#define MAX_FILES 10000
#define BASE_DIR "/tmp/unpack"
#define BUFFER_SIZE 8192

/* Secure path validation preventing directory traversal */
int validate_path(const char* base, const char* path, char* resolved, size_t resolved_size) {
    /* Validate inputs are not NULL */
    if (!base || !path || !resolved || resolved_size == 0) {
        return 0;
    }
    
    /* Build full path safely with bounds checking */
    char full_path[PATH_MAX];
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", base, path);
    if (ret < 0 || ret >= (int)sizeof(full_path)) {
        return 0;
    }
    
    /* Resolve to canonical path (prevents .. and symlinks) */
    char* real = realpath(full_path, resolved);
    if (!real) {
        /* If file doesn't exist yet, validate parent directory */\n        char parent[PATH_MAX];\n        char* last_slash = strrchr(full_path, '/');\n        if (!last_slash) {\n            return 0;\n        }\n        \n        size_t parent_len = last_slash - full_path;\n        if (parent_len >= sizeof(parent)) {\n            return 0;\n        }\n        \n        memcpy(parent, full_path, parent_len);\n        parent[parent_len] = '\\0';\n        \n        char parent_real[PATH_MAX];\n        if (!realpath(parent, parent_real)) {\n            return 0;\n        }\n        \n        /* Build resolved path from parent */\n        ret = snprintf(resolved, resolved_size, "%s/%s", parent_real, last_slash + 1);\n        if (ret < 0 || ret >= (int)resolved_size) {\n            return 0;\n        }\n    }\n    \n    /* Validate resolved path starts with base directory */\n    char base_real[PATH_MAX];\n    if (!realpath(base, base_real)) {\n        return 0;\n    }\n    \n    size_t base_len = strlen(base_real);\n    if (strncmp(resolved, base_real, base_len) != 0) {\n        return 0;\n    }\n    \n    /* Ensure there's a path separator after base (or it's exactly the base) */\n    if (resolved[base_len] != '\\0' && resolved[base_len] != '/') {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Secure tar extraction function preventing path traversal and resource exhaustion */\nint extract_tar_archive(const char* tar_file_path, const char* compression_type) {\n    struct archive* a = NULL;\n    struct archive_entry* entry = NULL;\n    size_t total_bytes_extracted = 0;\n    int file_count = 0;\n    int result = -1;\n    \n    /* Validate inputs are not NULL */\n    if (!tar_file_path) {\n        fprintf(stderr, "Invalid tar file path\\n");\n        return -1;\n    }\n    \n    /* Create base directory with restrictive permissions (0700) */\n    struct stat st;\n    if (stat(BASE_DIR, &st) != 0) {\n        if (mkdir(BASE_DIR, 0700) != 0) {\n            fprintf(stderr, "Failed to create base directory\\n");\n            return -1;\n        }\n    }\n    \n    /* Initialize libarchive for reading */\n    a = archive_read_new();\n    if (!a) {\n        fprintf(stderr, "Failed to create archive reader\\n");\n        return -1;\n    }\n    \n    /* Enable only tar format (no automatic format detection for security) */\n    archive_read_support_format_tar(a);\n    \n    /* Set compression based on type */\n    if (compression_type && strcmp(compression_type, "gzip") == 0) {\n        archive_read_support_filter_gzip(a);\n    } else if (compression_type && strcmp(compression_type, "bzip2") == 0) {\n        archive_read_support_filter_bzip2(a);\n    } else {\n        archive_read_support_filter_none(a);\n    }\n    \n    /* Open archive with validation */\n    if (archive_read_open_filename(a, tar_file_path, 10240) != ARCHIVE_OK) {\n        fprintf(stderr, "Failed to open archive: %s\\n", archive_error_string(a));\n        goto cleanup;\n    }\n    \n    /* Process each entry in archive */\n    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {\n        /* Limit number of files to prevent zip bombs */\n        if (++file_count > MAX_FILES) {\n            fprintf(stderr, "Archive contains too many files\\n");\n            goto cleanup;\n        }\n        \n        /* Only process regular files */\n        mode_t filetype = archive_entry_filetype(entry);\n        if (!S_ISREG(filetype)) {\n            archive_read_data_skip(a);\n            continue;\n        }\n        \n        /* Validate entry size */\n        int64_t entry_size = archive_entry_size(entry);\n        if (entry_size < 0 || (size_t)entry_size > MAX_FILE_SIZE) {\n            fprintf(stderr, "Entry size exceeds maximum\\n");\n            goto cleanup;\n        }\n        \n        /* Check total size */\n        if (total_bytes_extracted + entry_size > MAX_TOTAL_SIZE) {\n            fprintf(stderr, "Total extraction size exceeds maximum\\n");\n            goto cleanup;\n        }\n        \n        /* Get and validate entry path */\n        const char* entry_name = archive_entry_pathname(entry);\n        if (!entry_name || entry_name[0] == '\\0') {\n            archive_read_data_skip(a);\n            continue;\n        }\n        \n        /* Remove leading slashes for security */\n        while (*entry_name == '/' || *entry_name == '\\\\') {\n            entry_name++;\n        }\n        \n        /* Validate path doesn't attempt traversal */
        char resolved_path[PATH_MAX];
        if (!validate_path(BASE_DIR, entry_name, resolved_path, sizeof(resolved_path))) {
            fprintf(stderr, "Path traversal attempt detected: %s\\n", entry_name);
            goto cleanup;
        }
        
        /* Open output file with restrictive permissions (0600) using open-then-validate */
        int fd = open(resolved_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
        if (fd < 0) {
            fprintf(stderr, "Failed to create file: %s\\n", resolved_path);
            goto cleanup;
        }
        
        /* Validate opened file descriptor */
        struct stat fd_stat;
        if (fstat(fd, &fd_stat) != 0 || !S_ISREG(fd_stat.st_mode)) {
            fprintf(stderr, "Opened descriptor is not a regular file\\n");
            close(fd);
            goto cleanup;
        }
        
        /* Extract data with size validation */
        size_t bytes_written = 0;
        char buffer[BUFFER_SIZE];
        
        ssize_t bytes_read;
        while ((bytes_read = archive_read_data(a, buffer, BUFFER_SIZE)) > 0) {
            /* Check for integer overflow */
            if (bytes_written > SIZE_MAX - bytes_read) {
                fprintf(stderr, "Integer overflow detected\\n");
                close(fd);
                goto cleanup;
            }
            
            /* Validate against declared size */
            if (bytes_written + bytes_read > (size_t)entry_size) {
                fprintf(stderr, "Entry size mismatch\\n");
                close(fd);
                goto cleanup;
            }
            
            ssize_t written = write(fd, buffer, bytes_read);
            if (written != bytes_read) {
                fprintf(stderr, "Write error\\n");
                close(fd);
                goto cleanup;
            }
            
            bytes_written += bytes_read;
        }
        
        /* Ensure data is written to disk */
        fsync(fd);
        close(fd);
        
        total_bytes_extracted += bytes_written;
    }
    
    result = 0;

cleanup:
    if (a) {
        archive_read_free(a);
    }
    return result;
}

/* Secure file reading with TOCTOU prevention */
char* read_file(const char* file_path) {
    int fd = -1;
    char* content = NULL;
    
    /* Validate input */
    if (!file_path || file_path[0] == '\\0') {
        fprintf(stderr, "Invalid file path\\n");
        return NULL;
    }
    
    /* Validate and resolve path */
    char resolved_path[PATH_MAX];
    if (!validate_path(BASE_DIR, file_path, resolved_path, sizeof(resolved_path))) {
        fprintf(stderr, "Path traversal attempt detected\\n");
        return NULL;
    }
    
    /* Open file first using open (TOCTOU prevention) */
    fd = open(resolved_path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Failed to open file\\n");
        return NULL;
    }
    
    /* Validate opened file descriptor */
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) != 0) {
        fprintf(stderr, "Failed to stat file\\n");
        close(fd);
        return NULL;
    }
    
    /* Ensure it's a regular file */\n    if (!S_ISREG(fd_stat.st_mode)) {\n        fprintf(stderr, "Not a regular file\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    /* Check file size */\n    if (fd_stat.st_size < 0 || (size_t)fd_stat.st_size > MAX_FILE_SIZE) {\n        fprintf(stderr, "File size exceeds maximum\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    /* Allocate buffer with space for null terminator */\n    size_t file_size = fd_stat.st_size;\n    content = (char*)malloc(file_size + 1);\n    if (!content) {\n        fprintf(stderr, "Memory allocation failed\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    /* Initialize buffer to ensure null termination */\n    memset(content, 0, file_size + 1);\n    \n    /* Read file content */\n    size_t total_read = 0;\n    while (total_read < file_size) {\n        ssize_t bytes_read = read(fd, content + total_read, file_size - total_read);\n        if (bytes_read < 0) {\n            fprintf(stderr, "Read error\\n");\n            free(content);\n            close(fd);\n            return NULL;\n        }\n        if (bytes_read == 0) {\n            break;\n        }\n        total_read += bytes_read;\n    }\n    \n    close(fd);\n    content[total_read] = '\\0';
    return content;
}

int main(void) {
    /* Test case 1: Extract uncompressed tar */
    printf("Test 1: Extract uncompressed tar\\n");
    /* extract_tar_archive("/path/to/archive.tar", NULL); */
    printf("Note: Requires actual tar file to test\\n");
    
    /* Test case 2: Extract gzip compressed tar */
    printf("\\nTest 2: Extract gzip tar\\n");
    /* extract_tar_archive("/path/to/archive.tar.gz", "gzip"); */
    printf("Note: Requires actual tar.gz file to test\\n");
    
    /* Test case 3: Extract bzip2 compressed tar */
    printf("\\nTest 3: Extract bzip2 tar\\n");
    /* extract_tar_archive("/path/to/archive.tar.bz2", "bzip2"); */
    printf("Note: Requires actual tar.bz2 file to test\\n");
    
    /* Test case 4: Read extracted file */
    printf("\\nTest 4: Read file\\n");
    /* char* content = read_file("example.txt"); */
    /* if (content) { printf("Content: %s\\n", content); free(content); } */
    printf("Note: Requires extracted file to test\\n");
    
    /* Test case 5: Path traversal prevention */
    printf("\\nTest 5: Path traversal prevention\\n");
    char* content = read_file("../../etc/passwd");
    if (content) {
        printf("FAIL: Should have returned NULL\\n");
        free(content);
    } else {
        printf("PASS: Path traversal blocked\\n");
    }
    
    return 0;
}
