// This code requires the 'cpp-httplib' single-header library.
// For simplicity, to meet the "single source file" requirement, you can
// download httplib.h from https://github.com/yhirose/cpp-httplib and
// place it in the same directory as this file.
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

// You can get the latest version from https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
#include "httplib.h" 

const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

// Function to sanitize filename
std::string sanitize_filename(const std::string& filename) {
    // --- SECURITY: Path Traversal ---
    // Use std::filesystem to get only the filename part
    std::string basename = std::filesystem::path(filename).filename().string();
    
    // --- SECURITY: Invalid Characters ---
    // Replace any characters that are not alphanumeric, dot, underscore, or hyphen
    std::regex pattern("[^a-zA-Z0-9._-]");
    return std::regex_replace(basename, pattern, "");
}

int main() {
    // Create upload directory if it doesn't exist
    if (!std::filesystem::exists(UPLOAD_DIR)) {
        std::filesystem::create_directory(UPLOAD_DIR);
    }

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"(
            <html>
            <body>
            <h2>Upload a PDF File</h2>
            <form action="/upload" method="post" enctype="multipart/form-data">
              Select a PDF to upload:
              <input type="file" name="pdfFile" id="pdfFile" accept=".pdf">
              <input type="submit" value="Upload PDF" name="submit">
            </form>
            </body>
            </html>
        )", "text/html");
    });

    svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_file("pdfFile")) {
            res.status = 400;
            res.set_content("Bad Request: 'pdfFile' field not found.", "text/plain");
            return;
        }

        const auto& file = req.get_file_value("pdfFile");

        // --- SECURITY: File Size Check ---
        if (file.content.length() > MAX_FILE_SIZE) {
            res.status = 413;
            res.set_content("File too large. Max size is " + std::to_string(MAX_FILE_SIZE / (1024*1024)) + "MB", "text/plain");
            return;
        }

        std::string safe_filename = sanitize_filename(file.filename);

        if (safe_filename.empty()) {
            res.status = 400;
            res.set_content("Bad Request: Invalid or empty filename after sanitization.", "text/plain");
            return;
        }

        // --- SECURITY: File Type Check (filename extension) ---
        std::string lower_filename = safe_filename;
        std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);
        if (lower_filename.rfind(".pdf") != lower_filename.length() - 4) {
             res.status = 400;
             res.set_content("Invalid file type. Only PDF files are allowed.", "text/plain");
             return;
        }
        
        // --- SECURITY: File Type Check (Content-Type header) ---
        if(file.content_type != "application/pdf"){
            res.status = 400;
            res.set_content("Invalid Content-Type. Expected 'application/pdf'.", "text/plain");
            return;
        }

        std::filesystem::path file_path = std::filesystem::path(UPLOAD_DIR) / safe_filename;
        std::ofstream ofs(file_path.string(), std::ios::binary);
        if (!ofs) {
            res.status = 500;
            res.set_content("Internal Server Error: Could not open file for writing.", "text/plain");
            return;
        }
        
        ofs.write(file.content.c_str(), file.content.length());
        ofs.close();

        res.set_content("File '" + safe_filename + "' uploaded successfully.", "text/plain");
    });

    int port = 8000;
    std::cout << "Server started on http://localhost:" << port << std::endl;
    std::cout << "Upload directory is: " << UPLOAD_DIR << std::endl;
    
    std::cout << "\n--- How to Test (requires httplib.h) ---" << std::endl;
    std::cout << "1. Download httplib.h and place it in the same directory." << std::endl;
    std::cout << "2. Compile and run this C++ file." << std::endl;
    std::cout << "3. Open http://localhost:8000 in your web browser." << std::endl;
    std::cout << "Test Case 1: Upload a valid PDF file smaller than 10MB. It should succeed." << std::endl;
    std::cout << "Test Case 2: Upload a non-PDF file (e.g., .txt). It should be rejected." << std::endl;
    std::cout << "Test Case 3: Upload a file larger than 10MB. It should be rejected." << std::endl;
    std::cout << "Test Case 4: Upload a file with name '../test.pdf'. It should be saved as 'test.pdf'." << std::endl;
    std::cout << "Test Case 5: Refresh the root page to ensure the form is still served." << std::endl;

    svr.listen("localhost", port);
    
    return 0;
}