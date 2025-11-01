#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <random>
#include <set>
#include "httplib.h"

// --- Configuration ---
const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::set<std::string> ALLOWED_EXTENSIONS = {".txt", ".pdf", ".png", ".jpg", ".jpeg", ".gif"};

namespace fs = std::filesystem;

// --- Helper Functions ---
std::string get_file_extension(const std::string& filename) {
    fs::path path(filename);
    return path.has_extension() ? path.extension().string() : "";
}

bool is_extension_allowed(const std::string& filename) {
    std::string ext = get_file_extension(filename);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ALLOWED_EXTENSIONS.count(ext) > 0;
}

std::string generate_random_string(size_t length) {
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<size_t> distribution(0, chars.size() - 1);
    std::string random_string(length, '\0');
    for (size_t i = 0; i < length; ++i) {
        random_string[i] = chars[distribution(generator)];
    }
    return random_string;
}

// --- Main Server Logic ---
void handle_upload(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_file("file")) {
        res.set_content("No file part in the request", "text/plain");
        res.status = 400;
        return;
    }

    const auto& file = req.get_file_value("file");

    // 1. Sanitize filename to prevent path traversal.
    // We only use it to get the extension. The final name is generated.
    fs::path original_path(file.filename);
    std::string sanitized_filename = original_path.filename().string();
    
    if (sanitized_filename.empty() || sanitized_filename == "." || sanitized_filename == "..") {
        res.set_content("Invalid filename", "text/plain");
        res.status = 400;
        return;
    }

    // 2. Validate file type
    if (!is_extension_allowed(sanitized_filename)) {
        res.set_content("File type not allowed", "text/plain");
        res.status = 400;
        return;
    }

    // 3. Generate a unique and safe filename
    std::string extension = get_file_extension(sanitized_filename);
    std::string unique_filename = generate_random_string(16) + extension;
    fs::path final_path = fs::path(UPLOAD_DIR) / unique_filename;
    
    // 4. Write to a temporary file and then atomically rename it
    fs::path temp_path = fs::path(UPLOAD_DIR) / (unique_filename + ".tmp");
    
    try {
        // Write content to a temporary file
        std::ofstream temp_file(temp_path, std::ios::binary);
        if (!temp_file) {
            throw std::runtime_error("Could not open temporary file for writing.");
        }
        temp_file.write(file.content.c_str(), file.content.length());
        temp_file.close();
        if (!temp_file) { // Check for write errors
            throw std::runtime_error("Error writing to temporary file.");
        }
        
        // Atomically rename the file
        fs::rename(temp_path, final_path);
        
        // Set restrictive permissions (owner read/write)
        fs::permissions(final_path, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::replace);

        res.set_content("File uploaded successfully: " + unique_filename, "text/plain");
        res.status = 200;

    } catch (const std::exception& e) {
        // Cleanup the temporary file if it exists
        if (fs::exists(temp_path)) {
            fs::remove(temp_path);
        }
        res.set_content("File upload failed: " + std::string(e.what()), "text/plain");
        res.status = 500;
    }
}

void run_tests() {
    httplib::Client cli("localhost", 8080);
    cli.set_connection_timeout(5, 0);

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid .txt file
    {
        httplib::MultipartFormDataItems items = {
            {"file", "Hello C++", "test1.txt", "text/plain"},
        };
        auto res = cli.Post("/upload", items);
        if (res && res->status == 200) {
            std::cout << "[PASS] Test 1: Valid .txt file" << std::endl;
        } else {
            std::cout << "[FAIL] Test 1: Valid .txt file. Status: " << (res ? res->status : -1) << std::endl;
        }
    }

    // Test Case 2: Valid .jpg file
    {
        httplib::MultipartFormDataItems items = {
            {"file", "\xff\xd8\xff\xe0", "test2.jpg", "image/jpeg"},
        };
        auto res = cli.Post("/upload", items);
        if (res && res->status == 200) {
            std::cout << "[PASS] Test 2: Valid .jpg file" << std::endl;
        } else {
            std::cout << "[FAIL] Test 2: Valid .jpg file. Status: " << (res ? res->status : -1) << std::endl;
        }
    }

    // Test Case 3: Disallowed .html file
    {
        httplib::MultipartFormDataItems items = {
            {"file", "<html></html>", "test3.html", "text/html"},
        };
        auto res = cli.Post("/upload", items);
        if (res && res->status == 400) {
            std::cout << "[PASS] Test 3: Disallowed .html file" << std::endl;
        } else {
            std::cout << "[FAIL] Test 3: Disallowed .html file. Status: " << (res ? res->status : -1) << std::endl;
        }
    }

    // Test Case 4: Path traversal attempt
    {
        httplib::MultipartFormDataItems items = {
            {"file", "malicious", "../../etc/passwd.txt", "text/plain"},
        };
        auto res = cli.Post("/upload", items);
        // Should succeed because the server sanitizes the path
        if (res && res->status == 200) {
            std::cout << "[PASS] Test 4: Path traversal attempt" << std::endl;
        } else {
            std::cout << "[FAIL] Test 4: Path traversal attempt. Status: " << (res ? res->status : -1) << std::endl;
        }
    }

    // Test Case 5: No file part (wrong name)
    {
        httplib::MultipartFormDataItems items = {
            {"other_field", "some content", "file.txt", "text/plain"},
        };
        auto res = cli.Post("/upload", items);
        if (res && res->status == 400) {
            std::cout << "[PASS] Test 5: No 'file' part" << std::endl;
        } else {
            std::cout << "[FAIL] Test 5: No 'file' part. Status: " << (res ? res->status : -1) << std::endl;
        }
    }
    
    std::cout << "--- Test Cases Finished ---" << std::endl;
}


int main(int argc, char* argv[]) {
    // Create upload directory if it doesn't exist
    if (!fs::exists(UPLOAD_DIR)) {
        if (!fs::create_directory(UPLOAD_DIR)) {
            std::cerr << "Failed to create upload directory." << std::endl;
            return 1;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "test") {
        // Run client tests. Assumes server is running in another process.
        run_tests();
        return 0;
    }

    httplib::Server svr;

    // Set a payload limit to prevent DoS attacks from large requests
    svr.set_payload_max_length(MAX_FILE_SIZE + 1024 * 1024); // Add buffer for multipart overhead

    svr.Post("/upload", handle_upload);
    
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"(
            <!DOCTYPE html>
            <html>
            <head><title>File Upload</title></head>
            <body>
                <h1>Upload a File</h1>
                <form action="/upload" method="post" enctype="multipart/form-data">
                    <input type="file" name="file">
                    <input type="submit" value="Upload">
                </form>
            </body>
            </html>
        )", "text/html");
    });


    std::cout << "Server starting on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}