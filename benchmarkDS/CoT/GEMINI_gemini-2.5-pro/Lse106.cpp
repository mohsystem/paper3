#include <iostream>
#include <fstream>
#include <string>
#include <cstdio> // For remove()

// For chmod on POSIX-compliant systems (Linux, macOS)
#include <sys/stat.h>

/**
 * Creates a file, writes secret info to it, and sets permissions to owner-read-only.
 *
 * @param fileName The name of the file to create.
 * @param secretInfo The secret string to write to the file.
 * @return true on success, false on failure.
 */
bool createAndSecureFile(const std::string& fileName, const std::string& secretInfo) {
    // 1. Create and write to the file using an output file stream (ofstream).
    // The file is closed automatically when outFile goes out of scope (RAII).
    {
        std::ofstream outFile(fileName);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << fileName << std::endl;
            return false;
        }
        outFile << secretInfo;
        if (outFile.fail()) {
            std::cerr << "Error: Failed to write to file: " << fileName << std::endl;
            return false; // The file will be closed by the destructor
        }
    } // outFile is closed here

    // 2. Change permissions to read-only for the owner (0400).
    // This uses chmod, which is specific to POSIX systems. For Windows,
    // you would need to use _chmod and different permission constants.
    if (chmod(fileName.c_str(), S_IRUSR) != 0) {
        perror("Error changing file permissions");
        // Attempt to clean up the created file on permission failure
        remove(fileName.c_str());
        return false;
    }

    std::cout << "Successfully created and secured file: " << fileName << std::endl;
    return true;
}

int main() {
    // 5 test cases
    std::cout << "--- C++ Test Cases ---" << std::endl;
    createAndSecureFile("secret_alpha.txt", "The launch code is 1234.");
    createAndSecureFile("secret_beta.log", "Meeting at midnight.");
    createAndSecureFile("secret_gamma.dat", "Alpha protocol is a go.");
    createAndSecureFile("top_secret.conf", "Password=SuperSecret!@#");
    createAndSecureFile("confidential_notes.txt", "This is a test of the emergency secret system.");
    std::cout << "----------------------\n" << std::endl;
    
    return 0;
}