/*
 * This code requires the 'httplib.h' single-file header-only C++ HTTP library.
 * Download it from: https://github.com/yhirose/cpp-httplib
 * Place the 'httplib.h' file in the same directory as this source file to compile.
 *
 * Compile with: g++ Lse46.cpp -o Lse46 -std=c++11 -pthread
 */
#include "httplib.h"
#include <iostream>
#include <fstream>
#include <string>

// C++ doesn't have decorators in the Python sense.
// Web framework libraries provide routing functions (e.g., svr.Get) to associate a URL with a handler function.

const char* IMAGE_DIR = "./images";

void setup_test_files() {
    // Create directory (platform-specific, this works on POSIX)
#if defined(_WIN32)
    _mkdir(IMAGE_DIR);
#else
    mkdir(IMAGE_DIR, 0755);
#endif

    std::ofstream("images/test1.txt") << "This is test file 1.";
    std::ofstream("images/image.png") << "This is a dummy png file content.";
}


// The handler function for the "/img" route
void img_handler(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("name")) {
        res.set_content("Bad Request: 'name' parameter is missing.", "text/plain");
        res.status = 400;
        return;
    }

    std::string file_name = req.get_param_value("name");

    // Security: Basic check to prevent directory traversal
    if (file_name.find("..") != std::string::npos) {
        res.set_content("Bad Request: Invalid file name.", "text/plain");
        res.status = 400;
        return;
    }

    std::string file_path = std::string(IMAGE_DIR) + "/" + file_name;

    std::ifstream file(file_path, std::ios::in | std::ios::binary);
    if (file) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.set_content(content, "application/octet-stream");
        res.status = 200;
    } else {
        res.set_content("Not Found: The requested file does not exist.", "text/plain");
        res.status = 404;
    }
}

class Lse46 {
public:
    static void run() {
        setup_test_files();
        
        httplib::Server svr;

        // Associate the URL /img with our handler function
        svr.Get("/img", img_handler);

        int port = 8080;
        std::cout << "C++ server started on port " << port << std::endl;
        std::cout << "Test cases:" << std::endl;
        std::cout << "1. Valid file: http://localhost:" << port << "/img?name=test1.txt" << std::endl;
        std::cout << "2. Another valid file: http://localhost:" << port << "/img?name=image.png" << std::endl;
        std::cout << "3. Non-existent file: http://localhost:" << port << "/img?name=notfound.txt" << std::endl;
        std::cout << "4. Missing parameter: http://localhost:" << port << "/img" << std::endl;
        std::cout << "5. Directory traversal attempt: http://localhost:" << port << "/img?name=../somefile.txt" << std::endl;

        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Lse46::run();
    return 0;
}