/*
 * IMPORTANT: This C++ code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib and place it
 * in the same directory as this source file.
 *
 * COMPILE with: g++ Lse71.cpp -o Lse71 -I . -lpthread -std=c++11
 */
#include <iostream>
#include <fstream>
#include <string>

// Include the header-only library
#include "httplib.h"

#define UPLOAD_DIR "./uploads"

class Lse71 {
public:
    void run() {
        // Create upload directory if it doesn't exist (platform-specific)
#ifdef _WIN32
        _mkdir(UPLOAD_DIR);
#else
        mkdir(UPLOAD_DIR, 0777); // For Linux/macOS
#endif

        httplib::Server svr;

        // Serve the main HTML upload form
        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            std::string html_content = R"(
                <!DOCTYPE html>
                <html>
                <head><title>File Upload</title></head>
                <body>
                <h1>Upload a File</h1>
                <form action="/upload" method="post" enctype="multipart/form-data">
                    Select file to upload:
                    <input type="file" name="file" id="file">
                    <input type="submit" value="Upload File" name="submit">
                </form>
                </body>
                </html>
            )";
            res.set_content(html_content, "text/html");
        });

        // Handle file upload
        svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_file("file")) {
                const auto& file = req.get_file_value("file");
                std::string filepath = std::string(UPLOAD_DIR) + "/" + file.filename;

                std::ofstream ofs(filepath, std::ios::binary);
                if (ofs.is_open()) {
                    ofs.write(file.content.c_str(), file.content.length());
                    ofs.close();
                    std::string success_msg = "File " + file.filename + " uploaded successfully to " + UPLOAD_DIR + " directory.";
                    res.set_content(success_msg, "text/plain");
                } else {
                    res.status = 500;
                    res.set_content("Failed to open file for writing on server.", "text/plain");
                }
            } else {
                res.status = 400;
                res.set_content("Bad Request: 'file' field not found.", "text/plain");
            }
        });
        
        int port = 8080;
        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Visit http://localhost:" << port << " to upload a file." << std::endl;
        std::cout << "Or use curl for testing from your terminal:" << std::endl;
        std::cout << "-------------------- TEST CASES --------------------" << std::endl;
        std::cout << "1. Create a test file: echo \"Test file 1 content\" > test1.txt" << std::endl;
        std::cout << "2. Upload test1.txt: curl -F \"file=@test1.txt\" http://localhost:" << port << "/upload" << std::endl;
        std::cout << "3. Create another file: echo \"This is the second file.\" > test2.txt" << std::endl;
        std::cout << "4. Upload test2.txt: curl -F \"file=@test2.txt\" http://localhost:" << port << "/upload" << std::endl;
        std::cout << "5. Try a GET request (will show a 404 error from library): curl http://localhost:" << port << "/upload" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Lse71 server_app;
    server_app.run();
    return 0;
}