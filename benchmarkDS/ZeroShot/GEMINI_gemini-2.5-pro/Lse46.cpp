#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem> // C++17

// For cross-platform directory creation
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

namespace fs = std::filesystem;

/**
 * Securely retrieves an image file from a designated 'images' directory.
 * This function simulates a web endpoint handler for a URL like /img?name=...
 * It prevents path traversal attacks by validating the filename and ensuring
 * the resolved file path is within the intended directory.
 *
 * @param name The name of the file requested from the 'images' directory.
 * @return A vector of characters containing the file's content. The vector will be
 *         empty if the file is not found or if a security violation is detected.
 */
std::vector<char> img(const std::string& name) {
    // 1. Basic validation: reject names containing path separators.
    if (name.empty() || name.find('/') != std::string::npos || name.find('\\') != std::string::npos || name.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid characters or path traversal attempt in filename '" << name << "'." << std::endl;
        return {};
    }

    try {
        // 2. Define the secure base directory.
        fs::path base_dir = fs::absolute("images");
        if (!fs::exists(base_dir) || !fs::is_directory(base_dir)) {
            std::cerr << "Error: Base 'images' directory does not exist." << std::endl;
            return {};
        }

        // 3. Resolve the requested file path.
        fs::path requested_path = base_dir / name;
        
        // Use weakly_canonical to resolve symlinks and ".." etc.
        fs::path canonical_path = fs::weakly_canonical(requested_path);

        // 4. Security Check: Ensure the resolved path is within the base directory.
        std::string base_str = base_dir.string();
        std::string canonical_str = canonical_path.string();
        
        if (canonical_str.rfind(base_str, 0) != 0) {
            std::cerr << "Security Error: Path traversal detected for filename '" << name << "'." << std::endl;
            return {};
        }

        // 5. Check if the file exists and is a regular file.
        if (fs::exists(canonical_path) && fs::is_regular_file(canonical_path)) {
            std::ifstream file(canonical_path, std::ios::binary | std::ios::ate);
            if (!file) {
                 std::cerr << "Error: Could not open file: " << canonical_path << std::endl;
                 return {};
            }
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<char> buffer(size);
            if (file.read(buffer.data(), size)) {
                return buffer;
            }
        } else {
             std::cerr << "Error: File not found or is not a regular file: " << canonical_path << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return {};
}

// Main function with test cases
int main() {
    // Setup: Create a dummy directory and files for testing
    const std::string image_dir = "images";
    const std::string valid_image_path = image_dir + "/test.jpg";
    const std::string secret_file_path = "secret.txt";

    MKDIR(image_dir.c_str());
    std::ofstream(valid_image_path) << "jpeg_data";
    std::ofstream(secret_file_path) << "secret_data";
    std::cout << "Test environment created." << std::endl;

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    std::vector<std::string> test_cases = {
        "test.jpg",                     // 1. Valid case
        "../secret.txt",                // 2. Path traversal
        "nonexistent.png",              // 3. File not found
        "test.jpg/../test.jpg",         // 4. More complex invalid name
        "../../../../etc/passwd"        // 5. Classic path traversal attack
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        std::cout << "\nTest Case " << (i + 1) << ": Requesting '" << test_cases[i] << "'" << std::endl;
        std::vector<char> result = img(test_cases[i]);
        if (!result.empty()) {
            std::string content(result.begin(), result.end());
            std::cout << "Success: Returned " << result.size() << " bytes. Content: " << content << std::endl;
        } else {
            std::cout << "Failure: Request denied as expected." << std::endl;
        }
    }

    // Teardown: Clean up the created files and directory
    fs::remove(valid_image_path);
    fs::remove(secret_file_path);
    fs::remove(image_dir);
    std::cout << "\n--- Test environment cleaned up. ---" << std::endl;

    return 0;
}