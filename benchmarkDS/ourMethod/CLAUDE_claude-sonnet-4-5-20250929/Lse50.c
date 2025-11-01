
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <archive.h>
#include <archive_entry.h>

/* Maximum allowed file size: 100 MB to prevent zip bombs */
#define MAX_FILE_SIZE (100 * 1024 * 1024)
/* Maximum number of files to prevent excessive resource usage */
#define MAX_FILES 10000
/* Base directory for extraction */
#define BASE_DIR "/tmp/unpack"

/* Helper function to create directories recursively with error checking */
int create_directories(const char* path) {
    char tmp[PATH_MAX];
    char* p = NULL;
    size_t len;
    
    if (!path || strlen(path) == 0) return 0;
    
    /* Copy path safely with bounds checking */
    len = strlen(path);
    if (len >= sizeof(tmp)) return 0;
    memcpy(tmp, path, len);
    tmp[len] = '\\0';
    
    /* Remove trailing slash if present */
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\\0';
    }
    
    /* Create directories iteratively */
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\\0';
            if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
                return 0;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
        return 0;
    }
    
    return 1;
}

/* Helper function to normalize and validate paths */
int is_path_safe(const char* base_path, const char* entry_path, char* resolved_path, size_t resolved_size) {
    char full_path[PATH_MAX];
    char resolved_base[PATH_MAX];
    char resolved_parent[PATH_MAX];
    char parent_path[PATH_MAX];
    char* last_slash;
    size_t parent_len;
    size_t base_len;
    int ret;
    
    if (!base_path || !entry_path || !resolved_path) return 0;
    
    /* Build full path with bounds checking */
    ret = snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry_path);
    if (ret < 0 || (size_t)ret >= sizeof(full_path)) {
        return 0;
    }
    
    /* Resolve to absolute path */
    if (realpath(full_path, resolved_path) == NULL) {
        /* If file doesn't exist yet, resolve parent and append filename */\n        last_slash = strrchr(full_path, '/');\n        if (!last_slash) return 0;\n        \n        parent_len = last_slash - full_path;\n        if (parent_len >= sizeof(parent_path)) return 0;\n        \n        memcpy(parent_path, full_path, parent_len);\n        parent_path[parent_len] = '\\0';\n        \n        if (realpath(parent_path, resolved_parent) == NULL) {\n            /* Create parent directory if it doesn't exist */
            if (!create_directories(parent_path)) return 0;
            if (realpath(parent_path, resolved_parent) == NULL) return 0;
        }
        
        ret = snprintf(resolved_path, resolved_size, "%s%s", resolved_parent, last_slash);
        if (ret < 0 || (size_t)ret >= resolved_size) return 0;
    }
    
    /* Check if resolved path starts with base directory (prevent directory traversal) */
    if (realpath(base_path, resolved_base) == NULL) {
        return 0;
    }
    
    base_len = strlen(resolved_base);
    if (strncmp(resolved_path, resolved_base, base_len) != 0) {
        return 0;
    }
    
    return 1;
}

int extract_zip_archive(const char* zip_file_path) {
    struct archive* a = NULL;
    struct archive_entry* entry = NULL;
    char resolved_path[PATH_MAX];
    const char* entry_name;
    int r;
    int file_count = 0;
    long total_size = 0;
    la_int64_t size;
    int fd;
    const void* buff;
    size_t buff_size;
    la_int64_t offset;
    ssize_t written;
    
    /* Validate input */
    if (!zip_file_path || strlen(zip_file_path) == 0) {
        fprintf(stderr, "Zip file path cannot be null or empty\\n");
        return 0;
    }
    
    /* Create base directory */
    if (!create_directories(BASE_DIR)) {
        fprintf(stderr, "Failed to create base directory\\n");
        return 0;
    }
    
    /* Initialize libarchive for reading */
    a = archive_read_new();
    if (!a) {
        fprintf(stderr, "Failed to create archive reader\\n");
        return 0;
    }
    
    /* Support zip format */
    archive_read_support_format_zip(a);
    archive_read_support_filter_all(a);
    
    /* Open the archive file */
    r = archive_read_open_filename(a, zip_file_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open archive: %s\\n", archive_error_string(a));
        archive_read_free(a);
        return 0;
    }
    
    /* Extract all entries */
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        file_count++;
        
        /* Prevent zip bomb by limiting file count */
        if (file_count > MAX_FILES) {
            fprintf(stderr, "Too many files in archive\\n");
            archive_read_free(a);
            return 0;
        }
        
        entry_name = archive_entry_pathname(entry);
        if (!entry_name || strlen(entry_name) == 0) {
            archive_read_data_skip(a);
            continue;
        }
        
        /* Validate and resolve path */
        if (!is_path_safe(BASE_DIR, entry_name, resolved_path, sizeof(resolved_path))) {
            fprintf(stderr, "Entry is outside target directory: %s\\n", entry_name);
            archive_read_free(a);
            return 0;
        }
        
        /* Get file size */
        size = archive_entry_size(entry);
        if (size > MAX_FILE_SIZE) {
            fprintf(stderr, "File too large: %s\\n", entry_name);
            archive_read_free(a);
            return 0;
        }
        
        total_size += size;
        if (total_size > MAX_FILE_SIZE * 10) {
            fprintf(stderr, "Total extracted size exceeds limit\\n");
            archive_read_free(a);
            return 0;
        }
        
        /* Handle directories */
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            mkdir(resolved_path, 0700);
        } else {
            /* Extract regular file */
            fd = open(resolved_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
            if (fd < 0) {
                fprintf(stderr, "Failed to create file: %s\\n", resolved_path);
                archive_read_free(a);
                return 0;
            }
            
            while (1) {
                r = archive_read_data_block(a, &buff, &buff_size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r != ARCHIVE_OK) {
                    fprintf(stderr, "Read error: %s\\n", archive_error_string(a));
                    close(fd);
                    archive_read_free(a);
                    return 0;
                }
                
                written = write(fd, buff, buff_size);
                if (written != (ssize_t)buff_size) {
                    fprintf(stderr, "Write error\\n");
                    close(fd);
                    archive_read_free(a);
                    return 0;
                }
            }
            
            /* Ensure data is written to disk */
            fsync(fd);
            close(fd);
        }
    }
    
    archive_read_free(a);
    return 1;
}

int main(void) {
    /* Test case 1: Valid zip file */
    printf("Test 1: Extracting valid archive\\n");
    if (extract_zip_archive("archive.zip")) {
        printf("Test 1 passed\\n");
    } else {
        printf("Test 1 failed\\n");
    }
    
    /* Test case 2: Non-existent file */
    printf("\\nTest 2: Non-existent file\\n");
    if (!extract_zip_archive("nonexistent.zip")) {
        printf("Test 2 passed\\n");
    } else {
        printf("Test 2 failed\\n");
    }
    
    /* Test case 3: NULL input */
    printf("\\nTest 3: NULL input\\n");
    if (!extract_zip_archive(NULL)) {
        printf("Test 3 passed\\n");
    } else {
        printf("Test 3 failed\\n");
    }
    
    /* Test case 4: Empty string input */
    printf("\\nTest 4: Empty string input\\n");
    if (!extract_zip_archive("")) {
        printf("Test 4 passed\\n");
    } else {
        printf("Test 4 failed\\n");
    }
    
    /* Test case 5: Path with directory traversal attempt */
    printf("\\nTest 5: Malicious path (if archive contains traversal)\\n");
    if (extract_zip_archive("test_traversal.zip")) {
        printf("Test 5: extraction completed with validation\\n");
    } else {
        printf("Test 5: rejected malicious path\\n");
    }
    
    return 0;
}
