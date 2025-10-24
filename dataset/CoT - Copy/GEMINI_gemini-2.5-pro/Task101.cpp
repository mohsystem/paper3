#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdio> // For std::remove
#include <sys/stat.h> // For chmod

/**
 * Creates a file and makes it executable (rwxr-xr-x).
 * This function is intended for POSIX-compliant systems (Linux, macOS).
 *
 * @param filename The name of the file to create.
 * @return true if successful, false otherwise.
 */
bool createAndMakeExecutable(const std::string& filename) {
    // For test repeatability, remove the file if it exists.
    std::remove(filename.c_str());

    // Step 1: Create the file.
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create file '" << filename << "'." << std::endl;
        return false;
    }
    file.close();

    // Step 2: Set permissions to rwxr-xr-x (octal 0755).
    // S_IRWXU: Read, write, execute for owner.
    // S_IRGRP | S_IXGRP: Read, execute for group.
    // S_IROTH | S_IXOTH: Read, execute for others.
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    if (chmod(filename.c_str(), mode) != 0) {
        perror(("Error setting permissions for '" + filename + "'").c_str());
        return false;
    }

    std::cout << "Successfully created and set permissions for '" << filename << "'." << std::endl;
    return true;
}

int main() {
    // 5 test cases
    std::vector<std::string> testFiles = {
        "script1.sh",
        "script2.sh",
        "script3.sh",
        "script4.sh",
        "script5.sh"
    };

    for (const auto& filename : testFiles) {
        std::cout << "--- Testing: " << filename << " ---" << std::endl;
        createAndMakeExecutable(filename);
        std::cout << std::endl;
    }

    return 0;
}