
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>

void extract_zip_archive(const char* zip_file_path, const char* dest_directory) {
    int err = 0;
    zip_t* archive = zip_open(zip_file_path, 0, &err);
    
    if (archive == NULL) {
        fprintf(stderr, "Cannot open zip file: %s\\n", zip_file_path);
        return;
    }
    
    mkdir(dest_directory, 0755);
    
    int num_entries = zip_get_num_entries(archive, 0);
    
    for (int i = 0; i < num_entries; i++) {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(archive, i, 0, &st);
        
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", dest_directory, st.name);
        
        if (st.name[strlen(st.name) - 1] == '/') {
            mkdir(file_path, 0755);
        } else {
            zip_file_t* file = zip_fopen_index(archive, i, 0);
            if (!file) continue;
            
            FILE* out_file = fopen(file_path, "wb");
            if (!out_file) {
                zip_fclose(file);
                continue;
            }
            
            char buffer[4096];
            int bytes_read;
            
            while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, bytes_read, out_file);
            }
            
            fclose(out_file);
            zip_fclose(file);
        }
    }
    
    zip_close(archive);
}

int main() {
    const char* test_cases[] = {
        "archive.zip",
        "test1.zip",
        "test2.zip",
        "test3.zip",
        "test4.zip"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Extracting: %s\\n", test_cases[i]);
        extract_zip_archive(test_cases[i], "/tmp/unpack");
        printf("Successfully extracted: %s\\n", test_cases[i]);
    }
    
    return 0;
}
