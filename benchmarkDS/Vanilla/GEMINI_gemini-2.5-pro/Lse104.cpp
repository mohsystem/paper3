#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

bool createFileAndWrite(const std::string& filename, const std::string& content) {
    // O_RDWR: Open for reading and writing.
    // O_CREAT: Create the file if it does not exist.
    // O_TRUNC: Truncate file to length 0 if it exists.
    // 0644: Permissions for the new file (owner read/write, group/other read).
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    
    if (fd == -1) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }
    
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

int main() {
    std::string content = "important_config";

    std::cout << "--- C++ Test Cases ---" << std::endl;
    
    // Test Case 1: Standard file creation
    std::string file1 = "cpp_test1.cfg";
    std::cout << "\nTest 1: Writing to '" << file1 << "'" << std::endl;
    if (createFileAndWrite(file1, content)) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    
    // Test Case 2: Different content
    std::string file2 = "cpp_test2.dat";
    std::cout << "\nTest 2: Writing to '" << file2 << "'" << std::endl;
    if (createFileAndWrite(file2, content + " for test 2")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    
    // Test Case 3: Path with non-existent directory (should fail)
    std::string file3 = "data/cpp_test3.txt";
    std::cout << "\nTest 3: Writing to '" << file3 << "'" << std::endl;
    if (createFileAndWrite(file3, content)) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    
    // Test Case 4: Empty filename (should fail)
    std::string file4 = "";
    std::cout << "\nTest 4: Writing to empty filename" << std::endl;
    if (createFileAndWrite(file4, content)) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    
    // Test Case 5: Writing empty content
    std::string file5 = "cpp_test5.log";
    std::cout << "\nTest 5: Writing an empty string to '" << file5 << "'" << std::endl;
    if (createFileAndWrite(file5, "")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }

    return 0;
}