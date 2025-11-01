#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <filesystem>
#include <system_error>

// For POSIX-specific file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h> // for basename

namespace fs = std::filesystem;

const std::string BASE_IMAGE_DIR = "test_images_cpp";
const off_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

std::vector<char> img(const std::string& file) {
    if (file.empty()) {
        std::cerr << "Error: Filename is empty." << std::endl;
        return {};
    }

    // Rule #1 & #5: Sanitize input by taking only the basename.
    char* file_cstr = new char[file.length() + 1];
    std::strcpy(file_cstr, file.c_str());
    std::string sanitized_filename = basename(file_cstr);
    delete[] file_cstr;
    
    if (sanitized_filename != file || sanitized_filename.find("..") != std::string::npos) {
        std::cerr << "Error: Potential path traversal in filename '" << file << "'." << std::endl;
        return {};
    }

    // Rule #6: Open first, then validate (TOCTOU avoidance)
    int dir_fd = open(BASE_IMAGE_DIR.c_str(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        perror("Error opening base directory");
        return {};
    }

    int fd = openat(dir_fd, sanitized_filename.c_str(), O_RDONLY | O_NOFOLLOW);
    close(dir_fd);

    if (fd < 0) {
        std::cerr << "Error: Could not open file '" << sanitized_filename << "'." << std::endl;
        return {};
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("Error getting file status");
        close(fd);
        return {};
    }

    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: '" << sanitized_filename << "' is not a regular file." << std::endl;
        close(fd);
        return {};
    }

    // Rule #3: Check file size before reading.
    if (st.st_size > MAX_FILE_SIZE) {
        std::cerr << "Error: File size " << st.st_size << " exceeds limit." << std::endl;
        close(fd);
        return {};
    }

    if (st.st_size == 0) {
        close(fd);
        return {};
    }
    
    std::vector<char> buffer(st.st_size);
    ssize_t bytes_read = read(fd, buffer.data(), st.st_size);
    close(fd);

    if (bytes_read < 0 || bytes_read != st.st_size) {
        perror("Error reading file or incomplete read");
        return {};
    }

    return buffer;
}

void setup_test_environment() {
    try {
        fs::create_directory(BASE_IMAGE_DIR);
        std::ofstream(fs::path(BASE_IMAGE_DIR) / "test.txt") << "valid content";
        fs::create_directory(fs::path(BASE_IMAGE_DIR) / "subdir");
        std::ofstream(fs::path(BASE_IMAGE_DIR) / "subdir" / "nested.txt") << "nested content";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Setup failed: " << e.what() << std::endl;
    }
}

void cleanup_test_environment() {
    fs::remove_all(BASE_IMAGE_DIR);
}

void run_test_case(int num, const std::string& desc, const std::string& input, bool success_expected) {
    std::cout << "\n" << num << ". Testing " << desc << " '" << input << "'" << std::endl;
    std::vector<char> content = img(input);
    bool succeeded = !content.empty() || (img("test.txt").size() > 0 && input == "test.txt");

    if (success_expected) {
        if (succeeded) {
            std::cout << "Success. Content: " << std::string(content.begin(), content.end()) << std::endl;
        } else {
            std::cout << "Failed." << std::endl;
        }
    } else {
        if (!succeeded) {
            std::cout << "Success. Request blocked as expected." << std::endl;
        } else {
            std::cout << "Failed." << std::endl;
        }
    }
}

int main() {
    setup_test_environment();
    
    std::cout << "--- C++ Test Cases ---" << std::endl;

    run_test_case(1, "valid file", "test.txt", true);
    run_test_case(2, "non-existent file", "notfound.txt", false);
    run_test_case(3, "path traversal", "../test.txt", false);
    std::string abs_path = fs::absolute(fs::path(BASE_IMAGE_DIR) / "test.txt").string();
    run_test_case(4, "absolute path", abs_path, false);
    run_test_case(5, "nested path", "subdir/nested.txt", false);

    cleanup_test_environment();
    return 0;
}