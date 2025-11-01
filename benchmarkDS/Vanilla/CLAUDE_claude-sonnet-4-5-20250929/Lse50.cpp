
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <zip.h>

void extractZipArchive(const std::string& zipFilePath, const std::string& destDirectory) {
    int err = 0;
    zip* archive = zip_open(zipFilePath.c_str(), 0, &err);
    
    if (archive == nullptr) {
        std::cerr << "Cannot open zip file: " << zipFilePath << std::endl;
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
            if (!file) continue;
            
            std::ofstream outFile(filePath, std::ios::binary);
            char buffer[4096];
            int bytesRead;
            
            while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                outFile.write(buffer, bytesRead);
            }
            
            outFile.close();
            zip_fclose(file);
        }
    }
    
    zip_close(archive);
}

int main() {
    std::string testCases[] = {
        "archive.zip",
        "test1.zip",
        "test2.zip",
        "test3.zip",
        "test4.zip"
    };
    
    for (const auto& testCase : testCases) {
        std::cout << "Extracting: " << testCase << std::endl;
        extractZipArchive(testCase, "/tmp/unpack");
        std::cout << "Successfully extracted: " << testCase << std::endl;
    }
    
    return 0;
}
