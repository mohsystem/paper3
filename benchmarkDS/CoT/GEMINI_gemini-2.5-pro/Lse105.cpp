#include <iostream>
#include <string>
#include <cstdio> // For remove()

// Use preprocessor directives to handle platform differences (POSIX vs Windows)
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring> // For strerror
#endif

class Lse105 {
public:
    /**
     * @brief Creates a file with given content and restrictive permissions.
     * On POSIX systems, this is done atomically by setting mode 0600 on creation.
     * On Windows, it's a two-step process (create, then set permissions).
     * 
     * @param filename The name of the file to create.
     * @param content The string content to write to the file.
     * @return true on success, false on failure.
     */
    static bool createSecretFile(const std::string& filename, const std::string& content) {
#ifdef _WIN32
        // Windows implementation (less secure due to create-then-chmod)
        int fd = _open(filename.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);
        if (fd == -1) {
            perror("Error opening/creating file on Windows");
            return false;
        }

        if (_write(fd, content.c_str(), content.length()) != static_cast<int>(content.length())) {
            perror("Error writing to file on Windows");
            _close(fd);
            return false;
        }

        _close(fd);

        if (_chmod(filename.c_str(), _S_IREAD | _S_IWRITE) != 0) {
            perror("Error setting file permissions on Windows");
            return false;
        }
        std::cout << "Successfully created '" << filename << "' on Windows with basic permissions." << std::endl;
        return true;
#else
        // POSIX implementation (secure, atomic permission setting)
        // S_IRUSR: read permission for owner. S_IWUSR: write permission for owner. (0600)
        mode_t mode = S_IRUSR | S_IWUSR;
        
        // O_WRONLY: write-only. O_CREAT: create. O_TRUNC: truncate if exists.
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
        
        if (fd == -1) {
            std::cerr << "Error opening/creating file '" << filename << "': " << strerror(errno) << std::endl;
            return false;
        }
        
        ssize_t bytes_written = write(fd, content.c_str(), content.length());
        
        if (bytes_written == -1) {
            std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
            close(fd);
            return false;
        }
        
        if (static_cast<size_t>(bytes_written) != content.length()) {
            std::cerr << "Error: Incomplete write to file '" << filename << "'" << std::endl;
            close(fd);
            return false;
        }
        
        if (close(fd) == -1) {
            std::cerr << "Error closing file '" << filename << "': " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "Successfully created and secured '" << filename << "' with permissions 0600." << std::endl;
        return true;
#endif
    }
};

int main() {
    std::string filename = "secret.txt";
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        
        // Clean up before each test to ensure a fresh start
        remove(filename.c_str());

        bool success = Lse105::createSecretFile(filename, content);
        std::cout << "Test " << i << " result: " << (success ? "Success" : "Failure") << std::endl << std::endl;
    }

    return 0;
}