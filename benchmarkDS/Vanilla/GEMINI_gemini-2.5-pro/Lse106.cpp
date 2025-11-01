#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h> // For chmod on POSIX systems

/**
 * Creates a file, writes content to it, and sets permissions to owner-read-only.
 * Note: This uses the POSIX `chmod` function and may not work as expected on 
 * non-POSIX systems like Windows.
 *
 * @param fileName The name of the file to create.
 * @param content The super secret info to write to the file.
 * @return The name of the file as a string, or an empty string on failure.
 */
std::string createSecretFile(const std::string& fileName, const std::string& content) {
    // 1. & 2. Create a file and write the super secret info to it.
    std::ofstream outFile(fileName);
    if (!outFile) {
        std::cerr << "Error: Could not open file for writing: " << fileName << std::endl;
        return "";
    }
    outFile << content;
    outFile.close();

    // 3. Change the permissions of the file to read-only for the owner (0400).
    // S_IRUSR is the POSIX constant for owner read permission.
    if (chmod(fileName.c_str(), S_IRUSR) != 0) {
        perror("Error changing file permissions");
        return "";
    }

    // 4. Print the name of the file to the screen.
    std::cout << fileName << std::endl;
    return fileName;
}

int main() {
    std::cout << "--- Running CPP Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string fileName = "cpp_secret_file_" + std::to_string(i) + ".txt";
        std::string secretInfo = "This is C++ secret #" + std::to_string(i) + ". The code is 'gamma'.";
        std::cout << "Created file: ";
        createSecretFile(fileName, secretInfo);
    }
    std::cout << "--------------------------\n" << std::endl;
    return 0;
}