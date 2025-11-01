#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // Requires C++17 and linking with -lstdc++fs or /std:c++17

namespace fs = std::filesystem;

/**
 * Creates a file, writes content, and sets restrictive permissions.
 * Note: This requires a C++17 compliant compiler and standard library.
 *
 * @param filename The name of the file to create.
 * @param content  The string content to write to the file.
 * @return true if the operation was successful, false otherwise.
 */
bool createAndSecureFile(const std::string& filename, const std::string& content) {
    // 1. Create and write to the file.
    // The ofstream is scoped to ensure it's closed before we change permissions.
    {
        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
            return false;
        }
        outfile << content;
    } // outfile is closed here
    std::cout << "Successfully wrote to " << filename << std::endl;

    // 2. Change permissions to be restrictive (owner read/write).
    try {
        fs::permissions(filename, 
                        fs::perms::owner_read | fs::perms::owner_write, 
                        fs::perm_options::replace);
        std::cout << "Set restrictive permissions for " << filename << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error setting permissions for " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // 5 Test Cases: Create 5 separate files to demonstrate the function.
    for (int i = 1; i <= 5; ++i) {
        std::string filename = "secret" + std::to_string(i) + ".txt";
        std::cout << "\n--- Test Case " << i << ": Creating " << filename << " ---" << std::endl;
        bool success = createAndSecureFile(filename, content);
        if (success) {
            std::cout << "Test Case " << i << " PASSED." << std::endl;
        } else {
            std::cout << "Test Case " << i << " FAILED." << std::endl;
        }
    }

    return 0;
}