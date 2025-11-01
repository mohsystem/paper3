#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <fstream>
#include <system_error>

// For secure file I/O on POSIX systems
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

/**
 * Validates a filename to prevent path traversal.
 * @param filename The filename to check.
 * @return true if the filename is simple and does not contain path components.
 */
bool is_valid_filename(const std::string& filename) {
    // Rule #1: Prevent path traversal.
    // Disallow path separators and special directory names.
    return !(filename.empty() ||
             filename.find('/') != std::string::npos ||
             filename.find('\\') != std::string::npos ||
             filename == "." || filename == "..");
}

/**
 * Generates 3 random floats, concatenates them, and writes to a file securely.
 * @param fileName The name of the file to create.
 * @return true if successful, false otherwise.
 */
bool generateAndWriteRandomFloats(const std::string& fileName) {
    if (!is_valid_filename(fileName)) {
        std::cerr << "Error: Invalid file name '" << fileName << "'. Path components are not allowed." << std::endl;
        return false;
    }

    // 1. Generate 3 random floats using a secure seed.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);

    std::stringstream ss;
    // 2. Convert to strings and 3. Concatenate.
    for (int i = 0; i < 3; ++i) {
        ss << distrib(gen);
    }
    std::string content = ss.str();

    // 4. Write to file securely using POSIX APIs.
    // Rules #2, #3: Use open() with O_CREAT and O_EXCL for a race-safe operation.
    // Set restrictive permissions (user read/write only).
    int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Error opening file " << fileName << ": " << strerror(errno) << std::endl;
        return false;
    }

    // Write content to the file descriptor.
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written < 0 || (size_t)bytes_written != content.length()) {
        std::cerr << "Error writing to file " << fileName << ": " << strerror(errno) << std::endl;
        close(fd); // Best effort to close.
        unlink(fileName.c_str()); // Clean up the partially written file.
        return false;
    }

    // Close the file descriptor.
    if (close(fd) == -1) {
        std::cerr << "Error closing file " << fileName << ": " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

void run_test_case(const std::string& fileName) {
    // Clean up before test.
    unlink(fileName.c_str());

    std::cout << "Attempting to write to: " << fileName << std::endl;
    if (generateAndWriteRandomFloats(fileName)) {
        std::cout << "Successfully wrote to " << fileName << std::endl;
        std::ifstream ifs(fileName);
        if (ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            std::cout << "File content: " << content << std::endl;
        } else {
            std::cerr << "Failed to read back file content." << std::endl;
        }
    } else {
        std::cerr << "Failed to write to " << fileName << std::endl;
    }
    std::cout << "---" << std::endl;
}

int main() {
    std::cout << "Running C++ test cases..." << std::endl;
    
    std::vector<std::string> testFiles = {
        "cpp_test1.txt", "cpp_test2.txt", "cpp_test3.txt",
        "cpp_test4.txt", "cpp_test5.txt"
    };

    for (const auto& fileName : testFiles) {
        run_test_case(fileName);
    }
    
    // Test case with an invalid path.
    std::cout << "Testing invalid path..." << std::endl;
    if (!generateAndWriteRandomFloats("../invalid_test.txt")) {
        std::cout << "Correctly failed to write to invalid path." << std::endl;
    } else {
        std::cerr << "Incorrectly allowed writing to invalid path." << std::endl;
    }

    return 0;
}