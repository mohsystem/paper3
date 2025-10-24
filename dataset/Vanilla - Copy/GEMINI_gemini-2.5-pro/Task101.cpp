#include <iostream>
#include <fstream>
#include <string>
#include <vector>
// Required for chmod on POSIX systems (Linux, macOS, etc.)
#include <sys/stat.h>

/**
 * Creates a file with the given name, writes a simple shell script to it,
 * and makes it executable.
 * Note: This implementation uses chmod, which is specific to POSIX-compliant
 * operating systems and will not work on Windows.
 *
 * @param filename The name of the file to create.
 * @return true if successful, false otherwise.
 */
bool createAndMakeExecutable(const std::string& filename) {
    // Create and write to the file
    std::ofstream scriptFile(filename);
    if (!scriptFile.is_open()) {
        std::cerr << "Error: Could not create file " << filename << std::endl;
        return false;
    }
    scriptFile << "#!/bin/bash" << std::endl;
    scriptFile << "echo \"Hello from '" << filename << "'!\"" << std::endl;
    scriptFile.close();
    std::cout << "File created: " << filename << std::endl;

    // Set permissions to rwxr-xr-x (0755)
    if (chmod(filename.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        perror("Error setting file permissions");
        return false;
    }
    
    std::cout << "File permissions set to executable." << std::endl;
    return true;
}

int main() {
    // 5 Test Cases
    std::vector<std::string> testFiles = {"script.sh", "test_script_1.sh", "test_script_2.sh", "another.sh", "final.sh"};
    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": " << testFiles[i] << " ---" << std::endl;
        bool success = createAndMakeExecutable(testFiles[i]);
        std::cout << "Result: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        std::cout << std::endl;
    }
    return 0;
}