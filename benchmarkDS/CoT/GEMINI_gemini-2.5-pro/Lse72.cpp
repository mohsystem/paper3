#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // Requires C++17 or later

namespace fs = std::filesystem;

/**
 * Handles the upload of a file by saving content to a specified filename
 * in an 'uploads' directory. It creates the directory if it doesn't exist
 * and sanitizes the filename to prevent path traversal attacks.
 *
 * @param filename The desired name of the file. Path information will be stripped.
 * @param content The string content to write to the file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool upload_file(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        std::cerr << "Error: Filename cannot be empty." << std::endl;
        return false;
    }

    // --- Security: Path Traversal Prevention ---
    // Use filesystem::path to safely extract only the filename part.
    fs::path input_path(filename);
    std::string sanitized_filename = input_path.filename().string();

    // After sanitization, check if the result is empty or represents a parent/current directory.
    if (sanitized_filename.empty() || sanitized_filename == "." || sanitized_filename == "..") {
        std::cerr << "Error: Invalid filename provided: " << filename << std::endl;
        return false;
    }

    const fs::path upload_dir = "uploads";

    // Create the directory if it doesn't exist.
    try {
        if (!fs::exists(upload_dir)) {
            fs::create_directory(upload_dir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory '" << upload_dir << "': " << e.what() << std::endl;
        return false;
    }
    
    fs::path destination_path = upload_dir / sanitized_filename;

    // --- Security: Final canonical path check ---
    // Ensure the final path is within the intended directory.
    try {
        fs::path canonical_dest = fs::weakly_canonical(destination_path);
        fs::path canonical_dir = fs::weakly_canonical(upload_dir);
        if (canonical_dest.string().rfind(canonical_dir.string(), 0) != 0) {
            std::cerr << "Error: Final path check failed. Attempted write outside of uploads directory." << std::endl;
            return false;
        }
    } catch(const fs::filesystem_error& e) {
        std::cerr << "Error resolving canonical path: " << e.what() << std::endl;
        return false;
    }

    // Write the content to the file.
    std::ofstream outfile(destination_path);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << destination_path << std::endl;
        return false;
    }

    outfile << content;
    outfile.close();

    std::cout << "Successfully saved file: " << destination_path << std::endl;
    return true;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid simple filename
    upload_file("test1.txt", "This is a valid test file.");

    // Test Case 2: Another valid filename
    upload_file("data.json", "{ \"key\": \"value\" }");

    // Test Case 3: Path traversal attempt (should fail)
    upload_file("../../root/.bashrc", "malicious content");

    // Test Case 4: Absolute path with traversal (should fail)
    upload_file("/etc/../boot/grub.cfg", "more malicious content");
    
    // Test Case 5: A Windows-style path (should be sanitized)
    upload_file("C:\\Users\\test\\Desktop\\secret.txt", "secret data");
    
    std::cout << "--------------------" << std::endl << std::endl;

    return 0;
}