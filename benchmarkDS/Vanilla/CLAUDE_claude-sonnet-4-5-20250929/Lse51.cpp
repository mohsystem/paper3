
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

class Lse51 {
public:
    static void unzipTarArchive(const std::string& tarFilePath, const std::string& destDirectory) {
        // Create destination directory
        std::string mkdirCmd = "mkdir -p " + destDirectory;
        system(mkdirCmd.c_str());
        
        // Determine compression type and extract
        std::string extractCmd;
        if (tarFilePath.find(".tar.gz") != std::string::npos || 
            tarFilePath.find(".tgz") != std::string::npos) {
            extractCmd = "tar -xzf " + tarFilePath + " -C " + destDirectory;
        } else if (tarFilePath.find(".tar.bz2") != std::string::npos) {
            extractCmd = "tar -xjf " + tarFilePath + " -C " + destDirectory;
        } else if (tarFilePath.find(".tar") != std::string::npos) {
            extractCmd = "tar -xf " + tarFilePath + " -C " + destDirectory;
        } else {
            std::cerr << "Unsupported file format" << std::endl;
            return;
        }
        
        system(extractCmd.c_str());
    }
    
    static std::string openFile(const std::string& filePath, const std::string& mode) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "Error: Could not open file";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        return content;
    }
};

int main() {
    std::string testCases[] = {
        "test1.tar.gz",
        "test2.tar.bz2",
        "test3.tar",
        "test4.tgz",
        "test5.tar.gz"
    };
    
    for (const auto& testCase : testCases) {
        std::cout << "Extracting: " << testCase << std::endl;
        try {
            Lse51::unzipTarArchive(testCase, "/tmp/unpack");
            std::cout << "Successfully extracted: " << testCase << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error extracting " << testCase << ": " << e.what() << std::endl;
        }
    }
    
    return 0;
}
