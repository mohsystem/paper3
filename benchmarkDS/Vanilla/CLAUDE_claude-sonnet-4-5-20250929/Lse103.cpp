
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

class Lse103 {
public:
    static int createFileExcl(const char* filename) {
        // O_CREAT: create if doesn't exist
        // O_EXCL: fail if exists
        // O_WRONLY: write only
        // O_TRUNC: truncate to 0 bytes
        int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1) {
            return -1;
        }
        close(fd);
        return 0;
    }
    
    static int createFileTrunc(const char* filename) {
        // Without O_EXCL - will truncate existing files
        int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1) {
            return -1;
        }
        close(fd);
        return 0;
    }
};

int main() {
    // Test case 1: Create new file with O_EXCL
    std::cout << "Test 1 - Create new file: " << Lse103::createFileExcl("test1.txt") << std::endl;
    
    // Test case 2: Try to create existing file with O_EXCL (should fail)
    std::cout << "Test 2 - Create existing file with O_EXCL: " << Lse103::createFileExcl("test1.txt") << std::endl;
    
    // Test case 3: Create/truncate file without O_EXCL
    std::cout << "Test 3 - Create/truncate file: " << Lse103::createFileTrunc("test2.txt") << std::endl;
    
    // Test case 4: Truncate existing file
    std::cout << "Test 4 - Truncate existing file: " << Lse103::createFileTrunc("test2.txt") << std::endl;
    
    // Test case 5: Create file in valid directory
    std::cout << "Test 5 - Create another file: " << Lse103::createFileExcl("test3.txt") << std::endl;
    
    // Cleanup
    unlink("test1.txt");
    unlink("test2.txt");
    unlink("test3.txt");
    
    return 0;
}
