
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/types.h>
#endif

using namespace std;

bool fileExists(const string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

bool createDirectory(const string& path) {
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

string getFileName(const string& path) {
    size_t pos = path.find_last_of("/\\\\");\n    if (pos != string::npos) {\n        return path.substr(pos + 1);\n    }\n    return path;\n}\n\nstring readAndWriteFile(const string& sourceFilePath, const string& tempDirectory) {\n    try {\n        // Validate input parameters\n        if (sourceFilePath.empty()) {\n            cerr << "Invalid argument: Source file path cannot be empty" << endl;\n            return "";\n        }\n        if (tempDirectory.empty()) {\n            cerr << "Invalid argument: Temp directory cannot be empty" << endl;\n            return "";\n        }\n        \n        // Check if source file exists\n        if (!fileExists(sourceFilePath)) {\n            cerr << "File not found error: Source file does not exist: " << sourceFilePath << endl;\n            return "";\n        }\n        if (isDirectory(sourceFilePath)) {\n            cerr << "Invalid argument: Source path is not a file: " << sourceFilePath << endl;\n            return "";\n        }\n        \n        // Create temp directory if it doesn't exist\n        if (!fileExists(tempDirectory)) {\n            if (!createDirectory(tempDirectory)) {\n                cerr << "IO error: Failed to create temp directory: " << tempDirectory << endl;\n                return "";\n            }\n        }\n        \n        // Create temp file path\n        string fileName = getFileName(sourceFilePath);\n        string tempFilePath = tempDirectory + "/temp_" + to_string(time(nullptr)) + "_" + fileName;\n        \n        // Read from source\n        ifstream reader(sourceFilePath.c_str());\n        if (!reader.is_open()) {\n            cerr << "IO error: Cannot open source file: " << sourceFilePath << endl;\n            return "";\n        }\n        \n        // Write to temp\n        ofstream writer(tempFilePath.c_str());\n        if (!writer.is_open()) {\n            reader.close();\n            cerr << "IO error: Cannot create temp file: " << tempFilePath << endl;\n            return "";\n        }\n        \n        string line;\n        while (getline(reader, line)) {\n            writer << line << endl;\n        }\n        \n        reader.close();\n        writer.close();\n        \n        return tempFilePath;\n        \n    } catch (const exception& e) {\n        cerr << "Unexpected error: " << e.what() << endl;\n        return "";\n    } catch (...) {\n        cerr << "Unexpected error occurred" << endl;\n        return "";\n    }\n}\n\nint main() {\n    cout << "Testing File Read and Write with Exception Handling\
" << endl;\n    \n    // Test Case 1: Valid file and temp directory\n    cout << "Test Case 1: Valid file and temp directory" << endl;\n    {\n        ofstream testFile("test1.txt");\n        testFile << "Hello World!" << endl << "This is test file 1." << endl;\n        testFile.close();\n        \n        string result = readAndWriteFile("test1.txt", "./temp");\n        cout << "Result: " << (result.empty() ? "Failed" : "Success - " + result) << endl;\n        remove("test1.txt");\n    }\n    cout << endl;\n    \n    // Test Case 2: Non-existent source file\n    cout << "Test Case 2: Non-existent source file" << endl;\n    string result2 = readAndWriteFile("nonexistent.txt", "./temp");\n    cout << "Result: " << (result2.empty() ? "Failed (Expected)" : "Success - " + result2) << endl;\n    cout << endl;\n    \n    // Test Case 3: Empty source file path\n    cout << "Test Case 3: Empty source file path" << endl;\n    string result3 = readAndWriteFile("", "./temp");\n    cout << "Result: " << (result3.empty() ? "Failed (Expected)" : "Success - " + result3) << endl;\n    cout << endl;\n    \n    // Test Case 4: Empty temp directory\n    cout << "Test Case 4: Empty temp directory" << endl;\n    {\n        ofstream testFile("test4.txt");\n        testFile << "Test content" << endl;\n        testFile.close();\n        \n        string result4 = readAndWriteFile("test4.txt", "");\n        cout << "Result: " << (result4.empty() ? "Failed (Expected)" : "Success - " + result4) << endl;\n        remove("test4.txt");\n    }\n    cout << endl;\n    \n    // Test Case 5: Valid file with special characters\n    cout << "Test Case 5: Valid file with special characters" << endl;\n    {\n        ofstream testFile("test5.txt");\n        testFile << "Special chars: @#$%^&*()" << endl;\n        testFile.close();\n        \n        string result5 = readAndWriteFile("test5.txt", "./temp");\n        cout << "Result: " << (result5.empty() ? "Failed" : "Success - " + result5) << endl;\n        remove("test5.txt");
    }
    cout << endl;
    
    return 0;
}
