#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <cstdio> // For remove()

/**
 * @brief Demonstrates the POSIX open() call with various flags.
 * 
 * @param path The path to the file.
 * @param flags The bitmask of flags for open().
 * @param description A description of the test case.
 */
void demonstrate_open(const std::string& path, int flags, const std::string& description) {
    std::cout << "--- Test: " << description << " ---" << std::endl;
    std::cout << "Attempting to open '" << path << "'..." << std::endl;

    mode_t mode = S_IRUSR | S_IWUSR; // 0600 permissions
    int fd;

    // The mode argument is only used when O_CREAT is specified.
    if ((flags & O_CREAT) != 0) {
        fd = open(path.c_str(), flags, mode);
    } else {
        fd = open(path.c_str(), flags);
    }

    if (fd == -1) {
        std::cerr << "Failed as expected: " << std::strerror(errno) << std::endl;
    } else {
        std::cout << "Success: File opened with descriptor " << fd << "." << std::endl;
        
        if (description.find("Write data") != std::string::npos) {
            const char* content = "hello world";
            if (write(fd, content, strlen(content)) == -1) {
                 std::cerr << "Failed to write to file: " << std::strerror(errno) << std::endl;
            } else {
                std::cout << "Successfully wrote to file." << std::endl;
            }
        }
        
        close(fd);
    }
    std::cout << std::endl;
}

int main() {
    const std::string test_file = "cpp_testfile.txt";
    const std::string non_existent_file = "cpp_non_existent.txt";

    // Initial cleanup
    remove(test_file.c_str());
    remove(non_existent_file.c_str());

    // Test 1: O_WRONLY | O_CREAT | O_EXCL - Atomically create a new file. Fails if it exists.
    std::cout << "1) Demonstrating O_EXCL (exclusive creation)" << std::endl;
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_EXCL, "O_WRONLY | O_CREAT | O_EXCL (exclusive create)");

    // Test 2: Try to create it again with O_EXCL. Must fail.
    std::cout << "2) Demonstrating O_EXCL on an existing file" << std::endl;
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_EXCL, "O_WRONLY | O_CREAT | O_EXCL (on existing file)");

    // Test 3: Write some data to the file to demonstrate truncation later.
    std::cout << "3) Writing data to the file" << std::endl;
    demonstrate_open(test_file, O_WRONLY | O_TRUNC, "Write data to existing file");

    // Test 4: O_WRONLY | O_CREAT | O_TRUNC - Create if not exists, or truncate if it does.
    std::cout << "4) Demonstrating O_TRUNC on an existing file" << std::endl;
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_TRUNC, "O_WRONLY | O_CREAT | O_TRUNC (on existing file)");

    // Test 5: O_WRONLY on a non-existent file (without O_CREAT). Must fail.
    std::cout << "5) Demonstrating opening a non-existent file without O_CREAT" << std::endl;
    demonstrate_open(non_existent_file, O_WRONLY, "O_WRONLY (on non-existent file)");
    
    // Final cleanup
    remove(test_file.c_str());

    return 0;
}