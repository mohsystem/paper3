#include <iostream>
#include <fstream>
#include <string>

// Platform-specific include for setting file permissions
#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#endif

class Lse101 {
public:
    /**
     * Creates a file, writes content to it, then sets secure permissions.
     * Note: Permissions are set after file creation, which is not an atomic
     * operation. This creates a small window for a race condition.
     * On POSIX systems, permissions are set to owner-only read/write (600).
     *
     * @param fileName The name of the file to write to.
     * @param content  The string content to write to the file.
     * @return true on success, false on failure.
     */
    static bool writeSecureFile(const std::string& fileName, const std::string& content) {
        if (fileName.empty()) {
            std::cerr << "Error: File name cannot be empty." << std::endl;
            return false;
        }

        // RAII handles closing the file automatically when outFile goes out of scope
        std::ofstream outFile(fileName, std::ios::out | std::ios::trunc);

        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file '" << fileName << "' for writing." << std::endl;
            return false;
        }

        outFile << content;

        if (outFile.fail()) {
            std::cerr << "Error: Failed to write content to file '" << fileName << "'." << std::endl;
            return false;
        }
        
        outFile.close();

        // Set file permissions to be readable/writable by owner only (600)
        #if defined(__unix__) || defined(__APPLE__)
        if (chmod(fileName.c_str(), S_IRUSR | S_IWUSR) != 0) {
            // This is a warning, not a fatal error, as the write succeeded.
            std::cerr << "Warning: Could not set permissions on file '" << fileName << "'." << std::endl;
        }
        #endif

        return true;
    }

    static void runTestCase(const std::string& fileName, const std::string& content) {
        std::cout << "Running test for file: '" << fileName << "'" << std::endl;
        bool success = writeSecureFile(fileName, content);
        if (success) {
            std::cout << " -> Success." << std::endl;
        } else {
            std::cout << " -> Failed." << std::endl;
        }
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    Lse101::runTestCase("important_config", "important_config");
    Lse101::runTestCase("user_settings.conf", "user=admin;level=5;");
    Lse101::runTestCase("api_key.txt", "ABC-123-DEF-456");
    Lse101::runTestCase("log_file.log", "Initial log entry.");
    Lse101::runTestCase("", "some content"); // Invalid filename

    return 0;
}