
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <cstring>

using namespace std;

string readFileFromArgument(const char* filename) {
    if (filename == nullptr || strlen(filename) == 0) {
        return "Error: Filename is null or empty";
    }
    
    string fname(filename);
    
    // Validate filename format
    if (fname.find('\\0') != string::npos || fname.length() > 255) {
        return "Error: Invalid filename format";
    }
    
    // Check if file exists
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        return "Error: File does not exist: " + fname;
    }
    
    // Check if it's a regular file
    if (!S_ISREG(buffer.st_mode)) {
        return "Error: Path is not a file: " + fname;
    }
    
    // Try to read file
    ifstream file(filename);
    if (!file.is_open()) {
        return "Error: File is not readable: " + fname;
    }
    
    try {
        stringstream content;
        content << file.rdbuf();
        file.close();
        return content.str();
    } catch (const exception& e) {
        file.close();
        return "Error: Failed to read file: " + string(e.what());
    }
}

int main() {
    cout << "=== Test Case 1: Valid file ===" << endl;
    ofstream outFile1("test1.txt");
    if (outFile1.is_open()) {
        outFile1 << "Hello World!" << endl << "This is a test file.";
        outFile1.close();
        string result1 = readFileFromArgument("test1.txt");
        cout << result1 << endl;
    }
    
    cout << "\\n=== Test Case 2: Non-existent file ===" << endl;
    string result2 = readFileFromArgument("nonexistent.txt");
    cout << result2 << endl;
    
    cout << "\\n=== Test Case 3: Null filename ===" << endl;
    string result3 = readFileFromArgument(nullptr);
    cout << result3 << endl;
    
    cout << "\\n=== Test Case 4: Empty filename ===" << endl;
    string result4 = readFileFromArgument("");
    cout << result4 << endl;
    
    cout << "\\n=== Test Case 5: Another valid file ===" << endl;
    ofstream outFile2("test2.txt");
    if (outFile2.is_open()) {
        outFile2 << "Line 1" << endl << "Line 2" << endl << "Line 3";
        outFile2.close();
        string result5 = readFileFromArgument("test2.txt");
        cout << result5 << endl;
    }
    
    // Cleanup
    remove("test1.txt");
    remove("test2.txt");
    
    return 0;
}
