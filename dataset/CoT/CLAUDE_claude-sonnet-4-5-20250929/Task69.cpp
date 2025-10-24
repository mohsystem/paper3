
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <cstring>

bool deleteFile(const std::string& filePath) {
    if (filePath.empty()) {
        std::cout << "Error: File path cannot be empty" << std::endl;
        return false;
    }
    
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        std::cout << "Error: File does not exist: " << filePath << std::endl;
        return false;
    }
    
    if (!S_ISREG(fileInfo.st_mode)) {
        std::cout << "Error: Path is not a regular file: " << filePath << std::endl;
        return false;
    }
    
    if (remove(filePath.c_str()) == 0) {
        std::cout << "File deleted successfully: " << filePath << std::endl;
        return true;
    } else {
        std::cout << "Error: Failed to delete file - " << strerror(errno) << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
        
        std::cout << "\\n=== Running Test Cases ===\\n" << std::endl;
        
        std::string testFile1 = "test1_temp.txt";
        std::ofstream file1(testFile1);
        file1 << "test";
        file1.close();
        std::cout << "Test 1: Delete existing file" << std::endl;
        deleteFile(testFile1);
        
        std::cout << "\\nTest 2: Delete non-existent file" << std::endl;
        deleteFile("non_existent_file.txt");
        
        std::cout << "\\nTest 3: Delete with empty path" << std::endl;
        deleteFile("");
        
        std::string testFile4 = "test4_temp.txt";
        std::ofstream file4(testFile4);
        file4 << "test";
        file4.close();
        std::cout << "\\nTest 4: Delete valid file" << std::endl;
        deleteFile(testFile4);
        
        std::string testFile5 = "test5_temp.txt";
        std::ofstream file5(testFile5);
        file5 << "test";
        file5.close();
        std::cout << "\\nTest 5: Delete another valid file" << std::endl;
        deleteFile(testFile5);
        
        return 0;
    }
    
    deleteFile(argv[1]);
    return 0;
}
