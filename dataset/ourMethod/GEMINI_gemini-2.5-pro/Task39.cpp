#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstdio> // For remove

/**
 * @brief Validates a filename against a strict whitelist.
 *
 * This function ensures that the filename is safe to be used in a shell command.
 * It checks for:
 * - Non-empty string.
 * - Length within a reasonable limit (255 characters).
 * - Characters are restricted to a whitelist (alphanumeric, dot, underscore, hyphen).
 * - Prevents special directory names "." and "..".
 * This helps mitigate command injection and path traversal vulnerabilities.
 *
 * @param filename The filename to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool isValidFilename(const std::string& filename) {
    if (filename.empty() || filename.length() > 255) {
        return false;
    }

    const std::string whitelist = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-";
    if (filename.find_first_not_of(whitelist) != std::string::npos) {
        return false;
    }

    if (filename == "." || filename == "..") {
        return false;
    }

    return true;
}

/**
 * @brief Displays the content of a file using a system command.
 *
 * It first validates the filename to prevent security vulnerabilities before
 * constructing and executing a system command (`cat` on POSIX, `type` on Windows).
 *
 * @param filename The name of the file to display. The file should be in the current directory.
 */
void displayFileContent(const std::string& filename) {
    if (!isValidFilename(filename)) {
        std::cerr << "Error: Invalid filename provided: \"" << filename << "\"" << std::endl;
        return;
    }

    // Construct the command safely. Quoting adds an extra layer of defense.
#ifdef _WIN32
    std::string command = "type \"" + filename + "\"";
#else
    std::string command = "cat '" + filename + "'";
#endif

    std::cout << "--- Executing command: " << command << " ---" << std::endl;
    int result = system(command.c_str());
    if (result != 0) {
        // The shell command (cat/type) usually prints a descriptive error to stderr
        // (e.g., "file not found"), so we just note that the command might have failed.
        std::cerr << "--- Command may have failed or file not found. Exit code: " << result << " ---" << std::endl;
    }
    std::cout << "--- End of content for " << filename << " ---" << std::endl << std::endl;
}

/**
 * @brief Helper function to create a dummy file for testing purposes.
 * @param filename The name of the file to create.
 * @param content The content to write to the file.
 */
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
    }
}

int main() {
    // Setup: Create a couple of files for testing.
    createTestFile("test1.txt", "This is the first test file.");
    createTestFile("test-file_2.log", "This is the second test file.\nIt has a hyphen and an underscore.");

    std::vector<std::string> testFilenames = {
        // Test Case 1: A valid, simple filename that exists.
        "test1.txt",
        // Test Case 2: A valid filename with allowed special characters that exists.
        "test-file_2.log",
        // Test Case 3: An invalid filename with a command injection attempt.
        "test.txt; ls -la",
        // Test Case 4: An invalid filename with path traversal characters.
        "../../etc/passwd",
        // Test Case 5: A non-existent but validly named file.
        "no_such_file.txt"
    };

    for (const auto& filename : testFilenames) {
        std::cout << "Attempting to display: \"" << filename << "\"" << std::endl;
        displayFileContent(filename);
    }

    // Cleanup: Remove the created test files.
    remove("test1.txt");
    remove("test-file_2.log");

    return 0;
}