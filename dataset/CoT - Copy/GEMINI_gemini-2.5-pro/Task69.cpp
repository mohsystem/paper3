#include <iostream>
#include <string>
#include <cstdio> // For std::remove and perror
#include <fstream> // For file creation in tests
#include <sys/stat.h> // For mkdir on POSIX systems
#ifdef _WIN32
#include <direct.h> // For _mkdir on Windows
#endif

/**
 * Deletes a file specified by the file path.
 * @param filePath The path to the file to be deleted.
 */
void deleteFile(const std::string& filePath) {
    if (filePath.empty()) {
        std::cerr << "Error: File path cannot be empty." << std::endl;
        return;
    }

    // std::remove returns 0 on success, non-zero on failure
    if (std::remove(filePath.c_str()) == 0) {
        std::cout << "Success: File '" << filePath << "' was deleted successfully." << std::endl;
    } else {
        // perror prints a descriptive error message to stderr
        // It uses the global 'errno' variable set by std::remove on failure
        std::cerr << "Error: Failed to delete file '" << filePath << "'. ";
        perror(""); // Passing "" or NULL prints only the system error message
    }
}

/**
 * Sets up and runs a series of test cases.
 */
void runTestCases() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    const std::string testFile1 = "test1.tmp";
    const std::string testFile2 = "test2.tmp";
    const std::string nonExistentFile = "nonexistent.tmp";
    const std::string alreadyDeletedFile = "test1.tmp";
    const std::string testDir = "testdir.tmp";

    // Setup test files and directory
    std::ofstream(testFile1).close();
    std::ofstream(testFile2).close();
#ifdef _WIN32
    _mkdir(testDir.c_str());
#else
    mkdir(testDir.c_str(), 0777);
#endif

    // Test Case 1: Delete an existing file
    std::cout << "\n[Test Case 1: Delete an existing file]" << std::endl;
    deleteFile(testFile1);

    // Test Case 2: Delete another existing file
    std::cout << "\n[Test Case 2: Delete another existing file]" << std::endl;
    deleteFile(testFile2);

    // Test Case 3: Attempt to delete a non-existent file
    std::cout << "\n[Test Case 3: Attempt to delete a non-existent file]" << std::endl;
    deleteFile(nonExistentFile);

    // Test Case 4: Attempt to delete a file that was already deleted
    std::cout << "\n[Test Case 4: Attempt to delete an already deleted file]" << std::endl;
    deleteFile(alreadyDeletedFile);
    
    // Test Case 5: Attempt to delete a directory
    std::cout << "\n[Test Case 5: Attempt to delete a directory]" << std::endl;
    deleteFile(testDir);

    // Cleanup the test directory
    std::remove(testDir.c_str());

    std::cout << "\n--- Test Cases Finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    runTestCases();

    // --- Main Logic for Command Line ---
    if (argc > 1) {
        std::cout << "\n--- Deleting file from command line argument ---" << std::endl;
        deleteFile(argv[1]);
    } else {
        std::cout << "\nUsage: " << argv[0] << " <file_path_to_delete>" << std::endl;
        std::cout << "Example: " << argv[0] << " my_file.txt" << std::endl;
    }

    return 0;
}