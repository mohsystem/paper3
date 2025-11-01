// Note: This code requires C++17 or later.
// On Linux/macOS, compile with: g++ this_file.cpp -o app -std=c++17 -lstdc++fs
// On Windows (MSVC), compile with: cl /std:c++17 this_file.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

const std::string CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

// Helper function to print file permissions in a human-readable format
void print_permissions(fs::perms p) {
    std::cout << ((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-")
              << ((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-")
              << ((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-")
              << ((p & fs::perms::group_read) != fs::perms::none ? "r" : "-")
              << ((p & fs::perms::group_write) != fs::perms::none ? "w" : "-")
              << ((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-")
              << ((p & fs::perms::others_read) != fs::perms::none ? "r" : "-")
              << ((p & fs::perms::others_write) != fs::perms::none ? "w" : "-")
              << ((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-");
}

void manage_secret_file(const std::string& filename) {
    fs::path file_path(filename);
    std::cout << "Processing file: " << filename << std::endl;

    try {
        // 1. Create the file and write the content
        // The ofstream destructor handles closing the file via RAII
        {
            std::ofstream secret_file(file_path);
            if (!secret_file.is_open()) {
                std::cerr << "Error: Could not create or open file " << filename << std::endl;
                return;
            }
            secret_file << CONTENT;
        } 
        std::cout << "File created and content written." << std::endl;

        // 2. Change file permissions to be restrictive (owner read/write only)
        fs::permissions(file_path, 
                        fs::perms::owner_read | fs::perms::owner_write, 
                        fs::perm_options::replace);
        std::cout << "Permissions set to restrictive (owner read/write)." << std::endl;

        // 3. Print the new permissions
        auto current_perms = fs::status(file_path).permissions();
        std::cout << "Current file permissions: ";
        print_permissions(current_perms);
        std::cout << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    std::cout << "---" << std::endl;
}

void cleanup(const std::string& filename) {
    try {
        fs::path file_path(filename);
        if (fs::exists(file_path)) {
            fs::remove(file_path);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error cleaning up file " << filename << ": " << e.what() << std::endl;
    }
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::string test_filename = "secret_cpp_" + std::to_string(i) + ".txt";
        try {
            manage_secret_file(test_filename);
        } catch (...) {
            std::cerr << "Test case " << i << " failed with an unknown exception for " << test_filename << std::endl;
        }
        cleanup(test_filename);
    }
    return 0;
}