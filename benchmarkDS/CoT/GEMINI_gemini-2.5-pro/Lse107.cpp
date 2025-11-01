#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // C++17 required
#include <sys/stat.h> // For a more detailed POSIX permission string

// Note: This code requires C++17 or later for the <filesystem> library.
// To compile: g++ -std=c++17 your_source_file.cpp -o your_executable -lstdc++fs

namespace fs = std::filesystem;

/**
 * Converts file permissions to a human-readable string (e.g., "rw-r--r--").
 * @param p The file permissions.
 * @return A string representation of the permissions.
 */
std::string permissions_to_string(fs::perms p) {
    std::string s;
    s += ((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-");
    s += ((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-");
    s += ((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-");
    s += ((p & fs::perms::group_read) != fs::perms::none ? "r" : "-");
    s += ((p & fs::perms::group_write) != fs::perms::none ? "w" : "-");
    s += ((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-");
    s += ((p & fs::perms::others_read) != fs::perms::none ? "r" : "-");
    s += ((p & fs::perms::others_write) != fs::perms::none ? "w" : "-");
    s += ((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-");
    return s;
}

/**
 * Creates a file, writes content, sets restrictive permissions, and prints them.
 * @param filename The name of the file to create.
 * @param content The content to write to the file.
 */
void handleSecretFile(const std::string& filename, const std::string& content) {
    std::cout << "Processing file: " << filename << std::endl;

    // 1. Create a file and write the string to it.
    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "  - Error: Could not open file for writing: " << filename << std::endl;
        return;
    }
    outfile << content;
    outfile.close();
    std::cout << "  - Successfully wrote to " << filename << std::endl;

    // 2. Change the permissions to restrictive (owner read/write only).
    try {
        fs::permissions(filename, fs::perms::owner_read | fs::perms::owner_write);
        std::cout << "  - Set permissions to rw-------" << std::endl;

        // 3. Print the permissions of the file.
        fs::perms p = fs::status(filename).permissions();
        std::cout << "  - Final permissions: " << permissions_to_string(p) << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "  - Error setting/getting permissions: " << e.what() << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
}

int main() {
    const std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // 5 Test Cases
    handleSecretFile("secret1.txt", content);
    handleSecretFile("secret2.txt", content);
    handleSecretFile("secret3.txt", content);
    handleSecretFile("secret4.txt", content);
    handleSecretFile("secret5.txt", content);

    return 0;
}