
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <archive.h>
#include <archive_entry.h>
#include <errno.h>
#include <libgen.h>

#define EXTRACT_DIR "/tmp/unpack"
#define BUFFER_SIZE 8192

int is_path_safe(const char* base_path, const char* target_path) {
    /* Security: Check for path traversal attempts */
    if (strstr(target_path, "..") != NULL) {
        return 0;
    }
    if (target_path[0] == '/') {
        return 0;
    }
    return 1;
}

int create_directory_recursive(const char* path) {
    char tmp[1024];
    char* p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

int extract_tar_archive(const char* tar_file_path) {
    struct archive* a;
    struct archive_entry* entry;
    int r;
    char target_path[2048];
    
    /* Create extraction directory */
    if (create_directory_recursive(EXTRACT_DIR) != 0) {
        fprintf(stderr, "Failed to create extraction directory\\n");
        return -1;
    }
    
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    if (archive_read_open_filename(a, tar_file_path, 10240) != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open archive: %s\\n", archive_error_string(a));
        archive_read_free(a);
        return -1;
    }
    
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* entry_name = archive_entry_pathname(entry);
        
        /* Security: Validate path */
        if (!is_path_safe(EXTRACT_DIR, entry_name)) {
            fprintf(stderr, "Path traversal attempt detected: %s\\n", entry_name);
            archive_read_free(a);
            return -1;
        }
        
        snprintf(target_path, sizeof(target_path), "%s/%s", EXTRACT_DIR, entry_name);
        archive_entry_set_pathname(entry, target_path);
        
        r = archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Warning: Failed to extract %s: %s\\n", 
                   entry_name, archive_error_string(a));
        }
    }
    
    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

int write_file_example(const char* file_path, const char* content) {
    FILE* file = fopen(file_path, "w");
    if (file == NULL) {
        return -1;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}

char* read_file_example(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, size, file);
    buffer[size] = '\\0';
    fclose(file);
    
    return buffer;
}

int main() {
    const char* test_cases[] = {
        "/tmp/test1.tar",
        "/tmp/test2.tar.gz",
        "/tmp/test3.tar.bz2",
        "/tmp/test4.tgz",
        "/tmp/test5.tar"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Extracting: %s\\n", test_cases[i]);
        if (extract_tar_archive(test_cases[i]) == 0) {
            printf("Successfully extracted to %s\\n", EXTRACT_DIR);
        } else {
            fprintf(stderr, "Error extracting %s\\n", test_cases[i]);
        }
    }
    
    /* Demonstrate file operations */
    char test_file_path[1024];
    snprintf(test_file_path, sizeof(test_file_path), "%s/test.txt", EXTRACT_DIR);
    
    if (write_file_example(test_file_path, "Test content\\n") == 0) {
        char* content = read_file_example(test_file_path);
        if (content != NULL) {
            printf("File content: %s", content);
            free(content);
        }
    }
    
    return 0;
}
