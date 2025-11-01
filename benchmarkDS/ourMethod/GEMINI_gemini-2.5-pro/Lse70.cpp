/*
 * This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory or in your include path.
 *
 * Compilation command:
 * g++ -std=c++17 -o server server.cpp -lpthread
 *
 * This example uses C++17 for std::filesystem, which is the most secure
 * way to handle paths. If C++17 is not available, a manual string-based
 * sanitize_filename function would be needed, but it is less robust.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't have OpenSSL for HTTPS
#include "httplib.h"

static const std::string UPLOADS_DIR = "uploads";
static constexpr size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

// Function to sanitize filename to prevent path traversal.
// Using std::filesystem::path is the recommended C++17 approach.
std::string sanitize_filename(const std::string& filename) {
    if (filename.empty()) {
        return "";
    }
    // This gets the filename part, stripping directory information.
    std::filesystem::path path(filename);
    std::string basename = path.filename().string();
    
    // Disallow filenames that could be directories like "." or ".."
    if (basename == "." || basename == "..") {
        return "";
    }
    return basename;
}


void upload_file(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_file("file")) {
        res.set_content("No file part in the request.", "text/plain");
        res.status = 400;
        return;
    }
    
    const auto& file = req.get_file_value("file");
    
    if (file.filename.empty()) {
        res.set_content("No file selected for upload.", "text/plain");
        res.status = 400;
        return;
    }

    // Check file size (in addition to the server-wide limit)
    if (file.content.length() > MAX_FILE_SIZE) {
        res.set_content("File is too large.", "text/plain");
        res.status = 413; // Payload Too Large
        return;
    }

    std::string sanitized_name = sanitize_filename(file.filename);
    if (sanitized_name.empty()) {
        res.set_content("Invalid filename provided.", "text/plain");
        res.status = 400;
        return;
    }

    std::filesystem::path dest_path = std::filesystem::path(UPLOADS_DIR) / sanitized_name;

    // Check if file already exists to prevent overwrite. This check is subject
    // to a TOCTOU race condition in a high-concurrency environment.
    // A better approach for POSIX is to use open() with O_CREAT|O_EXCL.
    // For this example, we use a standard fstream check.
    if (std::filesystem::exists(dest_path)) {
        res.set_content("File already exists.", "text/plain");
        res.status = 409; // Conflict
        return;
    }

    std::ofstream ofs(dest_path, std::ios::binary);
    if (!ofs) {
        res.set_content("Could not open file for writing.", "text/plain");
        res.status = 500;
        return;
    }

    ofs.write(file.content.data(), file.content.length());
    ofs.close();

    if (!ofs) {
        res.set_content("Error writing file to disk.", "text/plain");
        res.status = 500;
        // Attempt to clean up partially written file
        std::filesystem::remove(dest_path);
    } else {
        res.set_content("File uploaded successfully.", "text/plain");
        res.status = 200;
    }
}


int main() {
    // Create the uploads directory if it doesn't exist
    if (!std::filesystem::exists(UPLOADS_DIR)) {
        if (!std::filesystem::create_directory(UPLOADS_DIR)) {
            std::cerr << "Failed to create uploads directory." << std::endl;
            return 1;
        }
    }

    httplib::Server svr;

    // Set a server-wide maximum payload size to prevent DoS attacks
    svr.set_payload_max_length(MAX_FILE_SIZE);

    svr.Post("/upload", upload_file);

    std::cout << "Server starting on http://localhost:8080" << std::endl;
    std::cout << "Uploads will be saved to '" << UPLOADS_DIR << "' directory." << std::endl;
    
    std::cout << "\n--- How to Test with cURL ---" << std::endl;
    std::cout << "1. Create a test file: echo 'test content' > test.txt" << std::endl;
    std::cout << "2. Valid file upload: curl -F \"file=@test.txt\" http://localhost:8080/upload" << std::endl;
    std::cout << "3. File too large: create a file > 10MB and try to upload it." << std::endl;
    std::cout << "4. No file part: curl -X POST http://localhost:8080/upload" << std::endl;
    std::cout << "5. Path traversal attempt: curl -F \"file=@test.txt;filename=../../test.txt\" http://localhost:8080/upload" << std::endl;

    svr.listen("0.0.0.0", 8080);

    return 0;
}