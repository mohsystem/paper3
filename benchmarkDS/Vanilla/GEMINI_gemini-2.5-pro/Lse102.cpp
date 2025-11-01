#include <iostream>
#include <string>
#include <fcntl.h>   // For open() and O_ flags
#include <unistd.h>  // For write() and close()
#include <sys/stat.h> // For S_I flags (mode)
#include <cstring>   // For strerror()
#include <cerrno>    // For errno

/**
 * Opens/creates a file with specific POSIX permissions and writes content to it.
 *
 * @param filename The name of the file to create/open.
 * @param content  The string content to write to the file.
 */
void createAndWriteFile(const std::string& filename, const std::string& content) {
    // Permissions: S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    // Corresponds to mode 0644 (owner: rw, group: r, other: r)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    // Flags: O_RDWR (read/write), O_CREAT (create if non-existent)
    // O_TRUNC can be added to clear the file if it exists, otherwise write overwrites from the start.
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);
    
    if (fd == -1) {
        std::cerr << "Error opening file " << filename << ": " << strerror(errno) << std::endl;
        return;
    }
    
    // Write the content to the file
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file " << filename << ": " << strerror(errno) << std::endl;
    } else {
        std::cout << "Successfully wrote " << bytes_written << " bytes to " << filename << std::endl;
    }

    // Close the file descriptor
    if (close(fd) == -1) {
        std::cerr << "Error closing file " << filename << ": " << strerror(errno) << std::endl;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    // Test Case 1: As per the prompt
    createAndWriteFile("important_config", "important_config");
    // Test Case 2: A different file and content
    createAndWriteFile("cpp_test.log", "Log entry from C++.");
    // Test Case 3: A file with empty content
    createAndWriteFile("cpp_empty.txt", "");
    // Test Case 4: Overwriting the first file
    createAndWriteFile("important_config", "new important_config content from C++");
    // Test Case 5: Another new file
    createAndWriteFile("cpp_data.json", "{\"key\": \"value\"}");
    std::cout << "----------------------\n" << std::endl;
    return 0;
}