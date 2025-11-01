
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <zip.h>
#include <limits.h>
#include <unistd.h>

class Lse49 {
public:
    static bool unzipArchive(const std::string& zipFilePath, const std::string& destDirectory) {
        // Create destination directory
        mkdir(destDirectory.c_str(), 0755);
        
        // Get canonical path for security check
        char realDestPath[PATH_MAX];
        if (realpath(destDirectory.c_str(), realDestPath) == NULL) {
            strcpy(realDestPath, destDirectory.c_str());
        }
        std::string canonicalDestPath(realDestPath);
        
        int err = 0;
        zip* archive = zip_open(zipFilePath.c_str(), 0, &err);
        if (archive == NULL) {
            std::cerr << "Cannot open zip file: " << zipFilePath << std::endl;
            return false;
        }
        
        zip_int64_t numEntries = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < numEntries; i++) {
            const char* name = zip_get_name(archive, i, 0);
            if (name == NULL) continue;
            
            std::string fullPath = destDirectory + "/" + name;
            
            // Security check: prevent path traversal
            char realPath[PATH_MAX];
            std::string dirPath = fullPath.substr(0, fullPath.find_last_of('/'));
            mkdir(dirPath.c_str(), 0755);
            
            if (realpath(dirPath.c_str(), realPath) != NULL) {
                std::string canonicalPath(realPath);
                if (canonicalPath.find(canonicalDestPath) != 0) {
                    std::cerr << "Path traversal detected: " << name << std::endl;
                    continue;
                }
            }
            
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat_index(archive, i, 0, &st);
            
            if (name[strlen(name) - 1] != '/') {
                zip_file* file = zip_fopen_index(archive, i, 0);
                if (file) {
                    std::ofstream outFile(fullPath, std::ios::binary);
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
        return true;
    }
};

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
        if (Lse49::unzipArchive(testCase, "/tmp/unpack")) {
            std::cout << "Successfully extracted: " << testCase << std::endl;
        } else {
            std::cerr << "Failed to extract: " << testCase << std::endl;
        }
    }
    
    return 0;
}
