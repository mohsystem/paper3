
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

bool deleteFile(const char* filePath) {
    if (filePath == nullptr || strlen(filePath) == 0) {
        std::cout << "Error: File path is empty or null" << std::endl;
        return false;
    }
    
    struct stat pathStat;
    if (stat(filePath, &pathStat) != 0) {
        std::cout << "Error: File does not exist - " << filePath << std::endl;
        return false;
    }
    
    if (S_ISDIR(pathStat.st_mode)) {
        std::cout << "Error: Path is a directory, not a file - " << filePath << std::endl;
        return false;
    }
    
    if (remove(filePath) == 0) {
        std::cout << "File deleted successfully: " << filePath << std::endl;
        return true;
    } else {
        std::cout << "Error: Failed to delete file - " << filePath << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            deleteFile(argv[i]);
        }
    } else {
        // Test cases
        std::cout << "Running test cases...\\n" << std::endl;
        
        // Test case 1: Create and delete a test file
        std::cout << "Test 1:" << std::endl;
        std::ofstream testFile1("test_file_1.txt");
        testFile1 << "test content";
        testFile1.close();
        deleteFile("test_file_1.txt");
        
        std::cout << std::endl;
        
        // Test case 2: Try to delete non-existent file
        std::cout << "Test 2:" << std::endl;
        deleteFile("non_existent_file.txt");
        
        std::cout << std::endl;
        
        // Test case 3: Create and delete another test file
        std::cout << "Test 3:" << std::endl;
        std::ofstream testFile3("test_file_3.dat");
        testFile3 << "test data";
        testFile3.close();
        deleteFile("test_file_3.dat");
        
        std::cout << std::endl;
        
        // Test case 4: Try to delete with empty path
        std::cout << "Test 4:" << std::endl;
        deleteFile("");
        
        std::cout << std::endl;
        
        // Test case 5: Try to delete with null path
        std::cout << "Test 5:" << std::endl;
        deleteFile(nullptr);
    }
    
    return 0;
}
