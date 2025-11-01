#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <regex>
#include <algorithm> // for std::transform

// For C++17 filesystem
namespace fs = std::filesystem;

const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {".txt", ".pdf", ".jpg", ".png"};
const std::regex FILENAME_REGEX("^[a-zA-Z0-9._-]+$");

/**
 * Securely saves an uploaded file.
 *
 * @param original_filename The original name of the file from the user.
 * @param content The binary content of the file.
 * @return True on success, false on failure.
 */
bool upload_file(const std::string& original_filename, const std::vector<char>& content) {
    if (original_filename.empty() || content.empty()) {
        std::cerr << "Error: Filename or content is empty." << std::endl;
        return false;
    }

    // 1. Check file size
    if (content.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        return false;
    }
    
    // 2. Sanitize filename to prevent path traversal
    fs::path original_path(original_filename);
    std::string basename = original_path.filename().string();
    
    if (basename.empty()) {
        std::cerr << "Error: Filename is empty after sanitization." << std::endl;
        return false;
    }

    // Check for null bytes
    if (basename.find('\0') != std::string::npos) {
        std::cerr << "Error: Filename contains null byte." << std::endl;
        return false;
    }
    
    // Additional check for potentially harmful characters
    if (!std::regex_match(basename, FILENAME_REGEX)) {
        std::cerr << "Error: Filename '" << basename << "' contains invalid characters." << std::endl;
        return false;
    }

    // 3. Validate file extension
    std::string ext = fs::path(basename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    bool allowed = false;
    if (!ext.empty()) {
        for (const auto& allowed_ext : ALLOWED_EXTENSIONS) {
            if (ext == allowed_ext) {
                allowed = true;
                break;
            }
        }
    }
    if (!allowed) {
        std::cerr << "Error: File type '" << fs::path(basename).extension().string() << "' is not allowed." << std::endl;
        return false;
    }

    try {
        // 4. Create the upload directory if it doesn't exist
        fs::path upload_path(UPLOAD_DIR);
        fs::create_directories(upload_path);

        // 5. Construct a safe destination path. Here we check for existence to prevent overwrites.
        // A real-world application should generate a unique name (e.g., using a UUID).
        fs::path destination_path = upload_path / basename;
        
        // Final sanity check: ensure the final path is within our upload directory
        fs::path canonical_dest = fs::weakly_canonical(destination_path);
        fs::path canonical_upload_dir = fs::weakly_canonical(upload_path);
        if (canonical_dest.string().rfind(canonical_upload_dir.string(), 0) != 0) {
            std::cerr << "Error: Attempted file path is outside the designated upload directory." << std::endl;
            return false;
        }

        if (fs::exists(destination_path)) {
            std::cerr << "Error: File '" << basename << "' already exists. Upload aborted to prevent overwrite." << std::endl;
            return false;
        }
        
        // 6. Save the file
        std::ofstream outfile(destination_path, std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not open file for writing." << std::endl;
            return false;
        }
        outfile.write(content.data(), content.size());
        outfile.close();

        std::cout << "File saved successfully to: " << fs::absolute(destination_path).string() << std::endl;
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

void run_test(const std::string& test_name, const std::string& filename, const std::string& content_str) {
    std::cout << "\n[Test] " << test_name << std::endl;
    std::vector<char> content(content_str.begin(), content_str.end());
    upload_file(filename, content);
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    
    // Test Case 1: Normal, valid file upload
    run_test("Normal file upload", "report.txt", "This is a C++ report.");

    // Test Case 2: Path traversal attempt
    run_test("Path traversal attempt", "../../../etc/hosts", "127.0.0.1 localhost");

    // Test Case 3: File too large
    std::cout << "\n[Test] File too large" << std::endl;
    std::vector<char> large_content(MAX_FILE_SIZE + 1, 'A');
    upload_file("bigfile.jpg", large_content);
    
    // Test Case 4: Disallowed file type
    run_test("Disallowed file type", "virus.dll", "some malicious binary data");
    
    // Test Case 5: Filename with invalid characters
    run_test("Filename with invalid characters", "photo album: summer '23.png", "some image data");
    
    // Create the test file again to test overwrite protection
    run_test("Re-upload to test overwrite protection", "report.txt", "This is an updated report.");

    std::cout << "\n--- Test Cases Finished ---" << std::endl;
    return 0;
}