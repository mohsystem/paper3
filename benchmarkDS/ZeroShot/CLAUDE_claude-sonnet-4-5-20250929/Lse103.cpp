
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

class Lse103 {
public:
    /**
     * Opens a file with O_CREAT | O_TRUNC | O_WRONLY | O_EXCL flags
     * Creates file if it doesn't exist, fails if it exists
     */
    static int openFileExclusive(const char* filename) {
        // O_CREAT | O_EXCL | O_WRONLY | O_TRUNC
        int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, 0644);
        if (fd == -1) {
            return -1; // Failure
        }
        close(fd);
        return 0; // Success
    }

    /**
     * Opens a file with O_CREAT | O_TRUNC | O_WRONLY flags
     * Creates or truncates existing file
     */
    static int openFileTruncate(const char* filename) {
        // O_CREAT | O_WRONLY | O_TRUNC
        int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd == -1) {
            return -1; // Failure
        }
        close(fd);
        return 0; // Success
    }
};

int main() {
    std::cout << "Test Case 1: Create new file with exclusive flag" << std::endl;
    int result1 = Lse103::openFileExclusive("test1.txt");
    std::cout << "Result: " << (result1 == 0 ? "SUCCESS" : "FAILED") << std::endl;

    std::cout << "\\nTest Case 2: Try to create existing file with exclusive flag (should fail)" << std::endl;
    int result2 = Lse103::openFileExclusive("test1.txt");
    std::cout << "Result: " << (result2 == -1 ? "FAILED AS EXPECTED" : "UNEXPECTED SUCCESS") << std::endl;

    std::cout << "\\nTest Case 3: Create new file with truncate flag" << std::endl;
    int result3 = Lse103::openFileTruncate("test2.txt");
    std::cout << "Result: " << (result3 == 0 ? "SUCCESS" : "FAILED") << std::endl;

    std::cout << "\\nTest Case 4: Truncate existing file" << std::endl;
    int result4 = Lse103::openFileTruncate("test2.txt");
    std::cout << "Result: " << (result4 == 0 ? "SUCCESS" : "FAILED") << std::endl;

    std::cout << "\\nTest Case 5: Create file in subdirectory" << std::endl;
    mkdir("testdir", 0755);
    int result5 = Lse103::openFileTruncate("testdir/test3.txt");
    std::cout << "Result: " << (result5 == 0 ? "SUCCESS" : "FAILED") << std::endl;

    // Cleanup
    unlink("test1.txt");
    unlink("test2.txt");
    unlink("testdir/test3.txt");
    rmdir("testdir");

    return 0;
}
