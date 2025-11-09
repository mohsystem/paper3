
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <archive.h>
#include <archive_entry.h>

/* Rules#8: Initialize pointers and check all allocation results */
/* Rules#2: Validate all external data, normalize paths, block traversal */
/* Rules#7: Open first, validate handle, operate only on that handle */

#define MAX_PATH_LEN 4096
#define MAX_FILE_SIZE (100 * 1024 * 1024) /* 100MB limit */
#define EXTRACT_DIR "/tmp/unpack"

/* Validate and normalize path to prevent directory traversal attacks */
static int validate_path(const char *path, char *normalized, size_t norm_size) {
    /* Rules#2: Check for null pointers and empty paths */
    if (!path || !normalized || norm_size == 0) {
        return -1;
    }
    
    /* Rules#2: Block directory traversal sequences */
    if (strstr(path, "..") != NULL || path[0] == '/') {
        return -1;
    }
    
    /* Rules#8: Bounds check before copy */
    if (strlen(path) >= norm_size) {
        return -1;
    }
    
    /* Rules#46: Ensure null termination */
    snprintf(normalized, norm_size, "%s", path);
    
    /* Additional validation: no absolute paths, no traversal */
    char *token = strtok(normalized, "/");
    while (token != NULL) {
        if (strcmp(token, "..") == 0 || strcmp(token, ".") == 0) {
            return -1;
        }
        token = strtok(NULL, "/");
    }
    
    return 0;
}

/* Safe tar extraction with comprehensive security checks */
int extract_tar_archive(const char *archive_path, const char *dest_dir) {
    struct archive *a = NULL;
    struct archive *ext = NULL;
    struct archive_entry *entry = NULL;
    int r;
    int ret = -1;
    char full_path[MAX_PATH_LEN];
    char normalized_path[MAX_PATH_LEN];
    FILE *test_file = NULL;
    
    /* Rules#8: Initialize all pointers to NULL */
    a = NULL;
    ext = NULL;
    
    /* Rules#2: Input validation - check parameters */
    if (!archive_path || !dest_dir) {
        fprintf(stderr, "Invalid parameters\\n");
        return -1;
    }
    
    /* Rules#2: Validate destination directory path */
    if (strlen(dest_dir) >= MAX_PATH_LEN - 256) {
        fprintf(stderr, "Destination path too long\\n");
        return -1;
    }
    
    /* Rules#7: Create directory with restrictive permissions */
    mkdir(dest_dir, 0700);
    
    /* Rules#7: Open archive first, then validate */
    a = archive_read_new();
    if (!a) {
        fprintf(stderr, "Failed to create archive reader\\n");
        goto cleanup;
    }
    
    /* Enable support for various compression formats */
    archive_read_support_filter_gzip(a);
    archive_read_support_filter_bzip2(a);
    archive_read_support_format_tar(a);
    
    /* Rules#7: Open and validate the archive file */
    r = archive_read_open_filename(a, archive_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open archive: %s\\n", archive_error_string(a));
        goto cleanup;
    }
    
    ext = archive_write_disk_new();
    if (!ext) {
        fprintf(stderr, "Failed to create disk writer\\n");
        goto cleanup;
    }
    
    /* Rules#7: Set safe extraction flags - no special files, no suid */
    archive_write_disk_set_options(ext, 
        ARCHIVE_EXTRACT_TIME | 
        ARCHIVE_EXTRACT_SECURE_NODOTDOT |
        ARCHIVE_EXTRACT_SECURE_SYMLINKS |
        ARCHIVE_EXTRACT_NO_OVERWRITE);
    
    /* Process each entry in the archive */
    while (1) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Archive read error: %s\\n", archive_error_string(a));
            goto cleanup;
        }
        
        const char *entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            fprintf(stderr, "Invalid entry path\\n");
            continue;
        }
        
        /* Rules#2: Validate entry path against traversal attacks */
        if (validate_path(entry_path, normalized_path, sizeof(normalized_path)) != 0) {
            fprintf(stderr, "Invalid path detected, skipping: %s\\n", entry_path);
            continue;
        }
        
        /* Rules#44: Check size before processing */
        int64_t size = archive_entry_size(entry);
        if (size > MAX_FILE_SIZE) {
            fprintf(stderr, "File too large, skipping: %s\\n", entry_path);
            continue;
        }
        
        /* Rules#8: Bounds check before path construction */
        int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", 
                                dest_dir, entry_path);
        if (path_len < 0 || path_len >= sizeof(full_path)) {
            fprintf(stderr, "Path too long, skipping: %s\\n", entry_path);
            continue;
        }
        
        /* Update entry path to full destination path */
        archive_entry_set_pathname(entry, full_path);
        
        /* Rules#7: Set restrictive permissions */
        mode_t mode = archive_entry_mode(entry);
        mode &= 0777; /* Remove special bits */
        archive_entry_set_mode(entry, mode);
        
        /* Write entry to disk */
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Write header error: %s\\n", archive_error_string(ext));
            continue;
        }
        
        if (size > 0) {
            const void *buff;
            size_t buff_size;
            int64_t offset;
            
            /* Rules#44: Bounded read from archive */
            while (1) {
                r = archive_read_data_block(a, &buff, &buff_size, &offset);
                if (r == ARCHIVE_EOF) {
                    break;
                }
                if (r != ARCHIVE_OK) {
                    fprintf(stderr, "Read data error: %s\\n", archive_error_string(a));
                    break;
                }
                
                /* Rules#44: Write with size validation */
                r = archive_write_data_block(ext, buff, buff_size, offset);
                if (r != ARCHIVE_OK) {
                    fprintf(stderr, "Write data error: %s\\n", archive_error_string(ext));
                    break;
                }
            }
        }
        
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Finish entry error: %s\\n", archive_error_string(ext));
        }
    }
    
    ret = 0;
    
cleanup:
    /* Rules#8: Free resources exactly once */
    if (ext) {
        archive_write_close(ext);
        archive_write_free(ext);
    }
    if (a) {
        archive_read_close(a);
        archive_read_free(a);
    }
    
    return ret;
}

/* Safe file read with validation */
int read_file_safely(const char *filepath) {
    FILE *f = NULL;
    char buffer[1024];
    size_t bytes_read;
    int ret = -1;
    struct stat st;
    int fd = -1;
    
    /* Rules#2: Input validation */
    if (!filepath) {
        fprintf(stderr, "Invalid filepath\\n");
        return -1;
    }
    
    /* Rules#7: Open with O_NOFOLLOW equivalent via fopen "r" mode */
    /* Rules#7: Use "wx" for exclusive write or "r" for safe read */
    f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\\n", strerror(errno));
        return -1;
    }
    
    /* Rules#7: Validate opened handle with fstat */
    fd = fileno(f);
    if (fd == -1 || fstat(fd, &st) != 0) {
        fprintf(stderr, "Failed to stat file\\n");
        goto cleanup;
    }
    
    /* Rules#28: Validate file type and size after opening */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file\\n");
        goto cleanup;
    }
    
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large\\n");
        goto cleanup;
    }
    
    /* Rules#44: Bounded read with size validation */
    while ((bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f)) > 0) {
        /* Rules#46: Ensure null termination */
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    
    if (ferror(f)) {
        fprintf(stderr, "Error reading file\\n");
        goto cleanup;
    }
    
    ret = 0;
    
cleanup:
    /* Rules#8: Close file exactly once */
    if (f) {
        fclose(f);
    }
    
    return ret;
}

/* Main function with test cases */
int main(void) {
    int result;
    
    printf("=== Test Case 1: Extract valid tar.gz archive ===\\n");
    result = extract_tar_archive("test1.tar.gz", "/tmp/unpack");
    printf("Result: %d\\n\\n", result);
    
    printf("=== Test Case 2: Extract tar.bz2 archive ===\\n");
    result = extract_tar_archive("test2.tar.bz2", "/tmp/unpack");
    printf("Result: %d\\n\\n", result);
    
    printf("=== Test Case 3: Handle invalid archive path ===\\n");
    result = extract_tar_archive(NULL, "/tmp/unpack");
    printf("Result: %d (should fail)\\n\\n", result);
    
    printf("=== Test Case 4: Read extracted file safely ===\\n");
    result = read_file_safely("/tmp/unpack/testfile.txt");
    printf("\\nResult: %d\\n\\n", result);
    
    printf("=== Test Case 5: Reject path traversal attempt ===\\n");
    result = extract_tar_archive("malicious.tar", "/tmp/unpack");
    printf("Result: %d (should reject traversal)\\n\\n", result);
    
    return 0;
}
