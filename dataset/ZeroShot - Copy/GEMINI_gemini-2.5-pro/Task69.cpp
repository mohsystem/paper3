#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Securely deletes a file after validating the path.
 *
 * The function ensures the path is a regular file and is located within the
 * current working directory. This is achieved by comparing the canonical path
 * of the file with the canonical path of the CWD, preventing path traversal.
 *
 * @param filePath The path to the file to be deleted.
 * @return true if the file was successfully deleted, false otherwise.
 */
bool deleteFileSecurely(const std::string& filePath) {
    if (filePath.empty()) {
        std::cerr << "Error: File path is empty." << std::endl;
        return false;
    }

    try {
        const fs::path pathObj(filePath);

        // Security Check 1: Resolve the path without requiring it to exist yet.
        // This allows checking for path traversal before accessing the file system.
        fs::path canonicalCwd = fs::canonical(fs::current_path());
        fs::path weaklyCanonicalPath = fs::weakly_canonical(pathObj);

        // Security Check 2: Check if the resolved path is within the CWD.
        // We compare the string representations of the paths.
        if (weaklyCanonicalPath.string().rfind(canonicalCwd.string(), 0) != 0) {
            std::cerr << "Error: Path traversal attempt detected. Cannot delete files outside the current directory." << std::endl;
            return false;
        }

        // Validation Check 1: Ensure path exists and is a regular file, not a directory.
        if (!fs::is_regular_file(pathObj)) {
            if (fs::exists(pathObj)) {
                std::cerr << "Error: Path does not point to a regular file (it might be a directory): " << filePath << std::endl;
            } else {
                std::cerr << "Error: File does not exist: " << filePath << std::endl;
            }
            return false;
        }

        // Attempt to delete the file
        if (fs::remove(pathObj)) {
            std::cout << "Successfully deleted file: " << filePath << std::endl;
            return true;
        } else {
            // This case might be rare if is_regular_file passed, but good for robustness
            std::cerr << "Error: Failed to delete file. Check permissions: " << filePath << std::endl;
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

void runTestCases() {
    std::cout << "\n--- Running C++ Test Cases ---" << std::endl;
    const std::string safeFile = "test_safe.txt";
    const std::string testDir = "test_dir";
    const std::string unsafeFile = "../test_unsafe.txt";

    // Setup test environment
    try {
        std::ofstream(safeFile).close();
        fs::create_directory(testDir);
        std::cout << "Test environment created." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        return;
    }

    // Run tests
    // Test 1: Success case
    std::cout << "\n[Test 1] Deleting a safe local file..." << std::endl;
    deleteFileSecurely(safeFile);

    // Test 2: Failure case - non-existent file
    std::cout << "\n[Test 2] Deleting a non-existent file..." << std::endl;
    deleteFileSecurely("nonexistent.txt");

    // Test 3: Failure case - directory
    std::cout << "\n[Test 3] Deleting a directory..." << std::endl;
    deleteFileSecurely(testDir);

    // Test 4: Failure case - path traversal
    std::cout << "\n[Test 4] Attempting path traversal deletion..." << std::endl;
    deleteFileSecurely(unsafeFile);

    // Test 5: Failure case - absolute path
    std::cout << "\n[Test 5] Attempting to delete file with an absolute path..." << std::endl;
    fs::path tmp_path = fs::temp_directory_path() / "abs_test.txt";
    std::ofstream(tmp_path).close(); // create temp file
    deleteFileSecurely(tmp_path.string());
    if(fs::exists(tmp_path)) fs::remove(tmp_path); // clean up if deletion failed

    // Cleanup
    try {
        // Safe file should already be gone, but remove just in case test failed
        if (fs::exists(safeFile)) fs::remove(safeFile);
        if (fs::exists(testDir)) fs::remove(testDir);
        std::cout << "\nTest environment cleaned up." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to clean up test environment: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << "Attempting to delete file from command line argument: " << argv[1] << std::endl;
        deleteFileSecurely(argv[1]);
    } else {
        std::cout << "No command line arguments provided. Running test cases..." << std::endl;
        runTestCases();
    }
    return 0;
}