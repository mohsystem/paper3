#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// POSIX headers for secure file I/O
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

const off_t MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MB limit

bool processFile(const std::string& inputPath, const std::string& outputPath) {
    // 1. Open the file securely, without following symlinks.
    int fd = open(inputPath.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        perror(("Error opening input file: " + inputPath).c_str());
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("Error getting file stats");
        close(fd);
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: Input path is not a regular file." << std::endl;
        close(fd);
        return false;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        std::cerr << "Error: File size " << st.st_size << " exceeds maximum limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        close(fd);
        return false;
    }

    std::vector<char> buffer(st.st_size);
    ssize_t bytes_read = 0;
    if (st.st_size > 0) {
        char* p = buffer.data();
        ssize_t remaining = st.st_size;
        while(remaining > 0) {
            ssize_t current_read = read(fd, p, remaining);
            if (current_read < 0) {
                perror("Error reading from file");
                close(fd);
                return false;
            }
            if(current_read == 0) break; // EOF
            bytes_read += current_read;
            p += current_read;
            remaining -= current_read;
        }

        if (bytes_read != st.st_size) {
            std::cerr << "Error: Could not read the entire file." << std::endl;
            close(fd);
            return false;
        }
    }
    close(fd);

    uint32_t checksum = 0;
    for (size_t i = 0; i < (size_t)bytes_read; ++i) {
        checksum += static_cast<unsigned char>(buffer[i]);
    }
    
    std::string tempOutputPath = outputPath + ".tmp";
    int out_fd = open(tempOutputPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (out_fd < 0) {
        perror(("Error creating temporary output file: " + tempOutputPath).c_str());
        return false;
    }
    
    char out_buffer[256];
    int len = snprintf(out_buffer, sizeof(out_buffer), "Bytes read: %zd\nChecksum: %u\n", bytes_read, checksum);

    if (write(out_fd, out_buffer, len) != len) {
        perror("Error writing to temporary file");
        close(out_fd);
        unlink(tempOutputPath.c_str());
        return false;
    }

    if (fsync(out_fd) != 0) {
        perror("Error syncing temporary file to disk");
        close(out_fd);
        unlink(tempOutputPath.c_str());
        return false;
    }
    close(out_fd);
    
    if (rename(tempOutputPath.c_str(), outputPath.c_str()) != 0) {
        perror("Error renaming temporary file");
        unlink(tempOutputPath.c_str());
        return false;
    }

    return true;
}

void createTestFile(const std::string& path, const std::string& content) {
    FILE* f = fopen(path.c_str(), "wb");
    if (f) {
        if (!content.empty()) {
            fwrite(content.c_str(), 1, content.size(), f);
        }
        fclose(f);
    }
}

void runTestCase(int testNum, const std::string& name, const std::string& content, bool create, bool shouldSucceed) {
    std::cout << "\n--- Test Case " << testNum << ": " << name << " ---" << std::endl;
    std::string inputPath = name + ".txt";
    std::string outputPath = name + ".out";
    
    unlink(inputPath.c_str());
    unlink(outputPath.c_str());
    if (create) {
        createTestFile(inputPath, content);
    }

    std::cout << "Input: " << inputPath << ", Output: " << outputPath << std::endl;
    bool result = processFile(inputPath, outputPath);

    std::cout << "Execution result: " << (result ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Expected result:  " << (shouldSucceed ? "SUCCESS" : "FAILURE") << std::endl;

    if (result == shouldSucceed) {
        std::cout << "Test PASSED." << std::endl;
    } else {
        std::cout << "Test FAILED." << std::endl;
    }
    
    unlink(inputPath.c_str());
    unlink(outputPath.c_str());
}

int main() {
    runTestCase(1, "cpp_empty_file", "", true, true);
    runTestCase(2, "cpp_hello_file", "hello world", true, true);
    std::string binary_content;
    binary_content += (char)0xff;
    binary_content += (char)0xfe;
    binary_content += (char)0x01;
    binary_content += (char)0x00;
    runTestCase(3, "cpp_binary_file", binary_content, true, true);
    runTestCase(4, "cpp_nulls_file", std::string("\0\0\0", 3), true, true);
    runTestCase(5, "cpp_non_existent_file", "", false, false);
    
    return 0;
}