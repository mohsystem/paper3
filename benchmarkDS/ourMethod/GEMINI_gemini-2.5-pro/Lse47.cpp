// Instructions:
// 1. This code requires the C++17 <filesystem> library.
// 2. It uses a single-header library `cpp-httplib`. Download `httplib.h` from
//    https://github.com/yhirose/cpp-httplib and place it in the same directory.
// 3. Compile with: g++ -std=c++17 -o server main.cpp -pthread
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "httplib.h"

// Define the base directory for images
const std::filesystem::path IMAGE_DIR = "images";

// Helper to get the MIME type from a file extension
std::string get_mime_type(const std::filesystem::path& path) {
    const auto ext = path.extension().string();
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".txt") return "text/plain";
    return "application/octet-stream";
}

// Function to handle image requests
void serve_image(const httplib::Request& req, httplib::Response& res, const std::filesystem::path& base_dir) {
    std::string requested_filename = req.matches[1];

    // Sanitize filename to prevent directory traversal.
    // .filename() extracts the final component, discarding directory info like ".." or "/".
    auto sanitized_filename = std::filesystem::path(requested_filename).filename();

    if (sanitized_filename.empty() || sanitized_filename.string() != requested_filename) {
        res.status = 400; // Bad Request
        res.set_content("Bad Request: Invalid filename.", "text/plain");
        return;
    }

    // Construct the full path securely.
    auto file_path = base_dir / sanitized_filename;

    // To prevent TOCTOU and securely resolve paths, we use `canonical`.
    // This resolves symlinks and normalizes the path.
    std::error_code ec;
    auto canonical_path = std::filesystem::canonical(file_path, ec);

    // If canonical fails (e.g., file not found), it's a 404.
    if (ec) {
        res.status = 404;
        res.set_content("Not Found", "text/plain");
        return;
    }
    
    // Security check: Ensure the final, resolved path is still inside our base directory.
    // We check if the canonical path string starts with the base directory path string.
    if (canonical_path.string().rfind(base_dir.string(), 0) != 0) {
        res.status = 403; // Forbidden
        res.set_content("Forbidden", "text/plain");
        return;
    }

    // Read the file and serve it
    std::ifstream ifs(canonical_path, std::ios::in | std::ios::binary);
    if (!ifs) {
        res.status = 500;
        res.set_content("Internal Server Error: Could not read file.", "text/plain");
        return;
    }
    
    std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    res.status = 200;
    res.set_content(body, get_mime_type(canonical_path).c_str());
}

void setup_test_environment() {
    if (!std::filesystem::exists(IMAGE_DIR)) {
        std::filesystem::create_directory(IMAGE_DIR);
    }
    std::ofstream(IMAGE_DIR / "test1.txt") << "This is a test file.";
    std::ofstream(IMAGE_DIR / "test2.jpg") << "This is a fake jpeg.";
}

int main() {
    setup_test_environment();

    httplib::Server svr;

    // Get the absolute path for our image directory to perform secure checks
    const auto absolute_image_path = std::filesystem::canonical(IMAGE_DIR);

    // Decorator equivalent: Map a URL pattern to a handler function.
    // R"(/images/(.+))" is a raw string literal for the regex.
    svr.Get(R"(/images/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        serve_image(req, res, absolute_image_path);
    });
    
    int port = 8081;
    std::cout << "Server starting on port " << port << "..." << std::endl;
    std::cout << "Test cases:" << std::endl;
    std::cout << "1. Valid file: http://localhost:8081/images/test1.txt" << std::endl;
    std::cout << "2. Valid file with extension: http://localhost:8081/images/test2.jpg" << std::endl;
    std::cout << "3. Non-existent file: http://localhost:8081/images/nonexistent.png" << std::endl;
    std::cout << "4. Path traversal attempt (should be 400): http://localhost:8081/images/../main.cpp" << std::endl;
    std::cout << "5. Path traversal with encoding (should be 400): http://localhost:8081/images/%2e%2e/main.cpp" << std::endl;

    svr.listen("0.0.0.0", port);

    return 0;
}