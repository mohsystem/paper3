
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <sys/stat.h>

using namespace std;

class Task39 {
public:
    static string displayFileContents(const string& filename) {
        if (filename.empty()) {
            return "Error: Filename cannot be empty";
        }
        
        // Sanitize input - remove dangerous characters
        string sanitized = filename;
        string dangerous = ";&|`$(){}[]<>'\\"";
        for (char c : dangerous) {
            sanitized.erase(remove(sanitized.begin(), sanitized.end(), c), sanitized.end());
        }
        
        // Prevent path traversal
        if (sanitized.find("..") != string::npos) {
            return "Error: Path traversal detected";
        }
        
        // Check if file exists
        struct stat buffer;
        if (stat(sanitized.c_str(), &buffer) != 0) {
            return "Error: File does not exist";
        }
        
        // Check if it's a regular file
        if (!S_ISREG(buffer.st_mode)) {
            return "Error: Not a valid file";
        }
        
        // Read file securely (no system command)
        ifstream file(sanitized);
        if (!file.is_open()) {
            return "Error: Cannot open file";
        }
        
        stringstream content;
        content << "Contents of " << filename << ":\\n";
        content << "-----------------------------------\\n";
        
        string line;
        while (getline(file, line)) {
            content << line << "\\n";
        }
        
        file.close();
        return content.str();
    }
};

int main() {
    // Test case 1: Valid text file
    cout << "Test 1:" << endl;
    cout << Task39::displayFileContents("test1.txt") << endl;
    
    // Test case 2: Empty filename
    cout << "\\nTest 2:" << endl;
    cout << Task39::displayFileContents("") << endl;
    
    // Test case 3: Path traversal attempt
    cout << "\\nTest 3:" << endl;
    cout << Task39::displayFileContents("../../../etc/passwd") << endl;
    
    // Test case 4: Command injection attempt
    cout << "\\nTest 4:" << endl;
    cout << Task39::displayFileContents("test.txt; rm -rf /") << endl;
    
    // Test case 5: Non-existent file
    cout << "\\nTest 5:" << endl;
    cout << Task39::displayFileContents("nonexistent.txt") << endl;
    
    return 0;
}
