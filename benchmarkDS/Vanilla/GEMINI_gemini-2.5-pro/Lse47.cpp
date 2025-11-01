#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

// Use a namespace to keep the code organized, similar to a class.
namespace Lse47 {

    const std::string IMAGE_DIR = "images";

    /**
     * @brief Simulates a web handler triggered by a URL.
     * 
     * It parses the URL, extracts the filename, and checks if it exists in the IMAGE_DIR.
     * This function mimics the behavior of a decorator-based route handler.
     * 
     * @param url The request URL, e.g., "/images/cat.jpg".
     * @return A string indicating the action to be taken.
     */
    std::string handleImageRequest(const std::string& url) {
        const std::string prefix = "/images/";
        if (url.rfind(prefix, 0) != 0) {
            return "HTTP 400 Bad Request: URL must start with " + prefix;
        }

        std::string filename = url.substr(prefix.length());

        // Security check for directory traversal or invalid characters
        if (filename.empty() || filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
            return "HTTP 400 Bad Request: Invalid filename.";
        }
        
        std::filesystem::path full_path = std::filesystem::path(IMAGE_DIR) / filename;

        if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
            return "HTTP 200 OK: Sending file '" + full_path.string() + "' to the browser.";
        } else {
            return "HTTP 404 Not Found: File '" + full_path.string() + "' does not exist.";
        }
    }

    /**
     * @brief Sets up a dummy 'images' directory with some files for testing.
     */
    void setupDummyFiles() {
        try {
            std::filesystem::create_directory(IMAGE_DIR);
            std::vector<std::string> files = {"cat.jpg", "dog.png", "bird.gif"};
            for (const auto& fname : files) {
                std::ofstream outfile(std::filesystem::path(IMAGE_DIR) / fname);
                outfile.close();
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error setting up dummy files: " << e.what() << std::endl;
        }
    }

} // namespace Lse47

int main() {
    // 1. Setup the environment
    Lse47::setupDummyFiles();

    // 2. Run test cases
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Requesting an existing file
    std::cout << "Test 1 (/images/cat.jpg):" << std::endl;
    std::cout << "  " << Lse47::handleImageRequest("/images/cat.jpg") << std::endl;

    // Test Case 2: Requesting another existing file
    std::cout << "Test 2 (/images/dog.png):" << std::endl;
    std::cout << "  " << Lse47::handleImageRequest("/images/dog.png") << std::endl;

    // Test Case 3: Requesting a non-existent file
    std::cout << "Test 3 (/images/lion.jpg):" << std::endl;
    std::cout << "  " << Lse47::handleImageRequest("/images/lion.jpg") << std::endl;

    // Test Case 4: Attempting directory traversal (should be blocked)
    std::cout << "Test 4 (/images/../somefile.txt):" << std::endl;
    std::cout << "  " << Lse47::handleImageRequest("/images/../somefile.txt") << std::endl;
    
    // Test Case 5: Requesting a malformed URL
    std::cout << "Test 5 (/other/cat.jpg):" << std::endl;
    std::cout << "  " << Lse47::handleImageRequest("/other/cat.jpg") << std::endl;

    std::cout << "\n--- Test Cases Finished ---" << std::endl;

    return 0;
}