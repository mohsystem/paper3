// This code requires the httplib.h single-file header library.
// You can get it from: https://github.com/yhirose/cpp-httplib
// Place httplib.h in the same directory or in your include path.
#define CPPHTTPLIB_OPENSSL_SUPPORT // If you want HTTPS support, requires OpenSSL libs
#include "httplib.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

const std::string UPLOAD_DIR = "uploads";
const long long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

// Function to generate a UUID (for unique filenames)
// A simple cross-platform way to get some randomness. Not a true UUID v4.
std::string generate_uuid() {
    static const char hex_chars[] = "0123456789abcdef";
    std::string uuid_str(36, ' ');
    srand(time(0));
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid_str[i] = '-';
        } else {
            uuid_str[i] = hex_chars[rand() % 16];
        }
    }
    return uuid_str;
}


int main() {
    // Create upload directory if it doesn't exist
    try {
        if (!fs::exists(UPLOAD_DIR)) {
            fs::create_directory(UPLOAD_DIR);
            std::cout << "Created directory: " << fs::absolute(UPLOAD_DIR) << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return 1;
    }

    httplib::Server svr;

    // Set a payload size limit to prevent DoS from very large requests
    svr.set_payload_max_length(MAX_FILE_SIZE + 1024 * 1024); // Allow some overhead for multipart boundaries

    // Serve the HTML form at the root
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            "<!DOCTYPE html><html><head><title>File Upload</title></head><body>"
            "<h2>Upload PDF File</h2>"
            "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
            "Select PDF file to upload:"
            "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\" accept=\".pdf\" required>"
            "<input type=\"submit\" value=\"Upload PDF\" name=\"submit\">"
            "</form></body></html>",
            "text/html");
    });

    // Handle file uploads at /upload
    svr.Post("/upload", [&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_file("fileToUpload")) {
            res.status = 400;
            res.set_content("Bad Request: 'fileToUpload' field is missing.", "text/plain");
            return;
        }

        const auto& file = req.get_file_value("fileToUpload");

        // --- Security: Check file size ---
        if (file.content.length() > MAX_FILE_SIZE) {
            res.status = 413; // Payload Too Large
            res.set_content("Payload Too Large: File size exceeds " + std::to_string(MAX_FILE_SIZE) + " bytes.", "text/plain");
            return;
        }

        if (file.content.empty()) {
            res.status = 400;
            res.set_content("Bad Request: Uploaded file is empty.", "text/plain");
            return;
        }

        // --- Security: Sanitize filename ---
        fs::path original_path(file.filename);
        std::string sanitized_basename = original_path.filename().string();

        // Check for invalid characters or empty filename
        if (sanitized_basename.empty() || std::string::npos != sanitized_basename.find("..")) {
            res.status = 400;
            res.set_content("Bad Request: Invalid filename.", "text/plain");
            return;
        }

        // Check file extension
        if (original_path.extension() != ".pdf") {
            res.status = 400;
            res.set_content("Bad Request: Invalid file type. Only PDF files are allowed.", "text/plain");
            return;
        }

        // Generate a unique filename to prevent overwrites and other attacks
        std::string unique_filename = generate_uuid() + "_" + sanitized_basename;

        // --- Security: Path Traversal Check ---
        fs::path base_path = fs::absolute(UPLOAD_DIR);
        fs::path destination_path = (base_path / unique_filename).lexically_normal();

        // Check if the resolved path is within the base directory
        auto [base_end, mismatch] = std::mismatch(base_path.begin(), base_path.end(), destination_path.begin());
        if(base_end != base_path.end()){
            res.status = 400;
            res.set_content("Bad Request: Invalid file path (Path Traversal attempt).", "text/plain");
            return;
        }

        // Save the file
        std::ofstream ofs(destination_path.string(), std::ios::binary);
        if (!ofs) {
            res.status = 500;
            res.set_content("Internal Server Error: Could not save file.", "text/plain");
            return;
        }
        ofs.write(file.content.c_str(), file.content.length());
        ofs.close();

        std::cout << "File saved: " << destination_path.string() << std::endl;
        res.set_content("File uploaded successfully!", "text/plain");
    });

    std::cout << "Server started on http://localhost:8080" << std::endl;
    std::cout << "Visit http://localhost:8080 to upload a file." << std::endl;

    // --- Test Cases ---
    std::cout << "\n--- To run test cases, execute the following commands in your terminal ---" << std::endl;
    std::cout << "1. Create some dummy files:" << std::endl;
    std::cout << "   echo \"%PDF-1.4...\" > test1.pdf" << std::endl;
    std::cout << "   echo \"This is text\" > not_a_pdf.txt" << std::endl;
    std::cout << "   echo \"../../etc/passwd\" > evil_name.pdf" << std::endl;
    std::cout << "   dd if=/dev/zero of=large_file.pdf bs=1M count=11" << std::endl;
    std::cout << "   touch empty.pdf" << std::endl;
    std::cout << "\n2. Run curl commands:" << std::endl;
    std::cout << "   # Test 1 (Success): Valid PDF" << std::endl;
    std::cout << "   curl -v -F \"fileToUpload=@test1.pdf\" http://localhost:8080/upload" << std::endl;
    std::cout << "   # Test 2 (Fail): Invalid file type" << std::endl;
    std::cout << "   curl -v -F \"fileToUpload=@not_a_pdf.txt\" http://localhost:8080/upload" << std::endl;
    std::cout << "   # Test 3 (Success, but sanitized): Malicious filename (server will strip path)" << std::endl;
    std::cout << "   curl -v -F \"fileToUpload=@evil_name.pdf;filename=../../evil.pdf\" http://localhost:8080/upload" << std::endl;
    std::cout << "   # Test 4 (Fail): File too large" << std::endl;
    std::cout << "   curl -v -F \"fileToUpload=@large_file.pdf\" http://localhost:8080/upload" << std::endl;
    std::cout << "   # Test 5 (Fail): Empty file" << std::endl;
    std::cout << "   curl -v -F \"fileToUpload=@empty.pdf\" http://localhost:8080/upload" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;

    svr.listen("0.0.0.0", 8080);

    return 0;
}