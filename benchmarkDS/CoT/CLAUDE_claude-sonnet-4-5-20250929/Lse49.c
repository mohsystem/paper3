
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zip.h>
#include <limits.h>

int unzip_archive(const char* zip_file_path, const char* dest_directory) {
    /* Create destination directory */
    mkdir(dest_directory, 0755);
    
    int err = 0;
    zip_t* archive = zip_open(zip_file_path, 0, &err);
    if (archive == NULL) {
        fprintf(stderr, "Cannot open zip file: %s\\n", zip_file_path);
        return 0;
    }
    
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(archive, i, 0);
        if (name == NULL) continue;
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_directory, name);
        
        /* Create directory structure */
        char* last_slash = strrchr(full_path, '/');
        if (last_slash != NULL) {
            char dir_path[PATH_MAX];
            strncpy(dir_path, full_path, last_slash - full_path);
            dir_path[last_slash - full_path] = '\\0';
            
            char* p = dir_path;
            while (*p) {
                if (*p == '/') {
                    *p = '\\0';
                    mkdir(dir_path, 0755);
                    *p = '/';
                }
                p++;
            }
            mkdir(dir_path, 0755);
        }
        
        /* Check if entry is a file */
        if (name[strlen(name) - 1] != '/') {
            zip_file_t* file = zip_fopen_index(archive, i, 0);
            if (file) {
                FILE* out_file = fopen(full_path, "wb");
                if (out_file) {
                    char buffer[4096];
                    zip_int64_t bytes_read;
                    
                    while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                        fwrite(buffer, 1, bytes_read, out_file);
                    }
                    
                    fclose(out_file);
                }
                zip_fclose(file);
            }
        }
    }
    
    zip_close(archive);
    return 1;
}

int main() {
    const char* test_cases[] = {
        "archive.zip",
        "test1.zip",
        "test2.zip",
        "test3.zip",
        "test4.zip"
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("Extracting: %s\\n", test_cases[i]);
        if (unzip_archive(test_cases[i], "/tmp/unpack")) {
            printf("Successfully extracted: %s\\n", test_cases[i]);
        } else {
            fprintf(stderr, "Failed to extract: %s\\n", test_cases[i]);
        }
    }
    
    return 0;
}
