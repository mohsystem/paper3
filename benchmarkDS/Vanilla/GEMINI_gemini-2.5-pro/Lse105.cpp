#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // Requires C++17 or later

// To compile: g++ -std=c++17 your_file.cpp -o your_executable
namespace fs = std::filesystem;

/**
 * @brief Creates a file, writes content to it, and sets restrictive permissions.
 *
 * @param filename The name of the file to create.
 * @param content The string content to write to the file.
 */
void createAndSecureFile(const std::string& filename, const std::string& content) {
    try {
        // 1. Write the string to the file.
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
            return;
        }
        outFile << content;
        outFile.close();
        std::cout << "Successfully created and wrote to " << filename << std::endl;

        // 2. Change permissions to be restrictive (owner read/write only).
        fs::permissions(filename,
                        fs::perms::owner_read | fs::perms::owner_write,
                        fs::perm_options::replace);
        std::cout << "Set permissions for " << filename << " to restrictive (owner-read/write)." << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error with '" << filename << "': " << e.what() << std::endl;
    }
}

int main() {
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    
    // 5 test cases, creating 5 different files.
    for (int i = 1; i <= 5; ++i) {
        std::string filename = "secret" + std::to_string(i) + ".txt";
        std::cout << "\n--- Test Case " << i << ": Creating " << filename << " ---" << std::endl;
        createAndSecureFile(filename, content);
    }
    return 0;
}