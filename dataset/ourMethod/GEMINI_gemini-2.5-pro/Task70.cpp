#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

/**
 * @brief Reads the content of a file, ensuring the path is safe.
 *
 * @param filePath The path to the file.
 * @return std::string The content of the file, or an error message on failure.
 */
std::string readFileContent(const std::string& filePath) {
    if (filePath.empty()) {
        return "Error: File path is empty.";
    }

    try {
        fs::path path(filePath);

        // Rule #7: Use status to check file type without following symlinks.
        auto status = fs::symlink_status(path);

        if (fs::is_symlink(status)) {
            return "Error: Symbolic links are not allowed.";
        }
        if (!fs::is_regular_file(status)) {
            if (!fs::exists(status)) {
                return "Error: File does not exist.";
            }
            return "Error: Path does not point to a regular file.";
        }
        
        // Rule #7: Prevent path traversal attacks.
        fs::path canonical_path = fs::canonical(path);
        fs::path base_dir = fs::current_path();

        // Check if the canonical path is within the base directory.
        std::string canonical_path_str = canonical_path.string();
        std::string base_dir_str = fs::canonical(base_dir).string();
        if (canonical_path_str.rfind(base_dir_str, 0) != 0) {
            return "Error: Path traversal attempt detected.";
        }

        // Rule #1, #8: Safely read the file using RAII for resource management.
        std::ifstream file(canonical_path);
        if (!file.is_open()) {
            return "Error: Could not open file for reading.";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();

    } catch (const fs::filesystem_error& e) {
        return "Error: " + std::string(e.what());
    } catch (const std::exception& e) {
        return "An unexpected error occurred: " + std::string(e.what());
    }
}

void runTests() {
    std::cout << "Running test cases..." << std::endl;
    fs::path test_dir = "cpp_test_files";
    fs::path safe_file = test_dir / "safe_file.txt";
    fs::path sub_dir = test_dir / "sub";
    fs::path symlink_file = test_dir / "slink.txt";

    try {
        // Setup test environment
        fs::create_directories(sub_dir);
        {
            std::ofstream ofs(safe_file);
            ofs << "This is a safe file.";
        }
        fs::create_symlink(safe_file.filename(), symlink_file);

        // Test Case 1: Read a valid file
        std::cout << "\n--- Test Case 1: Valid file ---" << std::endl;
        std::cout << "Reading: " << safe_file.string() << std::endl;
        std::cout << "Content: " << readFileContent(safe_file.string()) << std::endl;
        
        // Test Case 2: Read a non-existent file
        std::cout << "\n--- Test Case 2: Non-existent file ---" << std::endl;
        std::string non_existent_path = (test_dir / "nonexistent.txt").string();
        std::cout << "Reading: " << non_existent_path << std::endl;
        std::cout << "Content: " << readFileContent(non_existent_path) << std::endl;
        
        // Test Case 3: Attempt to read a directory
        std::cout << "\n--- Test Case 3: Directory path ---" << std::endl;
        std::cout << "Reading: " << sub_dir.string() << std::endl;
        std::cout << "Content: " << readFileContent(sub_dir.string()) << std::endl;
        
        // Test Case 4: Attempt to read a symbolic link
        std::cout << "\n--- Test Case 4: Symbolic link ---" << std::endl;
        std::cout << "Reading: " << symlink_file.string() << std::endl;
        std::cout << "Content: " << readFileContent(symlink_file.string()) << std::endl;

        // Test Case 5: Path traversal attempt
        std::cout << "\n--- Test Case 5: Path traversal ---" << std::endl;
        std::string traversal_path = "..";
        std::cout << "Reading: " << traversal_path << std::endl;
        std::cout << "Content: " << readFileContent(traversal_path) << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Test setup failed: " << e.what() << std::endl;
    }

    // Cleanup
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    if (ec) {
        std::cerr << "Test cleanup failed: " << ec.message() << std::endl;
    } else {
        std::cout << "\nTest cleanup complete." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Process file path from command line argument
        std::cout << readFileContent(argv[1]) << std::endl;
    } else {
        // Run test cases if no arguments are provided
        runTests();
    }
    return 0;
}