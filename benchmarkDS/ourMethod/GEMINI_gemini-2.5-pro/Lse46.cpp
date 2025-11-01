#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

// Instructions:
// 1. This code requires the 'httplib.h' single-file header library.
//    Download it from: https://github.com/yhirose/cpp-httplib
// 2. This code also requires a C++17 compatible compiler for std::filesystem.
// 3. Compile with: g++ -std=c++17 -o server main.cpp -lssl -lcrypto -pthread
// 4. Run: ./server

const std::string IMAGE_DIR_NAME = "images";

void setup_test_environment() {
    std::filesystem::path image_dir(IMAGE_DIR_NAME);
    if (!std::filesystem::exists(image_dir)) {
        std::filesystem::create_directory(image_dir);
    }
    std::ofstream test_file(image_dir / "test.txt");
    if (test_file.is_open()) {
        test_file << "This is a test file.";
        test_file.close();
    }
}

int main() {
    setup_test_environment();

    httplib::Server svr;

    // The Get method associates the URL path /img with this lambda function.
    svr.Get("/img", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("name")) {
            res.status = 400;
            res.set_content("Bad Request: 'name' parameter is missing.", "text/plain");
            return;
        }

        std::string name = req.get_param_value("name");
        if (name.empty()) {
            res.status = 400;
            res.set_content("Bad Request: 'name' parameter is empty.", "text/plain");
            return;
        }
        
        // Security: Basic sanitization of filename. It should not contain path traversal elements.
        if (name.find("..") != std::string::npos || name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
            res.status = 400;
            res.set_content("Bad Request: Invalid filename.", "text/plain");
            return;
        }
        
        try {
            const auto base_dir = std::filesystem::canonical(IMAGE_DIR_NAME);
            auto file_path = base_dir / name;

            // Security: Use weakly_canonical to resolve path without requiring file to exist.
            // This prevents certain TOCTOU attacks but still allows checking the path's structure.
            file_path = std::filesystem::weakly_canonical(file_path);

            // Security: After canonicalization, verify the path is still within the base directory.
            // We check this by seeing if base_dir is a prefix of file_path.
            auto base_dir_str = base_dir.string();
            auto file_path_str = file_path.string();
            if (file_path_str.rfind(base_dir_str, 0) != 0) {
                 res.status = 403;
                 res.set_content("Forbidden", "text/plain");
                 return;
            }

            // Security: Now check if it's a regular file (not a directory, symlink, etc.)
            if (!std::filesystem::is_regular_file(file_path)) {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
                return;
            }

            std::ifstream file(file_path, std::ios::binary);
            if (!file) {
                res.status = 500;
                res.set_content("Internal Server Error: Could not open file.", "text/plain");
                return;
            }

            std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(body, "application/octet-stream");

        } catch (const std::filesystem::filesystem_error& e) {
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    });

    std::cout << "Server started on port 8080" << std::endl;
    std::cout << "Run the following curl commands to test:" << std::endl;
    std::cout << "1. Test Case 1 (Valid): curl http://localhost:8080/img?name=test.txt" << std::endl;
    std::cout << "2. Test Case 2 (Not Found): curl http://localhost:8080/img?name=notfound.txt" << std::endl;
    std::cout << "3. Test Case 3 (Path Traversal): curl \"http://localhost:8080/img?name=../main.cpp\"" << std::endl;
    std::cout << "4. Test Case 4 (Missing Param): curl http://localhost:8080/img" << std::endl;
    std::cout << "5. Test Case 5 (Empty Param): curl http://localhost:8080/img?name=" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}