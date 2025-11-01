
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <zip.h>

void unzipArchive(const std::string& zipFilePath, const std::string& destDirectory) {
    int err = 0;
    zip* archive = zip_open(zipFilePath.c_str(), 0, &err);
    
    if (archive == nullptr) {
        std::cerr << "Failed to open zip file: " << zipFilePath << std::endl;
        return;
    }
    
    mkdir(destDirectory.c_str(), 0755);
    
    int numEntries = zip_get_num_entries(archive, 0);
    for (int i = 0; i < numEntries; i++) {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(archive, i, 0, &st);
        
        std::string filePath = destDirectory + "/" + st.name;
        
        if (st.name[strlen(st.name) - 1] == '/') {
            mkdir(filePath.c_str(), 0755);
        } else {
            zip_file* file = zip_fopen_index(archive, i, 0);
            if (file) {
                std::ofstream outFile(filePath, std::ios::binary);
                char buffer[4096];
                zip_int64_t bytesRead;
                
                while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                    outFile.write(buffer, bytesRead);
                }
                
                outFile.close();
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
    
    std::cout << "Unzip operations completed." << std::endl;
    
    return 0;
}
