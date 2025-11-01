#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> // C++17 required

// A predefined base directory to prevent access to other parts of the filesystem.
const std::string IMAGE_BASE_DIRECTORY = "test_images";

// Define a struct to hold the result
struct FileData {
    std::vector<char> data;
    bool success;
};

/**
 * Simulates fetching an image file based on a 'file' parameter.
 * This function securely reads a file from a predefined base directory,
 * preventing path traversal attacks.
 *
 * @param fileName The name of the file requested. This should be a simple
 *                 filename without any path components.
 * @return A FileData struct. `success` is true and `data` contains the file
 *         content if successful. Otherwise, `success` is false.
 */
FileData img(const std::string& fileName) {
    if (fileName.empty()) {
        std::cerr << "Error: File name is empty." << std::endl;
        return {{}, false};
    }

    try {
        // Create paths for the base directory and the requested file
        std::filesystem::path baseDir = std::filesystem::absolute(IMAGE_BASE_DIRECTORY);
        
        // Security Step 1: Sanitize the input filename.
        std::filesystem::path filePart(fileName);
        if (filePart.has_parent_path() && !filePart.parent_path().empty()) {
             std::cerr << "Security Error: Path components are not allowed in filename: " << fileName << std::endl;
             return {{}, false};
        }
        
        // Security Step 2: Construct the full path and canonicalize it.
        std::filesystem::path fullPath = baseDir / filePart;
        std::filesystem::path canonicalPath = std::filesystem::canonical(fullPath);

        // Security Step 3: Verify the canonical path is within the base directory.
        // This is the primary defense against path traversal attacks.
        auto res = std::mismatch(baseDir.begin(), baseDir.end(), canonicalPath.begin());
        if (res.first != baseDir.end()) {
            std::cerr << "Security Error: Path traversal attempt detected for file: " << fileName << std::endl;
            return {{}, false};
        }

        // Check if it's a regular file
        if (!std::filesystem::is_regular_file(canonicalPath)) {
            std::cerr << "Error: Not a regular file or does not exist: " << canonicalPath << std::endl;
            return {{}, false};
        }

        // Open the file in binary mode
        std::ifstream file(canonicalPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file: " << canonicalPath << std::endl;
            return {{}, false};
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size)) {
            return {buffer, true};
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return {{}, false};
}


void setup_test_environment() {
    try {
        std::filesystem::create_directory(IMAGE_BASE_DIRECTORY);
        std::ofstream safeFile(std::filesystem::path(IMAGE_BASE_DIRECTORY) / "test1.jpg", std::ios::binary);
        safeFile << "ImageData1";
        safeFile.close();

        std::ofstream secretFile("secret.txt", std::ios::binary);
        secretFile << "SecretData";
        secretFile.close();
        std::cout << "Test environment set up." << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
}

void cleanup_test_environment() {
    try {
        std::filesystem::remove_all(IMAGE_BASE_DIRECTORY);
        std::filesystem::remove("secret.txt");
        std::cout << "\n--- Test environment cleaned up. ---" << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to clean up test environment: " << e.what() << std::endl;
    }
}

// Main method with test cases
int main() {
    setup_test_environment();

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid file
    std::cout << "Test 1: Requesting a valid file 'test1.jpg'" << std::endl;
    FileData result1 = img("test1.jpg");
    std::string content1(result1.data.begin(), result1.data.end());
    if (result1.success && content1 == "ImageData1") {
        std::cout << "  Result: SUCCESS, content matches." << std::endl;
    } else {
        std::cout << "  Result: FAILED." << std::endl;
    }

    // Test Case 2: Non-existent file
    std::cout << "\nTest 2: Requesting a non-existent file 'notfound.jpg'" << std::endl;
    FileData result2 = img("notfound.jpg");
    if (!result2.success) {
        std::cout << "  Result: SUCCESS, function failed as expected." << std::endl;
    } else {
        std::cout << "  Result: FAILED." << std::endl;
    }

    // Test Case 3: Path traversal attempt
    std::cout << "\nTest 3: Path traversal attempt '../secret.txt'" << std::endl;
    FileData result3 = img("../secret.txt");
    if (!result3.success) {
        std::cout << "  Result: SUCCESS, path traversal blocked." << std::endl;
    } else {
        std::cout << "  Result: FAILED." << std::endl;
    }
    
    // Test Case 4: Path traversal with backslashes
    std::cout << "\nTest 4: Path traversal attempt '..\\secret.txt'" << std::endl;
    FileData result4 = img("..\\secret.txt");
    if (!result4.success) {
        std::cout << "  Result: SUCCESS, path traversal blocked." << std::endl;
    } else {
        std::cout << "  Result: FAILED." << std::endl;
    }

    // Test Case 5: Absolute path attempt
    std::filesystem::path secret_abs = std::filesystem::absolute("secret.txt");
    std::cout << "\nTest 5: Absolute path attempt '" << secret_abs.string() << "'" << std::endl;
    FileData result5 = img(secret_abs.string());
    if (!result5.success) {
        std::cout << "  Result: SUCCESS, absolute path blocked." << std::endl;
    } else {
        std::cout << "  Result: FAILED." << std::endl;
    }

    cleanup_test_environment();
    return 0;
}