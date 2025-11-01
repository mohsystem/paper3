/*
 * PREREQUISITE: This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory or in your include path.
 *
 * COMPILATION:
 * On Linux/macOS: g++ -std=c++17 -o server Lse70.cpp -pthread
 * On Windows (MSVC): cl /std:c++17 /EHsc Lse70.cpp /I .
 *
 * This example also uses C++17 <filesystem>. With older GCC/Clang,
 * you might need to link with -lstdc++fs.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // C++17 for path manipulation and directory creation
#include "httplib.h"

// Configuration
const std::string UPLOAD_DIRECTORY = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {".txt", ".pdf", ".png", ".jpg", ".jpeg"};

// Function to check for allowed file extensions
bool is_extension_allowed(const std::string& filename) {
    std::filesystem::path p(filename);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    for (const auto& allowed_ext : ALLOWED_EXTENSIONS) {
        if (ext == allowed_ext) {
            return true;
        }
    }
    return false;
}

// The function that handles the file upload
void upload_file(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_file("file")) {
        res.set_content("No file part in the request", "text/plain");
        res.status = 400;
        return;
    }

    const auto& file = req.get_file_value("file");

    // 1. Check file size
    if (file.content.length() > MAX_FILE_SIZE) {
        res.set_content("File size exceeds the limit of " + std::to_string(MAX_FILE_SIZE) + " bytes", "text/plain");
        res.status = 413; // Payload Too Large
        return;
    }

    // 2. Sanitize filename to prevent path traversal
    // std::filesystem::path(...).filename() securely extracts the filename part.
    std::filesystem::path original_path(file.filename);
    std::string sanitized_filename = original_path.filename().string();

    if (sanitized_filename.empty()) {
        res.set_content("Invalid filename", "text/plain");
        res.status = 400;
        return;
    }

    // 3. Validate file extension
    if (!is_extension_allowed(sanitized_filename)) {
        res.set_content("File type not allowed", "text/plain");
        res.status = 400;
        return;
    }

    // Create the uploads directory if it doesn't exist
    std::error_code ec;
    if (!std::filesystem::exists(UPLOAD_DIRECTORY) && !std::filesystem::create_directory(UPLOAD_DIRECTORY, ec)) {
        res.set_content("Server error: Could not create upload directory.", "text/plain");
        res.status = 500;
        return;
    }

    // 4. Construct the final path and save the file
    std::filesystem::path save_path = std::filesystem::path(UPLOAD_DIRECTORY) / sanitized_filename;

    std::ofstream ofs(save_path.string(), std::ios::binary);
    if (!ofs) {
        res.set_content("Server error: Could not save file.", "text/plain");
        res.status = 500;
        return;
    }
    
    ofs.write(file.content.c_str(), file.content.length());
    ofs.close();

    std::cout << "Successfully saved file to: " << save_path.string() << std::endl;
    res.set_content("File '" + sanitized_filename + "' uploaded successfully!", "text/plain");
    res.status = 200;
}

// Main class Lse70 is not a C++ construct, but we can wrap main logic
class Lse70 {
public:
    static int main() {
        httplib::Server svr;

        // Serve the HTML upload form
        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(
                "<html><body>"
                "<h1>C++ File Upload</h1>"
                "<form action='/upload' method='post' enctype='multipart/form-data'>"
                "<input type='file' name='file'>"
                "<input type='submit' value='Upload'>"
                "</form>"
                "</body></html>",
                "text/html"
            );
        });

        // Endpoint for handling file uploads
        svr.Post("/upload", upload_file);

        std::cout << "Server starting on http://localhost:8080" << std::endl;
        
        std::cout << "\n--- Test Cases (use curl from a separate terminal) ---" << std::endl;
        std::cout << "1. Valid PNG upload: \n   (create a file 'test.png')\n   curl -X POST -F 'file=@test.png' http://localhost:8080/upload" << std::endl;
        std::cout << "2. Invalid file type (e.g., .html): \n   (create a file 'test.html')\n   curl -X POST -F 'file=@test.html' http://localhost:8080/upload" << std::endl;
        std::cout << "3. Malicious filename (path traversal): \n   (create a file 'test.txt')\n   curl -X POST -F 'file=@test.txt;filename=../../malicious.txt' http://localhost:8080/upload" << std::endl;
        std::cout << "4. No file part: \n   curl -X POST -F 'otherdata=value' http://localhost:8080/upload" << std::endl;
        std::cout << "5. Large file (requires creating a >10MB file 'largefile.txt'): \n   curl -X POST -F 'file=@largefile.txt' http://localhost:8080/upload" << std::endl;
        
        svr.listen("0.0.0.0", 8080);
        return 0;
    }
};

int main() {
    return Lse70::main();
}