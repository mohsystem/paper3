#include <iostream>
#include <string>
#include <cstdlib> // For system()
#include <sys/stat.h> // For mkdir
#include <sys/types.h> // For mkdir

/**
 * Extracts a tar archive by calling the system's `tar` command.
 * This function first creates the destination directory if it doesn't exist.
 *
 * @param archivePath Path to the .tar, .tar.gz, or .tar.bz2 file.
 * @param destDir Path to the destination directory.
 * @return true on success (if the system command returns 0), false otherwise.
 */
bool extractArchive(const std::string& archivePath, const std::string& destDir) {
    // Create the destination directory. The '0755' gives rwx for owner, r-x for group/others.
    // On Unix-like systems. This might behave differently on Windows.
    mkdir(destDir.c_str(), 0755);

    // Construct the command string.
    // -x: extract, -f: file, -C: change to directory.
    // The tar command usually auto-detects compression (gz, bz2).
    std::string command = "tar -xf \"" + archivePath + "\" -C \"" + destDir + "\"";
    
    std::cout << "Executing: " << command << std::endl;
    
    // Execute the command using system()
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Error: System command failed with exit code " << result << std::endl;
        // A common issue could be that the archivePath does not exist.
        return false;
    }
    
    return true;
}

int main() {
    std::cout << "--- Running C++ Tests ---" << std::endl;
    std::cout << "Note: These tests assume the Python script has been run to create test archives in /tmp/." << std::endl;

    // Test Case 1: Extract a standard tar.gz archive
    std::cout << "\n[Test 1] Extracting simple.tar.gz to /tmp/cpp_unpack_1" << std::endl;
    bool success1 = extractArchive("/tmp/simple.tar.gz", "/tmp/cpp_unpack_1");
    std::cout << "Test 1 Success: " << (success1 ? "true" : "false") << std::endl;

    // Test Case 2: Extract a tar.bz2 archive
    std::cout << "\n[Test 2] Extracting simple.tar.bz2 to /tmp/cpp_unpack_2" << std::endl;
    bool success2 = extractArchive("/tmp/simple.tar.bz2", "/tmp/cpp_unpack_2");
    std::cout << "Test 2 Success: " << (success2 ? "true" : "false") << std::endl;

    // Test Case 3: Attempt to extract a non-existent file
    std::cout << "\n[Test 3] Extracting non_existent.tar.gz to /tmp/cpp_unpack_3 (expecting error)" << std::endl;
    bool success3 = extractArchive("/tmp/non_existent.tar.gz", "/tmp/cpp_unpack_3");
    std::cout << "Test 3 Success: " << (success3 ? "true" : "false") << std::endl;

    // Test Case 4: Extract an archive with subdirectories
    std::cout << "\n[Test 4] Extracting nested.tar.gz to /tmp/cpp_unpack_4" << std::endl;
    bool success4 = extractArchive("/tmp/nested.tar.gz", "/tmp/cpp_unpack_4");
    std::cout << "Test 4 Success: " << (success4 ? "true" : "false") << std::endl;

    // Test Case 5: Extract to the same directory again (should overwrite)
    std::cout << "\n[Test 5] Extracting simple.tar.gz to /tmp/cpp_unpack_1 again" << std::endl;
    bool success5 = extractArchive("/tmp/simple.tar.gz", "/tmp/cpp_unpack_1");
    std::cout << "Test 5 Success: " << (success5 ? "true" : "false") << std::endl;

    return 0;
}