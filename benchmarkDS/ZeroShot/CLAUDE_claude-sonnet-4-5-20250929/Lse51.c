
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>

int extract_tar_archive(const char* tar_file_path, const char* dest_directory) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;
    
    // Create destination directory
    mkdir(dest_directory, 0755);
    
    a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);
    archive_read_support_filter_bzip2(a);
    archive_read_support_filter_none(a);
    
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM);
    
    if ((r = archive_read_open_filename(a, tar_file_path, 10240))) {
        fprintf(stderr, "Error opening archive: %s\\n", archive_error_string(a));
        return 0;
    }
    
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_directory, archive_entry_pathname(entry));
        
        // Security check: prevent path traversal
        if (strstr(full_path, "..") != NULL) {
            fprintf(stderr, "Path traversal detected, skipping entry\\n");
            continue;
        }
        
        archive_entry_set_pathname(entry, full_path);
        
        r = archive_write_header(ext, entry);
        if (r == ARCHIVE_OK) {
            const void *buff;
            size_t size;
            int64_t offset;
            
            while ((r = archive_read_data_block(a, &buff, &size, &offset)) == ARCHIVE_OK) {
                archive_write_data_block(ext, buff, size, offset);
            }
        }
        archive_write_finish_entry(ext);
    }
    
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return 1;
}

int main() {
    const char* test_files[] = {
        "test1.tar",
        "test2.tar.gz",
        "test3.tar.bz2",
        "test4.tgz",
        "test5.tar"
    };
    
    for (int i = 0; i < 5; i++) {
        if (extract_tar_archive(test_files[i], "/tmp/unpack")) {
            printf("Successfully extracted: %s\\n", test_files[i]);
        } else {
            fprintf(stderr, "Failed to extract: %s\\n", test_files[i]);
        }
    }
    
    return 0;
}
