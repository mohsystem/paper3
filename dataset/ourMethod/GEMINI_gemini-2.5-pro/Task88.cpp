#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <iomanip>
#include <openssl/rand.h>

// This code requires C++17 for std::filesystem and linking with OpenSSL.
// Example compilation: g++ your_source.cpp -std=c++17 -o program -lstdc++fs -lssl -lcrypto

// Function to generate a cryptographically secure random string of a given length.
std::string generateRandomString(const int len) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int charset_size = sizeof(charset) - 1;
    std::string result;
    result.reserve(len);
    std::vector<unsigned char> rand_bytes(len);

    if (RAND_bytes(rand_bytes.data(), len) != 1) {
        throw std::runtime_error("Failed to generate random bytes using OpenSSL.");
    }

    for (int i = 0; i < len; ++i) {
        result += charset[rand_bytes[i] % charset_size];
    }
    return result;
}

/**
 * @brief Creates a temporary file, writes a random string's Unicode representation to it.
 * 
 * Note: Standard C++ fstream does not have a secure, race-condition-free way to create
 * temporary files. This implementation mitigates this by using a cryptographically
 * random filename, but it is not completely immune to TOCTOU attacks. For higher
- * security, platform-specific APIs (like mkstemp on POSIX) should be used.
 *
 * @return std::string The absolute path to the created temporary file.
 * @throws std::runtime_error if any file or cryptographic operation fails.
 */
std::string createTempFileWithUnicodeString() {
    std::filesystem::path temp_path;
    try {
        // 1. Generate a random filename in the system's temp directory.
        std::string rand_filename = "task88_" + generateRandomString(16) + ".txt";
        temp_path = std::filesystem::temp_directory_path() / rand_filename;

        // 2. Create and open the file.
        std::ofstream temp_file(temp_path, std::ios::out | std::ios::trunc);
        if (!temp_file.is_open()) {
            throw std::runtime_error("Failed to create temporary file at: " + temp_path.string());
        }

        // 3. Generate random string content.
        std::string random_content = generateRandomString(32);
        
        // 4. Convert to Unicode representation and write to file.
        for (char c : random_content) {
            // Use unsigned char to prevent sign extension when promoting to int for printing.
            unsigned char uc = static_cast<unsigned char>(c);
            temp_file << "U+" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(uc) << " ";
        }
        
        // Explicitly close to check for write errors.
        temp_file.close();
        if (temp_file.fail()) {
             throw std::runtime_error("Failed to write to or close temporary file: " + temp_path.string());
        }
        
        // 5. Return the file path.
        return std::filesystem::absolute(temp_path).string();
    } catch (const std::exception&) {
        // If file was created but an error occurred later, try to clean up.
        if (!temp_path.empty() && std::filesystem::exists(temp_path)) {
            std::error_code ec;
            std::filesystem::remove(temp_path, ec); // Ignore cleanup error
        }
        throw; // Re-throw the exception
    }
}

int main() {
    for (int i = 0; i < 5; ++i) {
        std::string file_path;
        try {
            file_path = createTempFileWithUnicodeString();
            std::cout << "Test Case " << i + 1 << ": Created temp file at: " << file_path << std::endl;
            
            // Optional: Read and print content for verification
            std::ifstream f(file_path);
            if (f.is_open()) {
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                std::cout << "  File content: " << content << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Test Case " << i + 1 << ": An error occurred: " << e.what() << std::endl;
        }

        // Clean up the created file
        if (!file_path.empty()) {
            std::error_code ec;
            if (std::filesystem::remove(file_path, ec)) {
                std::cout << "  Cleaned up temp file: " << file_path << std::endl;
            } else if (ec) {
                // Only print an error if the file was expected to exist.
                std::cerr << "  Failed to clean up temp file " << file_path << ": " << ec.message() << std::endl;
            }
        }
    }
    return 0;
}