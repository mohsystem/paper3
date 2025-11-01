/*
================================================================================
IMPORTANT: This code requires the 'cpp-httplib' library.
It is a header-only library. You can get it from:
https://github.com/yhirose/cpp-httplib

1. Download the 'httplib.h' file.
2. Place it in the same directory as this C++ source file, or in your include path.
3. Compile this file with C++11 or later, and link against pthread, ssl, and crypto.
   Example on Linux/macOS:
   g++ -std=c++11 -o Lse65 Lse65.cpp -lpthread -lssl -lcrypto
================================================================================
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// Conditional include to prevent massive error messages if the file is missing.
#if __has_include("httplib.h")
    #include "httplib.h"
#else
    #error "httplib.h not found. Please download it from https://github.com/yhirose/cpp-httplib and place it in the include path."
#endif

// Filesystem support for creating directories and files
#if __has_include(<filesystem>)
    #include <filesystem>
    namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #include <sys/stat.h> // For mkdir on POSIX systems
#endif

const std::string UPLOAD_DIR = "uploads";
const int PORT = 8082;
const std::string SERVER_HOST = "localhost";

// --- Server-side Logic ---
void start_server(httplib::Server& svr) {
    // Create uploads directory if it doesn't exist
    fs::create_directory(UPLOAD_DIR);

    svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_file("file")) {
            const auto& file = req.get_file_value("file");
            std::string filepath = UPLOAD_DIR + "/" + file.filename;
            
            std::ofstream ofs(filepath, std::ios::binary);
            if(ofs) {
                ofs.write(file.content.c_str(), file.content.length());
                ofs.close();
                
                std::string response_msg = "File '" + file.filename + "' uploaded successfully.";
                std::cout << "Server: " << response_msg << std::endl;
                res.set_content(response_msg, "text/plain");
                res.status = 200;
            } else {
                std::string error_msg = "Server Error: Could not save file " + file.filename;
                std::cerr << error_msg << std::endl;
                res.set_content(error_msg, "text/plain");
                res.status = 500;
            }
        } else {
            res.set_content("Bad Request: 'file' part is missing.", "text/plain");
            res.status = 400;
        }
    });

    std::cout << "Server starting on port " << PORT << std::endl;
    svr.listen(SERVER_HOST.c_str(), PORT);
}

// --- Client-side Logic ---
std::string upload_pdf_file(const std::string& file_path) {
    httplib::Client cli(SERVER_HOST, PORT);

    std::ifstream ifs(file_path, std::ios::in | std::ios::binary);
    if (!ifs) {
        return "Client Error: Could not open file " + file_path;
    }
    
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    httplib::MultipartFormDataItems items = {
        { "file", content, fs::path(file_path).filename().string(), "application/pdf" }
    };
    
    auto res = cli.Post("/upload", items);
    
    if (res) {
        return "Status: " + std::to_string(res->status) + ", Response: " + res->body;
    } else {
        auto err = res.error();
        return "Client HTTP Error: " + httplib::to_string(err);
    }
}

// --- Main function with test cases ---
class Lse65 {
public:
    static void main() {
        httplib::Server svr;

        // 1. Start Server in a separate thread
        std::thread server_thread(start_server, std::ref(svr));
        std::cout << "Server thread started. Waiting a moment for it to initialize..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::vector<std::string> test_files;
        
        try {
            // 2. Setup test directory and files
            fs::create_directory(UPLOAD_DIR);
            
            std::cout << "\n--- Running 5 Test Cases ---" << std::endl;
            for (int i = 1; i <= 5; ++i) {
                std::string file_name = "test_file_" + std::to_string(i) + ".pdf";
                test_files.push_back(file_name);
                std::ofstream ofs(file_name);
                ofs << "This is a dummy PDF file content for test " << i;
                ofs.close();

                std::cout << "\nTest Case " << i << ": Uploading " << file_name << std::endl;
                std::string result = upload_pdf_file(file_name);
                std::cout << "Client Result: " << result << std::endl;
                
                // Verify file exists in uploads directory
                if (fs::exists(fs::path(UPLOAD_DIR) / file_name)) {
                    std::cout << "Verification: SUCCESS - File found in '" << UPLOAD_DIR << "' folder." << std::endl;
                } else {
                    std::cout << "Verification: FAILED - File not found in '" << UPLOAD_DIR << "' folder." << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;
        }

        // 4. Stop Server
        std::cout << "\nShutting down server..." << std::endl;
        svr.stop();
        server_thread.join();
        std::cout << "Server stopped." << std::endl;
        
        // 5. Cleanup
        std::cout << "Cleaning up test files..." << std::endl;
        for(const auto& f : test_files) {
            fs::remove(f);
        }
    }
};

int main() {
    Lse65::main();
    return 0;
}