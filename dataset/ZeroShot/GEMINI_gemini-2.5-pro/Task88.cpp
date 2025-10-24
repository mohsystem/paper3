#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <cstdio> // For C-style file I/O

/**
 * Generates a random alphanumeric string.
 * Uses a thread_local generator for efficiency.
 * @param length The length of the alphanumeric part.
 * @return The generated random string, including a multi-byte Unicode character.
 */
std::string generateRandomString(int length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    // thread_local ensures each thread has its own generator, seeded once.
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

    std::string random_string;
    random_string.reserve(length + 4); // Reserve space for string + "-€" + null
    for (int i = 0; i < length; ++i) {
        random_string += charset[distribution(generator)];
    }
    
    // Add a unicode character (Euro sign) to demonstrate UTF-8
    // The Euro sign € is U+20AC, which is 0xE2 0x82 0xAC in UTF-8
    random_string += "-\xE2\x82\xAC";
    
    return random_string;
}

/**
 * Creates a secure temporary file, writes a random UTF-8 string, and returns the path.
 * @return The path to the created temporary file.
 * @throws std::runtime_error if the file cannot be created.
 */
std::string createTempFileWithRandomUnicode() {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    
    for (int i = 0; i < 10; ++i) { // Try up to 10 times to find a unique name
        // 1. Generate a highly random filename to avoid collisions.
        uint64_t timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;
        std::string filename = "tempfile_" + std::to_string(timestamp) + "_" + std::to_string(dist(generator)) + ".txt";
        std::filesystem::path file_path = temp_dir / filename;

        // 2. Securely create and open the file using C11 "wx" mode.
        // This is an atomic operation that fails if the file already exists,
        // preventing race condition vulnerabilities (TOCTOU).
        FILE* fp = fopen(file_path.string().c_str(), "wx");

        if (fp) {
            // 3. Generate random content and write to file.
            std::string random_content = generateRandomString(32);
            fputs(random_content.c_str(), fp);
            fclose(fp);
            // 4. Return the path.
            return file_path.string();
        }
        // If fp is null, the file might exist, so we loop and try a new name.
    }
    
    throw std::runtime_error("Failed to create a unique temporary file after multiple attempts.");
}

int main() {
    std::cout << "Running 5 test cases for C++...\n";
    std::vector<std::string> created_files;
    for (int i = 0; i < 5; ++i) {
        try {
            std::string file_path = createTempFileWithRandomUnicode();
            std::cout << "Test Case " << (i + 1) << ": Success. File created at: " << file_path << std::endl;
            created_files.push_back(file_path);
        } catch (const std::exception& e) {
            std::cerr << "Test Case " << (i + 1) << ": Failed. " << e.what() << std::endl;
        }
    }
    
    // Cleanup
    if (!created_files.empty()) {
        std::cout << "\nCleaning up temporary files...\n";
        for (const auto& path : created_files) {
            try {
                if (std::filesystem::remove(path)) {
                    std::cout << "Removed: " << path << std::endl;
                } else {
                    std::cerr << "Failed to remove: " << path << std::endl;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error removing " << path << ": " << e.what() << std::endl;
            }
        }
    }

    return 0;
}