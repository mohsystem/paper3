#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>  // For open, write, close, ftruncate
#include <fcntl.h>   // For O_RDWR, O_CREAT
#include <sys/stat.h> // For S_IRUSR, S_IWUSR
#include <cerrno>    // For errno
#include <cstring>   // For strerror

// The main class is only required for Java.
// For C++, we can use a namespace or standalone function.
class Lse104 {
public:
    /**
     * Opens a file for reading and writing, creating it if it doesn't exist.
     * It then writes the given content to the file.
     *
     * @param fileName The name of the file to open/create.
     * @param content  The string content to write to the file.
     * @return true if the operation was successful, false otherwise.
     */
    static bool writeConfig(const std::string& fileName, const std::string& content) {
        // O_RDWR: Open for reading and writing.
        // O_CREAT: Create the file if it does not exist.
        // S_IRUSR | S_IWUSR: File permissions 0600 (owner can read/write).
        // This is a security best practice.
        int fd = open(fileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        if (fd == -1) {
            std::cerr << "Error opening file '" << fileName << "': " << strerror(errno) << std::endl;
            return false;
        }

        // Truncate the file to 0 length to ensure we are writing fresh content
        if (ftruncate(fd, 0) == -1) {
            std::cerr << "Error truncating file '" << fileName << "': " << strerror(errno) << std::endl;
            close(fd);
            return false;
        }

        ssize_t bytes_written = write(fd, content.c_str(), content.length());

        if (bytes_written == -1) {
            std::cerr << "Error writing to file '" << fileName << "': " << strerror(errno) << std::endl;
            close(fd);
            return false;
        }

        if (static_cast<size_t>(bytes_written) != content.length()) {
            std::cerr << "Incomplete write to file '" << fileName << "'" << std::endl;
            close(fd);
            return false;
        }

        close(fd);
        return true;
    }
};

int main() {
    std::string content = "important_config";
    std::vector<std::string> testFiles = {
        "config1.txt",
        "test/config2.txt", // Note: This may fail if 'test' directory doesn't exist.
        "config3.txt",
        "config4.txt",
        "config5.txt"
    };
    
    // Create a directory for the second test case
    mkdir("test", 0755);

    for (const auto& fileName : testFiles) {
        std::cout << "Attempting to write to: " << fileName << std::endl;
        bool result = Lse104::writeConfig(fileName, content);
        std::cout << "  -> Success: " << (result ? "true" : "false") << std::endl;
        if (result) {
            remove(fileName.c_str()); // Clean up created file
        }
    }
    
    // Clean up the created directory
    rmdir("test");

    return 0;
}