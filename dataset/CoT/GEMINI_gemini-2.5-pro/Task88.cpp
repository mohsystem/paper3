#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <filesystem>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // For mkstemp
#endif

/**
 * @brief Generates a random alphanumeric string.
 * @note Uses std::random_device, which is often a non-deterministic random
 *       number generator, suitable for many purposes but may not meet strict
 *       cryptographic standards on all platforms.
 * @param length The desired length of the string.
 * @return The generated random string.
 */
std::string generate_random_string(size_t length) {
    static const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    result.reserve(length);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    return result;
}


/**
 * @brief Creates a secure temporary file, writes a random string's unicode
 *        escapes to it, and returns the file path.
 * @return std::string The absolute path to the newly created temporary file.
 */
std::string createTempFileWithUnicodeString() {
    std::string path;
    std::ofstream file_stream;

#ifdef _WIN32
    char temp_path_buffer[MAX_PATH];
    if (GetTempPathA(MAX_PATH, temp_path_buffer) == 0) {
        throw std::runtime_error("Failed to get temporary directory path.");
    }
    char temp_filename[MAX_PATH];
    if (GetTempFileNameA(temp_path_buffer, "tempfile_", 0, temp_filename) == 0) {
        throw std::runtime_error("Failed to create temporary file name.");
    }
    path = temp_filename;
    file_stream.open(path, std::ios::out);
#else
    std::string temp_dir = std::filesystem::temp_directory_path().string();
    std::string path_template_str = temp_dir + "/tempfile_XXXXXX";
    std::vector<char> path_template(path_template_str.begin(), path_template_str.end());
    path_template.push_back('\0');

    int fd = mkstemp(path_template.data());
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file with mkstemp.");
    }
    close(fd); // Close descriptor, we'll open via path with fstream
    path = std::string(path_template.data());
    file_stream.open(path, std::ios::out);
#endif

    if (!file_stream.is_open()) {
        throw std::runtime_error("Failed to open temporary file for writing.");
    }

    try {
        std::string random_string = generate_random_string(20);

        std::stringstream ss;
        for (unsigned char c : random_string) {
            ss << "\\u" << std::setw(4) << std::setfill('0') << std::hex << static_cast<int>(c);
        }

        file_stream << ss.str();
        file_stream.close();

        return path;
    } catch (...) {
        if (file_stream.is_open()) {
            file_stream.close();
        }
        std::error_code ec;
        std::filesystem::remove(path, ec);
        throw;
    }
}

int main() {
    std::cout << "Running 5 test cases for C++..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::string file_path;
        try {
            file_path = createTempFileWithUnicodeString();
            std::cout << "Test case " << i << ": Success." << std::endl;
            std::cout << "  - File created at: " << file_path << std::endl;

            std::ifstream reader(file_path);
            if (reader.is_open()) {
                std::stringstream buffer;
                buffer << reader.rdbuf();
                std::cout << "  - File content: " << buffer.str() << std::endl;
                reader.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Test case " << i << ": Failed. Error: " << e.what() << std::endl;
        }

        if (!file_path.empty()) {
            std::error_code ec;
            std::filesystem::remove(file_path, ec);
            if (ec) {
                std::cerr << "  - Failed to clean up file " << file_path << ": " << ec.message() << std::endl;
            } else {
                std::cout << "  - Cleaned up file: " << file_path << std::endl;
            }
        }
        std::cout << "---------------------------------" << std::endl;
    }
    return 0;
}