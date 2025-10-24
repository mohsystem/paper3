
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>

using namespace std;

class Task69 {
public:
    static string deleteFile(const string& filePath) {
        if (filePath.empty()) {
            return "Error: File path cannot be empty";
        }
        
        try {
            // Check if file exists
            struct stat buffer;
            if (stat(filePath.c_str(), &buffer) != 0) {
                return "Error: File does not exist: " + filePath;
            }
            
            // Check if it's a regular file
            if (!S_ISREG(buffer.st_mode)) {
                return "Error: Path is not a file: " + filePath;
            }
            
            // Try to delete the file
            if (remove(filePath.c_str()) == 0) {
                return "Success: File deleted successfully: " + filePath;
            } else {
                return "Error: Failed to delete file: " + filePath;
            }
        } catch (const exception& e) {
            return string("Error: ") + e.what();
        }
    }
};

int main(int argc, char* argv[]) {
    // Test cases
    cout << "Test Case 1: Empty file path" << endl;
    cout << Task69::deleteFile("") << endl;
    cout << endl;
    
    cout << "Test Case 2: Non-existent file" << endl;
    cout << Task69::deleteFile("nonexistent_file.txt") << endl;
    cout << endl;
    
    cout << "Test Case 3: Create and delete a test file" << endl;
    string testFile = "test_file_to_delete.txt";
    ofstream outFile(testFile);
    if (outFile.is_open()) {
        outFile << "test content";
        outFile.close();
        cout << Task69::deleteFile(testFile) << endl;
    } else {
        cout << "Error creating test file" << endl;
    }
    cout << endl;
    
    cout << "Test Case 4: Invalid path with special characters" << endl;
    cout << Task69::deleteFile("../../../etc/passwd") << endl;
    cout << endl;
    
    cout << "Test Case 5: Command line argument" << endl;
    if (argc > 1) {
        cout << Task69::deleteFile(argv[1]) << endl;
    } else {
        cout << "No command line argument provided" << endl;
    }
    
    return 0;
}
