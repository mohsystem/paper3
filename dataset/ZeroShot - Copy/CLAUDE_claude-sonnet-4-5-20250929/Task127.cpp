
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <cstring>
#include <sstream>

using namespace std;

class Task127 {
public:
    static string getTempDir() {
        const char* tmpDir = getenv("TMPDIR");
        if (tmpDir == nullptr) {
            tmpDir = getenv("TEMP");
        }
        if (tmpDir == nullptr) {
            tmpDir = getenv("TMP");
        }
        if (tmpDir == nullptr) {
            tmpDir = "/tmp";
        }
        return string(tmpDir);
    }
    
    static string generateUUID() {
        stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << hex << (rand() % 16);
        }
        return ss.str();
    }
    
    static bool fileExists(const string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }
    
    static bool isRegularFile(const string& path) {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) {
            return false;
        }
        return S_ISREG(buffer.st_mode);
    }
    
    static string readAndWriteFile(const string& sourceFilePath) {
        if (sourceFilePath.empty()) {
            cerr << "Error: Source file path cannot be empty" << endl;
            return "";
        }
        
        try {
            // Validate source file
            if (!fileExists(sourceFilePath)) {
                cerr << "Error: File not found - " << sourceFilePath << endl;
                return "";
            }
            
            if (!isRegularFile(sourceFilePath)) {
                cerr << "Error: Path is not a file - " << sourceFilePath << endl;
                return "";
            }
            
            // Open source file
            ifstream sourceFile(sourceFilePath.c_str());
            if (!sourceFile.is_open()) {
                cerr << "Error: Cannot open source file - " << sourceFilePath << endl;
                return "";
            }
            
            // Create temporary file path
            string tempDir = getTempDir();
            string uniqueId = generateUUID();
            string tempFilePath = tempDir + "/temp_" + uniqueId + ".txt";
            
            // Open temporary file
            ofstream tempFile(tempFilePath.c_str());
            if (!tempFile.is_open()) {
                sourceFile.close();
                cerr << "Error: Cannot create temporary file" << endl;
                return "";
            }
            
            // Copy content
            string line;
            while (getline(sourceFile, line)) {
                tempFile << line << endl;
            }
            
            // Close files
            sourceFile.close();
            tempFile.close();
            
            return tempFilePath;
            
        } catch (const exception& e) {
            cerr << "Error: Exception occurred - " << e.what() << endl;
            return "";
        } catch (...) {
            cerr << "Error: Unknown exception occurred" << endl;
            return "";
        }
    }
};

int main() {
    srand(time(nullptr));
    
    cout << "=== File Read/Write Error Handling Test Cases ===" << endl << endl;
    
    string tempDir = Task127::getTempDir();
    
    // Test Case 1: Valid file
    try {
        string testFile1 = tempDir + "/test_input_1.txt";
        ofstream ofs(testFile1.c_str());
        if (ofs.is_open()) {
            ofs << "This is test content line 1" << endl;
            ofs << "This is test content line 2" << endl;
            ofs.close();
            
            cout << "Test Case 1: Valid file" << endl;
            string result = Task127::readAndWriteFile(testFile1);
            cout << "Result: " << (result.empty() ? "Failed" : "Success - " + result) << endl;
            cout << endl;
        }
    } catch (const exception& e) {
        cout << "Test Case 1 setup failed: " << e.what() << endl << endl;
    }
    
    // Test Case 2: Non-existent file
    cout << "Test Case 2: Non-existent file" << endl;
    string result2 = Task127::readAndWriteFile("/non/existent/file.txt");
    cout << "Result: " << (result2.empty() ? "Failed as expected" : "Success") << endl;
    cout << endl;
    
    // Test Case 3: Empty file path
    cout << "Test Case 3: Empty file path" << endl;
    string result3 = Task127::readAndWriteFile("");
    cout << "Result: " << (result3.empty() ? "Failed as expected" : "Success") << endl;
    cout << endl;
    
    // Test Case 4: Directory instead of file
    cout << "Test Case 4: Directory instead of file" << endl;
    string result4 = Task127::readAndWriteFile(tempDir);
    cout << "Result: " << (result4.empty() ? "Failed as expected" : "Success") << endl;
    cout << endl;
    
    // Test Case 5: Create another valid file
    try {
        string testFile5 = tempDir + "/test_input_5.txt";
        ofstream ofs(testFile5.c_str());
        if (ofs.is_open()) {
            ofs << "Test Case 5 content" << endl;
            ofs.close();
            
            cout << "Test Case 5: Another valid file" << endl;
            string result = Task127::readAndWriteFile(testFile5);
            cout << "Result: " << (result.empty() ? "Failed" : "Success - " + result) << endl;
            cout << endl;
        }
    } catch (const exception& e) {
        cout << "Test Case 5 setup failed: " << e.what() << endl << endl;
    }
    
    return 0;
}
