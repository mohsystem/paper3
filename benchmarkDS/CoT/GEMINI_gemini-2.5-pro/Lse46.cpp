// This code requires the C++17 standard and the cpp-httplib library.
// 1. Download "httplib.h" from https://github.com/yhirose/cpp-httplib
// 2. Place "httplib.h" in the same directory or in your include path.
// 3. Compile with: g++ -std=c++17 -o Lse46 Lse46.cpp -pthread

#include "httplib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

const std::string IMAGE_DIR = "images";

/**
 * The function associated with the /img URL.
 * It handles GET requests, reads the 'name' parameter, and serves the file.
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 */
void img(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("name")) {
        res.set_content("Missing 'name' parameter.", "text/plain");
        res.status = 400; // Bad Request
        return;
    }

    std::string fileName = req.get_param_value("name");
    
    // Security: Reject any paths that contain '..' as a basic, initial check.
    // The canonical path check below is the primary defense.
    if (fileName.find("..") != std::string::npos || fileName.find("/") != std::string::npos || fileName.find("\\") != std::string::npos) {
        res.set_content("File not found.", "text/plain");
        res.status = 404; // Not Found
        return;
    }

    try {
        // Get the absolute, canonical path of the base directory.
        const auto base_path = std::filesystem::canonical(IMAGE_DIR);
        
        // Construct the full path to the requested file.
        auto requested_path = base_path / fileName;
        
        // Security: The critical check. Verify that the requested file actually exists
        // and is located within the intended base directory. We do this by comparing
        // the prefix of the file's canonical path with the base directory's canonical path.
        if (!std::filesystem::exists(requested_path) || !std::filesystem::is_regular_file(requested_path)) {
            res.set_content("File not found.", "text/plain");
            res.status = 404;
            return;
        }

        // Double-check the path after ensuring it exists.
        auto canonical_req_path = std::filesystem::canonical(requested_path);
        if (canonical_req_path.string().rfind(base_path.string(), 0) != 0) {
             res.set_content("File not found (Traversal Attempt).", "text/plain");
             res.status = 404;
             return;
        }

        // If all checks pass, read and serve the file.
        std::ifstream file(canonical_req_path, std::ios::binary);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "application/octet-stream"); // Generic content type
            res.status = 200; // OK
        } else {
             res.set_content("Could not open file.", "text/plain");
             res.status = 500; // Internal Server Error
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // This exception is often thrown if the base directory or file doesn't exist.
        res.set_content("File not found or invalid path.", "text/plain");
        res.status = 404;
    } catch (const std::exception& e) {
        res.set_content("An internal error occurred.", "text/plain");
        res.status = 500;
    }
}

/**
 * Sets up a test directory and a sample file.
 */
void setupTestEnvironment() {
    std::cout << "Setting up C++ test environment..." << std::endl;
    if (!std::filesystem::exists(IMAGE_DIR)) {
        std::filesystem::create_directory(IMAGE_DIR);
        std::cout << "Created directory: " << IMAGE_DIR << std::endl;
    }

    std::ofstream testFile(std::filesystem::path(IMAGE_DIR) / "test.txt");
    if (testFile.is_open()) {
        testFile << "This is a test file from C++.";
        testFile.close();
        std::cout << "Created test file: images/test.txt" << std::endl;
    }
}

// In C++, the main class name "Lse46" is represented by the executable name.
int main(void) {
    setupTestEnvironment();
    httplib::Server svr;

    // The decorator equivalent: associating a URL path with a handler function.
    svr.Get("/img", img);

    int port = 8081;
    std::cout << "\n--- C++ httplib Server ---" << std::endl;
    std::cout << "Server starting on http://127.0.0.1:" << port << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    std::cout << "\n--- 5 Test Cases ---" << std::endl;
    std::cout << "1. Valid file:         http://127.0.0.1:" << port << "/img?name=test.txt" << std::endl;
    std::cout << "2. File not found:     http://127.0.0.1:" << port << "/img?name=nonexistent.txt" << std::endl;
    std::cout << "3. Path traversal 1:   http://127.0.0.1:" << port << "/img?name=../Lse46.cpp (should be 404)" << std::endl;
    std::cout << "4. Path traversal 2:   http://127.0.0.1:" << port << "/img?name=..%2ftest.txt (should be 404)" << std::endl;
    std::cout << "5. Missing parameter:  http://127.0.0.1:" << port << "/img (should be 400)" << std::endl;

    svr.listen("127.0.0.1", port);

    return 0;
}