
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>

using namespace std;

string readFileContents(const string& filePath) {
    stringstream content;
    
    // Check if file exists and is a regular file
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        return "Error: File does not exist";
    }
    if (!S_ISREG(fileInfo.st_mode)) {
        return "Error: Path is not a file";
    }
    
    // Try to open and read the file
    ifstream file(filePath);
    if (!file.is_open()) {
        return "Error: Cannot open file";
    }
    
    if (!file.good()) {
        return "Error: File is not readable";
    }
    
    string line;
    while (getline(file, line)) {
        content << line << "\\n";
    }
    
    file.close();
    return content.str();
}

int main() {
    // Create test files
    ofstream testFile1("test1.txt");
    testFile1 << "Hello World!\\nThis is test file 1.";
    testFile1.close();
    
    ofstream testFile2("test2.txt");
    testFile2 << "Line 1\\nLine 2\\nLine 3";
    testFile2.close();
    
    ofstream testFile3("test3.txt");
    testFile3 << "Single line file";
    testFile3.close();
    
    ofstream testFile4("test4.txt");
    testFile4 << "";
    testFile4.close();
    
    ofstream testFile5("test5.txt");
    testFile5 << "Test file 5\\nWith multiple\\nLines of text";
    testFile5.close();
    
    // Test cases
    cout << "Test Case 1:" << endl;
    cout << readFileContents("test1.txt") << endl;
    
    cout << "Test Case 2:" << endl;
    cout << readFileContents("test2.txt") << endl;
    
    cout << "Test Case 3:" << endl;
    cout << readFileContents("test3.txt") << endl;
    
    cout << "Test Case 4 (empty file):" << endl;
    cout << readFileContents("test4.txt") << endl;
    
    cout << "Test Case 5:" << endl;
    cout << readFileContents("test5.txt") << endl;
    
    // Cleanup
    remove("test1.txt");
    remove("test2.txt");
    remove("test3.txt");
    remove("test4.txt");
    remove("test5.txt");
    
    return 0;
}
