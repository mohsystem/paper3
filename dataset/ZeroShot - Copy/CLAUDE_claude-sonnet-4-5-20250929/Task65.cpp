
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

using namespace std;

const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

string readFileSecurely(const string& filename) {
    if (filename.empty()) {
        return "Error: Filename cannot be null or empty";
    }
    
    // Validate filename format (no path traversal)
    if (filename.find("..") != string::npos || 
        filename.find("/") != string::npos || 
        filename.find("\\\\") != string::npos) {\n        return "Error: Invalid filename format - path traversal detected";\n    }\n    \n    // Validate filename characters\n    regex validPattern("^[a-zA-Z0-9._-]+$");\n    if (!regex_match(filename, validPattern)) {\n        return "Error: Filename contains invalid characters";\n    }\n    \n    // Check if file exists and get stats\n    struct stat fileStat;\n    if (stat(filename.c_str(), &fileStat) != 0) {\n        return "Error: File does not exist: " + filename;\n    }\n    \n    // Check if it's a regular file\n    if (!S_ISREG(fileStat.st_mode)) {\n        return "Error: Path is not a regular file: " + filename;\n    }\n    \n    // Check file size\n    if (fileStat.st_size > MAX_FILE_SIZE) {\n        return "Error: File size exceeds maximum allowed size";\n    }\n    \n    // Get canonical path\n    char currentDir[PATH_MAX];\n    char filePath[PATH_MAX];\n    \n    if (getcwd(currentDir, sizeof(currentDir)) == NULL) {\n        return "Error: Could not get current directory";\n    }\n    \n    if (realpath(filename.c_str(), filePath) == NULL) {\n        return "Error: Could not resolve file path";\n    }\n    \n    // Ensure file is in current directory\n    string filePathStr(filePath);\n    string currentDirStr(currentDir);\n    if (filePathStr.find(currentDirStr) != 0) {\n        return "Error: File access outside working directory is not allowed";\n    }\n    \n    // Read file content\n    ifstream file(filename);\n    if (!file.is_open()) {\n        return "Error: Failed to open file: " + filename;\n    }\n    \n    stringstream buffer;\n    buffer << file.rdbuf();\n    file.close();\n    \n    return buffer.str();\n}\n\nint main() {\n    cout << "=== Test Case 1: Valid file ===" << endl;\n    ofstream testFile("test1.txt");\n    if (testFile.is_open()) {\n        testFile << "Hello World!" << endl;\n        testFile << "This is a test file." << endl;\n        testFile.close();\n        cout << readFileSecurely("test1.txt") << endl;\n    } else {\n        cout << "Could not create test file" << endl;\n    }\n    \n    cout << "\
=== Test Case 2: Non-existent file ===" << endl;\n    cout << readFileSecurely("nonexistent.txt") << endl;\n    \n    cout << "\
=== Test Case 3: Path traversal attempt ===" << endl;\n    cout << readFileSecurely("../etc/passwd") << endl;\n    \n    cout << "\
=== Test Case 4: Invalid characters ===" << endl;\n    cout << readFileSecurely("test@#$.txt") << endl;\n    \n    cout << "\
=== Test Case 5: Empty filename ===" << endl;\n    cout << readFileSecurely("") << endl;
    
    return 0;
}
