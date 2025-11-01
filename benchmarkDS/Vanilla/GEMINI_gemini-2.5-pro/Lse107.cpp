#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <iomanip>

// Note: This code requires C++17 or later for the <filesystem> library.
// On older GCC/Clang, you might need to link with -lstdc++fs or -lc++fs.
// Compile with: g++ -std=c++17 your_file.cpp -o your_executable

void manageSecretFile(const std::string& filename, const std::string& content) {
    std::cout << "Processing file: " << filename << std::endl;
    try {
        // 1. Create a file and write the string to it.
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not open file for writing.");
        }
        outFile << content;
        outFile.close();
        std::cout << "  - Successfully wrote to " << filename << std::endl;

        // 2. Change permissions to restrictive (owner read/write).
        namespace fs = std::filesystem;
        fs::permissions(filename, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::replace);
        std::cout << "  - Set permissions to rw------- (600)" << std::endl;

        // 3. Print the permissions of the file.
        fs::file_status status = fs::status(filename);
        fs::perms p = status.permissions();
        
        // Mask to get only the permission bits and print in octal format.
        auto perm_val = static_cast<unsigned int>(p) & 0777;
        std::cout << "  - Current permissions: 0" << std::oct << perm_val << std::dec << std::endl;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
    
    // Clean up the created file for the next test case
    try {
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
            std::cout << "  - Cleaned up file: " << filename << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
         // Ignore cleanup errors
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // Test Case 1
    manageSecretFile("secret.txt", content);

    // Test Case 2
    manageSecretFile("secret_file_2.txt", content);

    // Test Case 3
    manageSecretFile("another_secret.log", content);

    // Test Case 4
    manageSecretFile("temp_secret_4.dat", content);
    
    // Test Case 5
    manageSecretFile("last_secret.txt", content);
    
    return 0;
}