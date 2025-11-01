// To compile: g++ -std=c++17 -o Lse47 Lse47.cpp -pthread
// Requires httplib.h, download from https://github.com/yhirose/cpp-httplib
// Place httplib.h in the same directory or in your include path.

#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support if needed
#include "httplib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// A helper function to read a file into a string
bool read_file(const fs::path& path, std::string& out) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        return false;
    }
    ifs.seekg(0, std::ios::end);
    out.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(&out[0], out.size());
    return true;
}

// A helper function to determine content type from file extension
std::string get_content_type(const fs::path& path) {
    const auto ext = path.extension().string();
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".txt") return "text/plain";
    return "application/octet-stream";
}

void setup_test_files() {
    try {
        if (!fs::exists("images")) {
            fs::create_directory("images");
        }
        std::ofstream png_file("images/test1.png", std::ios::binary);
        png_file.write("\x89PNG\r\n\x1a\n", 8);
        png_file.close();

        std::ofstream txt_file("images/safe.txt");
        txt_file << "This is a safe file.";
        txt_file.close();
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during setup: " << e.what() << std::endl;
    }
}


int main() {
    // A class in C++ doesn't have a static main method in the same way as Java.
    // The main function serves as the entry point for the application.
    // We name the source file Lse47.cpp to match the convention.
    
    httplib::Server svr;

    setup_test_files();

    const fs::path base_dir = fs::absolute("images");

    // This simulates a decorator. The server is configured to call this lambda
    // function when a GET request matches the pattern "/images/(.*)".
    svr.Get("/images/(.*)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string filename = req.matches[1];

        // --- Security Check 1: Basic filename validation ---
        // httplib decodes the URL, but we double-check for ".."
        if (filename.find("..") != std::string::npos) {
            res.status = 400; // Bad Request
            res.set_content("Invalid filename.", "text/plain");
            return;
        }

        fs::path requested_path = (base_dir / filename).lexically_normal();

        // --- Security Check 2: Path canonicalization and boundary check ---
        // Ensure the final path is within our intended 'images' directory.
        // We check if the 'base_dir' is a prefix of the 'requested_path'.
        auto [base_end, req_end] = std::mismatch(base_dir.begin(), base_dir.end(), requested_path.begin());
        if (base_end != base_dir.end()) {
             res.status = 403; // Forbidden
             res.set_content("Access denied.", "text/plain");
             return;
        }

        if (fs::exists(requested_path) && fs::is_regular_file(requested_path)) {
            std::string body;
            if (read_file(requested_path, body)) {
                res.status = 200;
                res.set_content(body, get_content_type(requested_path).c_str());
            } else {
                res.status = 500;
                res.set_content("Internal Server Error: Could not read file.", "text/plain");
            }
        } else {
            res.status = 404;
            res.set_content("File not found.", "text/plain");
        }
    });

    int port = 8081;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "--- Test Cases (run in another terminal) ---" << std::endl;
    std::cout << "1) Access a valid file: curl http://localhost:" << port << "/images/test1.png --output test1_out.png" << std::endl;
    std::cout << "2) Access another valid file: curl http://localhost:" << port << "/images/safe.txt" << std::endl;
    std::cout << "3) Access a non-existent file: curl -i http://localhost:" << port << "/images/nonexistent.jpg" << std::endl;
    std::cout << "4) Attempt path traversal (should fail): curl -i http://localhost:" << port << "/images/../Lse47.cpp" << std::endl;
    std::cout << "5) Attempt another path traversal: curl -i http://localhost:" << port << "/images/..%2fLse47.cpp" << std::endl;
    
    svr.listen("0.0.0.0", port);

    return 0;
}