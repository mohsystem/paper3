#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <optional>

// The base directory from which files can be served.
const std::filesystem::path BASE_DIRECTORY = "public";

namespace fs = std::filesystem;

/**
 * @brief Securely retrieves the content of a file from a predefined base directory.
 *
 * It prevents path traversal attacks by ensuring the requested file's canonical
 * path is within the base directory.
 * @param userProvidedFilename The name of the file provided by the user.
 * @return An optional containing the file content if successful, otherwise an empty optional.
 */
std::optional<std::string> retrieveFileContent(const std::string& userProvidedFilename) {
    if (userProvidedFilename.empty()) {
        std::cerr << "Error: Filename cannot be empty." << std::endl;
        return std::nullopt;
    }

    try {
        fs::path baseDirPath = fs::absolute(BASE_DIRECTORY);
        
        // Sanitize input: only consider the filename part, stripping any path.
        // This is a crucial step to prevent directory traversal.
        fs::path safeFilename = fs::path(userProvidedFilename).filename();
        if (safeFilename.empty()) {
             std::cerr << "Error: Invalid filename format provided." << std::endl;
             return std::nullopt;
        }
        
        fs::path fullPath = baseDirPath / safeFilename;

        // Check for existence before canonicalization to avoid exception for non-existent files.
        if (!fs::exists(fullPath)) {
            std::cerr << "Error: File does not exist at path: " << fullPath << std::endl;
            return std::nullopt;
        }

        // Get the canonical (real) path of the target file.
        // This resolves any '..', '.', or symbolic links.
        fs::path canonicalFullPath = fs::canonical(fullPath);
        fs::path canonicalBasePath = fs::canonical(baseDirPath);

        // Security Check: The real path of the file must be within the real path of the base directory.
        std::string fullPathStr = canonicalFullPath.string();
        std::string basePathStr = canonicalBasePath.string();

        if (fullPathStr.rfind(basePathStr, 0) != 0) {
            std::cerr << "Security Alert: Path traversal attempt detected for file: " << userProvidedFilename << std::endl;
            return std::nullopt;
        }
        
        if (fs::is_regular_file(canonicalFullPath)) {
            std::ifstream fileStream(canonicalFullPath, std::ios::in | std::ios::binary);
            if (fileStream) {
                std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
                return content;
            }
        } else {
             std::cerr << "Error: Path is not a regular file: " << canonicalFullPath << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error retrieving '" << userProvidedFilename << "': " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
    }

    return std::nullopt;
}

void setupTestEnvironment() {
    try {
        fs::remove_all(BASE_DIRECTORY);
        fs::remove_all("secret");
        
        fs::create_directories(BASE_DIRECTORY / "safe_dir");
        fs::create_directories("secret");
        
        std::ofstream(BASE_DIRECTORY / "file1.txt") << "This is file 1.";
        std::ofstream(BASE_DIRECTORY / "safe_dir" / "file2.txt") << "This is file 2.";
        std::ofstream("secret/secret.txt") << "This is a secret.";
        
        std::cout << "Test environment created successfully." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
}

int main() {
    setupTestEnvironment();

    // --- Test Cases ---
    std::vector<std::string> testCases = {
        "file1.txt",                          // 1. Valid file
        "safe_dir/file2.txt",                 // 2. Valid file in subdir (should fail due to basename sanitization)
        "../secret/secret.txt",               // 3. Path Traversal
    #ifdef _WIN32
        "C:\\Windows\\System32\\drivers\\etc\\hosts", // 4. Absolute Path (Windows)
    #else
        "/etc/passwd",                      // 4. Absolute Path (Unix-like)
    #endif
        "nonexistent.txt"                     // 5. Non-existent file
    };

    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << ": Requesting '" << testCases[i] << "' ---" << std::endl;
        auto content = retrieveFileContent(testCases[i]);
        if (content) {
            std::cout << "Success! File content:\n" << *content << std::endl;
        } else {
            std::cout << "Failed as expected." << std::endl;
        }
    }

    // --- Cleanup ---
    fs::remove_all(BASE_DIRECTORY);
    fs::remove_all("secret");
    std::cout << "\nTest environment cleaned up." << std::endl;

    return 0;
}