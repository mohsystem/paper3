#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

// This code requires the 'cpp-httplib' single-header library.
// Download "httplib.h" from https://github.com/yhirose/cpp-httplib
// and place it in the same directory as this source file.
#include "httplib.h"

namespace fs = std::filesystem;

/**
 * @brief The function that handles the file upload.
 * 
 * In cpp-httplib, this logic is typically inside a lambda or function
 * passed to the server's routing method.
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 */
void upload_file(const httplib::Request& req, httplib::Response& res) {
    const std::string UPLOAD_DIR = "uploads";

    // Create upload directory if it doesn't exist
    fs::create_directories(UPLOAD_DIR);

    if (req.has_file("file")) {
        const auto& file = req.get_file_value("file");
        // For security, you should sanitize the filename in a real application.
        const std::string filepath = UPLOAD_DIR + "/" + file.filename;

        std::ofstream ofs(filepath, std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(file.content.c_str(), file.content.length());
            ofs.close();
            res.set_content("File " + file.filename + " uploaded successfully!", "text/plain");
        } else {
            res.status = 500;
            res.set_content("Error: Could not save the file.", "text/plain");
        }
    } else {
        res.status = 400;
        res.set_content("Bad Request: 'file' part not found.", "text/plain");
    }
}

// In C++, the main class name 'Lse70' is not required for this type of application.
// A main function is used as the entry point.
int main() {
    httplib::Server svr;

    // Serve the HTML form for the root URL
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"(
            <!doctype html>
            <title>C++ File Upload</title>
            <h1>Upload a File</h1>
            <form method=post action="/upload" enctype=multipart/form-data>
              <input type=file name=file>
              <input type=submit value=Upload>
            </form>
        )", "text/html");
    });

    // Set the handler for the /upload URL
    svr.Post("/upload", upload_file);

    std::cout << "C++ server started on http://localhost:8080" << std::endl;
    std::cout << "Visit http://localhost:8080 to upload a file." << std::endl;
    std::cout << "\n--- 5 Test Cases (using cURL) ---" << std::endl;
    std::cout << "NOTE: Ensure you have a file (e.g., test1.txt) in your current directory before running these commands." << std::endl;
    std::cout << "1. Create a file for testing: echo 'hello cpp' > test1.txt" << std::endl;
    std::cout << "2. Test Case 1 (text file): curl -X POST -F 'file=@test1.txt' http://localhost:8080/upload" << std::endl;
    std::cout << "3. Create another file: echo 'int main() { return 0; }' > test2.cpp" << std::endl;
    std::cout << "4. Test Case 2 (cpp file): curl -X POST -F 'file=@test2.cpp' http://localhost:8080/upload" << std::endl;
    std::cout << "5. Test Case 3 (POST with no file part): curl -X POST http://localhost:8080/upload" << std::endl;
    std::cout << "6. Test Case 4 (POST with empty file part): curl -X POST -F 'file=' http://localhost:8080/upload" << std::endl;
    std::cout << "7. Test Case 5 (POST with wrong field name): curl -X POST -F 'data=@test1.txt' http://localhost:8080/upload\n" << std::endl;
    
    svr.listen("0.0.0.0", 8080);

    return 0;
}