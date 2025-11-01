#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

// C++17 <filesystem> provides a standard way to interact with the file system.
namespace fs = std::filesystem;

/**
 * Handles the upload of a file by saving it to the 'uploads' folder.
 * The 'uploads' folder is created in the same directory as the source file.
 * @param filename The name of the file to save.
 * @param content The string content to write into the file.
 * @return Boolean indicating success or failure.
 */
bool upload_file(const std::string& filename, const std::string& content) {
    std::string upload_folder = "uploads";
    try {
        // Ensure the uploads directory exists. create_directories is safe to call even if it exists.
        fs::create_directories(upload_folder);

        // Construct the full file path
        fs::path file_path = fs::path(upload_folder) / filename;

        // Save the file
        std::ofstream outfile(file_path);
        if (!outfile.is_open()) {
            std::cerr << "CPP Error: Could not open file for writing: " << file_path << std::endl;
            return false;
        }
        
        outfile << content;
        outfile.close();

        std::cout << "CPP: Successfully saved file to " << fs::absolute(file_path) << std::endl;
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "CPP Filesystem Error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    upload_file("cpp_test1.txt", "This is a test from C++.");
    upload_file("main.cpp", "#include <iostream>\nint main() { return 0; }");
    upload_file("CMakeLists.txt", "cmake_minimum_required(VERSION 3.10)");
    upload_file("user_data.xml", "<user><id>123</id></user>");
    upload_file("style.css", "body { color: #333; }");
    std::cout << "--------------------\n" << std::endl;
    return 0;
}