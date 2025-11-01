
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>

void unzipArchive(const char* zipFilePath, const char* destDirectory) {
    int err = 0;
    zip_t* archive = zip_open(zipFilePath, 0, &err);
    
    if (archive == NULL) {
        fprintf(stderr, "Failed to open zip file: %s\\n", zipFilePath);
        return;
    }
    
    mkdir(destDirectory, 0755);
    
    zip_int64_t numEntries = zip_get_num_entries(archive, 0);
    for (zip_int64_t i = 0; i < numEntries; i++) {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(archive, i, 0, &st);
        
        char filePath[512];
        snprintf(filePath, sizeof(filePath), "%s/%s", destDirectory, st.name);
        
        if (st.name[strlen(st.name) - 1] == '/') {
            mkdir(filePath, 0755);
        } else {
            zip_file_t* file = zip_fopen_index(archive, i, 0);
            if (file) {
                FILE* outFile = fopen(filePath, "wb");
                if (outFile) {
                    char buffer[4096];
                    zip_int64_t bytesRead;
                    
                    while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                        fwrite(buffer, 1, bytesRead, outFile);
                    }
                    
                    fclose(outFile);
                }
                zip_fclose(file);
            }
        }
    }
    
    zip_close(archive);
}

int main() {
    // Test case 1
    unzipArchive("archive.zip", "/tmp/unpack");
    
    // Test case 2
    unzipArchive("test1.zip", "/tmp/unpack1");
    
    // Test case 3
    unzipArchive("test2.zip", "/tmp/unpack2");
    
    // Test case 4
    unzipArchive("data.zip", "/tmp/unpack3");
    
    // Test case 5
    unzipArchive("files.zip", "/tmp/unpack4");
    
    printf("Unzip operations completed.\\n");
    
    return 0;
}
