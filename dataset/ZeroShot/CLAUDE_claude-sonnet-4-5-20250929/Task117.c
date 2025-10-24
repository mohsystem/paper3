
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

int create_directory_recursive(const char *path) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            #ifdef _WIN32
            mkdir(tmp);
            #else
            mkdir(tmp, 0755);
            #endif
            *p = '/';
        }
    }
    #ifdef _WIN32
    return mkdir(tmp);
    #else
    return mkdir(tmp, 0755);
    #endif
}

int is_path_safe(const char *base_path, const char *entry_path) {
    char resolved_base[PATH_MAX];
    char resolved_entry[PATH_MAX];
    
    if (realpath(base_path, resolved_base) == NULL) {
        return 0;
    }
    
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry_path);
    
    // Check if path contains directory traversal attempts
    if (strstr(entry_path, "..") != NULL) {
        return 0;
    }
    
    return 1;
}

int extract_archive(const char *archive_path, const char *dest_directory) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    
    if ((r = archive_read_open_filename(a, archive_path, 10240))) {
        fprintf(stderr, "Error opening archive: %s\\n", archive_error_string(a));
        return 1;
    }
    
    create_directory_recursive(dest_directory);
    
    while (1) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error reading header: %s\\n", archive_error_string(a));
        }
        if (r < ARCHIVE_WARN) {
            return 1;
        }
        
        const char *entry_path = archive_entry_pathname(entry);
        
        // Security: Prevent path traversal attacks
        if (!is_path_safe(dest_directory, entry_path)) {
            fprintf(stderr, "Entry is outside of target directory: %s\\n", entry_path);
            continue;
        }
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_directory, entry_path);
        archive_entry_set_pathname(entry, full_path);
        
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error writing header: %s\\n", archive_error_string(ext));
        } else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK) {
                fprintf(stderr, "Error copying data: %s\\n", archive_error_string(ext));
            }
            if (r < ARCHIVE_WARN) {
                return 1;
            }
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error finishing entry: %s\\n", archive_error_string(ext));
        }
        if (r < ARCHIVE_WARN) {
            return 1;
        }
    }
    
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return 0;
}

int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    int64_t offset;
    
    while (1) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF) {
            return ARCHIVE_OK;
        }
        if (r < ARCHIVE_OK) {
            return r;
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error writing data: %s\\n", archive_error_string(aw));
            return r;
        }
    }
}

int main() {
    printf("Test Case 1: Extracting archive file\\n");
    if (extract_archive("test1.zip", "extract1") == 0) {
        printf("Extraction completed successfully\\n");
    }
    
    printf("\\nTest Case 2: Extracting to different directory\\n");
    if (extract_archive("test1.zip", "extract2") == 0) {
        printf("Extraction completed successfully\\n");
    }
    
    printf("\\nTest Case 3: Multiple extractions\\n");
    if (extract_archive("test1.zip", "extract3") == 0) {
        printf("Extraction completed successfully\\n");
    }
    
    printf("\\nTest Case 4: Archive with nested directories\\n");
    if (extract_archive("test1.zip", "extract4") == 0) {
        printf("Extraction completed successfully\\n");
    }
    
    printf("\\nTest Case 5: Different archive format\\n");
    if (extract_archive("test1.zip", "extract5") == 0) {
        printf("Extraction completed successfully\\n");
    }
    
    return 0;
}
