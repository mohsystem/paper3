#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <stdexcept>
#include <sstream>
#include <memory>

// POSIX headers for secure file handling
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

// RAII wrapper for file descriptors using a unique_ptr with a custom deleter.
using FileDescriptor = std::unique_ptr<int, decltype(&close)>;

// Define a reasonable maximum file size to prevent resource exhaustion.
constexpr off_t MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MB

void process_file(const std::string& input_path, const std::string& output_path) {
    // --- Read Input File ---
    // Open the resource first, preventing symlink following with O_NOFOLLOW.
    int input_fd_raw = open(input_path.c_str(), O_RDONLY | O_NOFOLLOW);
    if (input_fd_raw < 0) {
        throw std::runtime_error("Error opening input file: " + std::string(strerror(errno)));
    }
    FileDescriptor input_fd(new int(input_fd_raw), &close);

    struct stat st;
    // Validate the already-opened handle.
    if (fstat(*input_fd, &st) != 0) {
        throw std::runtime_error("Error getting file stats: " + std::string(strerror(errno)));
    }

    // Reject if not a regular file or if size exceeds the policy limit.
    if (!S_ISREG(st.st_mode)) {
        throw std::runtime_error("Error: Input is not a regular file.");
    }
    if (st.st_size > MAX_FILE_SIZE) {
        throw std::runtime_error("Error: Input file size exceeds the limit.");
    }

    long file_size = st.st_size;
    std::vector<unsigned char> buffer;
    if (file_size > 0) {
        // std::vector safely manages memory for the buffer.
        buffer.resize(file_size);

        ssize_t bytes_read = read(*input_fd, buffer.data(), file_size);
        if (bytes_read < 0) {
            throw std::runtime_error("Error reading from input file: " + std::string(strerror(errno)));
        }
        if (bytes_read != file_size) {
            throw std::runtime_error("Error: Incomplete read from file.");
        }
    }
    input_fd.reset(); // Close file now that we are done with it

    // --- Calculate Checksum ---
    unsigned long checksum = std::accumulate(buffer.begin(), buffer.end(), 0UL);
    
    // --- Write Output File ---
    // Create output file securely. O_EXCL fails if the file already exists.
    int output_fd_raw = open(output_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (output_fd_raw < 0) {
        throw std::runtime_error("Error creating output file: " + std::string(strerror(errno)));
    }
    FileDescriptor output_fd(new int(output_fd_raw), &close);
    
    std::stringstream ss;
    ss << "Bytes read: " << buffer.size() << "\nChecksum: " << checksum << "\n";
    std::string output_content = ss.str();

    ssize_t bytes_written = write(*output_fd, output_content.c_str(), output_content.length());
    if (bytes_written < 0 || (size_t)bytes_written != output_content.length()) {
        throw std::runtime_error("Error writing to output file: " + std::string(strerror(errno)));
    }
    
    std::cout << "Successfully processed '" << input_path << "', results in '" << output_path << "'" << std::endl;
}

void run_test(const std::string& name, const std::string& content, bool should_succeed, bool create_file = true) {
    std::cout << "--- Test Case: " << name << " ---" << std::endl;
    const std::string input_file = "test_input.txt";
    const std::string output_file = "test_output.txt";

    remove(input_file.c_str());
    remove(output_file.c_str());
    
    if (create_file) {
        FILE* f = fopen(input_file.c_str(), "wb");
        if (f) {
            fwrite(content.c_str(), 1, content.size(), f);
            fclose(f);
        } else {
            std::cerr << "Failed to create test input file" << std::endl;
            return;
        }
    } else {
        std::cout << "Testing with non-existent input file." << std::endl;
    }

    try {
        process_file(input_file, output_file);
        if (!should_succeed) std::cout << "FAIL: Process succeeded unexpectedly." << std::endl;
        else std::cout << "PASS: Process succeeded as expected." << std::endl;
    } catch (const std::exception& e) {
        if (should_succeed) std::cout << "FAIL: Process failed unexpectedly with error: " << e.what() << std::endl;
        else std::cout << "PASS: Process failed as expected with error: " << e.what() << std::endl;
    }

    remove(input_file.c_str());
    remove(output_file.c_str());
    std::cout << std::endl;
}

int main() {
    run_test("Normal file", "Hello World!", true);
    run_test("Empty file", "", true);
    run_test("File with null bytes", std::string("a\0b\0c", 5), true);
    run_test("Non-existent file", "", false, false);
    
    std::cout << "--- Test Case: Output file exists ---" << std::endl;
    const std::string input_f = "test_input.txt";
    const std::string output_f = "test_output.txt";
    remove(input_f.c_str());
    remove(output_f.c_str());
    { FILE* f = fopen(input_f.c_str(), "w"); if(f) fclose(f); }
    { FILE* f = fopen(output_f.c_str(), "w"); if(f) fclose(f); }
    try {
        process_file(input_f, output_f);
        std::cout << "FAIL: Process succeeded but should have failed." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "PASS: Process failed as expected because output file exists." << std::endl;
    }
    remove(input_f.c_str());
    remove(output_f.c_str());
    std::cout << std::endl;
    
    return 0;
}