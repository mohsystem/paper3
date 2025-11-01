#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

// For creating directories and files in main
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

namespace fs = std::filesystem;

/**
 * Securely reads a file from a base directory, preventing path traversal.
 *
 * @param baseDirectory The secure directory from which files can be served.
 * @param fileName The name of the file requested by the user.
 * @return A vector of characters with the file contents. Returns an empty vector
 *         if the file is not found, not accessible, or the request is insecure.
 */
std::vector<char> img(const std::string& baseDirectory, const std::string& fileName) {
    try {
        // Get canonical (absolute, resolved) path for the base directory.
        const fs::path baseDirPath = fs::canonical(baseDirectory);
        
        // Construct the full path and normalize it to resolve "." and ".." lexically.
        // This is the path we want to check.
        fs::path requestedPath = (baseDirPath / fileName).lexically_normal();

        // *** SECURITY CHECK ***
        // To prevent path traversal, we get the canonical path of the request.
        // If the file doesn't exist or is outside the base directory, canonical()
        // will throw an exception, which we catch.
        // After that, we check if the resulting path is a subpath of our base dir.
        fs::path finalPath = fs::canonical(requestedPath);
        
        auto [baseEnd, mismatch] = std::mismatch(baseDirPath.begin(), baseDirPath.end(), finalPath.begin());

        if (baseEnd != baseDirPath.end()) {
            // This means the final path is not even a subpath of the base directory.
             std::cerr << "Security alert: Path traversal attempt detected for file: " << fileName << std::endl;
             return {};
        }

        // Ensure it's a regular file
        if (!fs::is_regular_file(finalPath)) {
            // std::cerr << "Error: Not a regular file: " << finalPath << std::endl;
            return {};
        }

        std::ifstream file(finalPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size)) {
            return buffer;
        }
    } catch (const fs::filesystem_error& e) {
        // This will catch errors from fs::canonical, e.g., if the file doesn't exist
        // or if a path component is not a directory. This is a safe failure.
        // std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return {}; // Return empty vector on any failure
}

void runTest(const std::string& testName, const std::string& baseDir, const std::string& fileName) {
    std::cout << "--- Running Test: " << testName << " ---" << std::endl;
    std::cout << "Requesting file: " << fileName << std::endl;
    std::vector<char> content = img(baseDir, fileName);
    if (!content.empty()) {
        std::cout << "Result: SUCCESS - Read " << content.size() << " bytes." << std::endl;
    } else {
        std::cout << "Result: FAILED (or file securely blocked) - as expected for invalid cases." << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    const fs::path baseDir{"safe_image_dir"};
    const fs::path unsafeDir{"unsafe_dir"};
    
    // Setup
    try {
        fs::create_directory(baseDir);
        fs::create_directory(baseDir / "subdir");
        fs::create_directory(unsafeDir);

        std::ofstream(baseDir / "image1.jpg") << "jpeg_data_for_image1";
        std::ofstream(baseDir / "subdir" / "image2.png") << "png_data_for_image2";
        std::ofstream(unsafeDir / "secret.txt") << "this is a secret file";

        // --- Test Cases ---
        // 1. Valid file in base directory
        runTest("Valid File", baseDir.string(), "image1.jpg");

        // 2. Valid file in subdirectory
        runTest("Valid Subdirectory File", baseDir.string(), "subdir/image2.png");
        
        // 3. Non-existent file
        runTest("Non-Existent File", baseDir.string(), "notfound.gif");

        // 4. Path traversal attack
        fs::path traversalPath = fs::path("..") / unsafeDir.filename() / "secret.txt";
        runTest("Path Traversal Attack", baseDir.string(), traversalPath.string());

        // 5. Absolute path attack
        runTest("Absolute Path Attack", baseDir.string(), fs::absolute(unsafeDir / "secret.txt").string());

    } catch (const std::exception& e) {
        std::cerr << "Setup or test run failed: " << e.what() << std::endl;
    }

    // Cleanup
    fs::remove_all(baseDir);
    fs::remove_all(unsafeDir);

    return 0;
}