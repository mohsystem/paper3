#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <regex>

/**
 * Validates a filename to ensure it is safe.
 * It checks for path traversal sequences and ensures the filename only
 * contains characters from a safe whitelist.
 *
 * @param filename The filename to validate.
 * @return True if the filename is safe, false otherwise.
 */
bool isFilenameSafe(const std::string& filename) {
    // Check for path traversal
    if (filename.find("..") != std::string::npos) {
        return false;
    }
    // Whitelist allowed characters: alphanumeric, dot, underscore, hyphen.
    // This regex ensures no shell metacharacters or directory separators are present.
    const std::regex safePattern("^[a-zA-Z0-9._-]+$");
    return std::regex_match(filename, safePattern);
}

/**
 * Displays the contents of a file using the system() call.
 * CRITICAL: This function relies heavily on the isFilenameSafe() validation
 * to prevent command injection, as system() is inherently dangerous.
 *
 * @param filename The name of the file to display.
 */
void displayFileContents(const std::string& filename) {
    std::cout << "--- Attempting to display file: " << filename << " ---" << std::endl;

    // Security Check 1: Input validation is the primary defense.
    if (!isFilenameSafe(filename)) {
        std::cout << "Error: Invalid or unsafe filename provided." << std::endl;
        std::cout << "--------------------------------------------------\n" << std::endl;
        return;
    }

    // Security Check 2: Verify file existence.
    std::ifstream file(filename);
    if (!file.good()) {
        std::cout << "Error: File does not exist or cannot be opened." << std::endl;
        std::cout << "--------------------------------------------------\n" << std::endl;
        return;
    }
    file.close();


    std::string command_str;

#ifdef _WIN32
    std::string command = "type";
    // On Windows, double quotes are used for arguments with spaces.
    command_str = command + " \"" + filename + "\"";
#else
    std::string command = "cat";
    // On POSIX systems, single quotes prevent shell expansion.
    command_str = command + " '" + filename + "'";
#endif
    
    std::cout << "> Executing command: " << command_str << std::endl;
    int result = system(command_str.c_str());

    if (result != 0) {
        std::cerr << "> Command may have failed with exit code: " << result << std::endl;
    }
    std::cout << "--------------------------------------------------\n" << std::endl;
}

void createTestFile(const std::string& name, const std::string& content) {
    std::ofstream outfile(name);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
    }
}

int main() {
    // Setup: Create some test files
    createTestFile("test1.txt", "This is a safe test file.");
    createTestFile("safe-file.log", "This is another safe log file.");

    // Test Case 1: A valid and safe filename
    displayFileContents("test1.txt");

    // Test Case 2: Another valid filename with a hyphen
    displayFileContents("safe-file.log");

    // Test Case 3: A file that does not exist
    displayFileContents("nonexistent.txt");

    // Test Case 4: A malicious input attempting command injection
    // This is blocked by isFilenameSafe().
    displayFileContents("test1.txt; ls");

    // Test Case 5: A malicious input attempting path traversal
    // This is blocked by isFilenameSafe().
    displayFileContents("../some_other_file");

    // Cleanup: Delete the test files
    remove("test1.txt");
    remove("safe-file.log");

    return 0;
}