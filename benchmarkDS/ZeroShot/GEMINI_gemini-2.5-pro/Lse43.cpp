#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

/**
 * Securely reads a file from the /safe/ directory.
 * Prevents path traversal by canonicalizing the path and ensuring it is
 * within the bounds of the safe directory.
 *
 * @param filename The name of the file to be read.
 * @return An optional string containing the file's content if successful,
 *         or an empty optional otherwise.
 */
std::optional<std::string> readFileFromSafeDirectory(const std::string& filename) {
    if (filename.empty() || filename.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid filename format or potential path traversal characters detected." << std::endl;
        return std::nullopt;
    }

    try {
        const fs::path baseDir = "/safe/";
        // It's important that baseDir exists, or canonical() will throw
        if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
             std::cerr << "Error: The base directory " << baseDir << " does not exist." << std::endl;
             return std::nullopt;
        }
        
        fs::path canonicalBaseDir = fs::canonical(baseDir);
        fs::path requestedPath = canonicalBaseDir / filename;

        // weakly_canonical doesn't fail if the path doesn't exist, which is good for checking.
        fs::path canonicalPath = fs::weakly_canonical(requestedPath);

        // Security Check: Check if the canonical path of the request is within the canonical base directory.
        // A robust way is to check if the base path is a prefix of the requested path.
        auto baseStr = canonicalBaseDir.string();
        auto pathStr = canonicalPath.string();

        if (pathStr.rfind(baseStr, 0) != 0) {
            std::cerr << "Error: Path traversal attempt detected for file: " << filename << std::endl;
            return std::nullopt;
        }

        // Additional check to ensure it's a regular file before opening
        if (!fs::is_regular_file(canonicalPath)) {
            std::cerr << "Error: Path does not point to a regular file: " << canonicalPath << std::endl;
            return std::nullopt;
        }

        std::ifstream fileStream(canonicalPath);
        if (!fileStream.is_open()) {
            std::cerr << "Error: Could not open file: " << filename << std::endl;
            return std::nullopt;
        }

        std::string content((std::istreambuf_iterator<char>(fileStream)),
                             std::istreambuf_iterator<char>());
        fileStream.close();
        return content;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    std::cout << "NOTE: For these tests to pass, create the /safe directory and files:" << std::endl;
    std::cout << "  sudo mkdir -p /safe/docs" << std::endl;
    std::cout << "  sudo chown -R $(whoami) /safe" << std::endl;
    std::cout << "  echo \"This is a safe report.\" > /safe/report.txt" << std::endl;
    std::cout << "  echo \"This is a nested manual.\" > /safe/docs/manual.pdf\n" << std::endl;

    std::vector<std::string> testFilenames = {
        "report.txt",                 // 1. Valid file
        "docs/manual.pdf",            // 2. Valid file in a subdirectory
        "nonexistent.txt",            // 3. Non-existent file
        "../../../../etc/passwd",     // 4. Path traversal attempt
        "/etc/passwd"                 // 5. Absolute path attempt
    };

    for (const auto& filename : testFilenames) {
        std::cout << "--- Testing with filename: '" << filename << "' ---" << std::endl;
        auto content = readFileFromSafeDirectory(filename);
        if (content) {
            std::cout << "Successfully read file. Content:" << std::endl;
            std::cout << *content << std::endl;
        } else {
            std::cout << "Failed to read file." << std::endl;
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    return 0;
}