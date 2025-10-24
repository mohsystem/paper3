
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

using namespace std;

class Task50 {
public:
    static const string UPLOAD_DIR;
    
    class FileUploadServer {
    public:
        FileUploadServer() {
            // Create upload directory if it doesn't exist\n            MKDIR(Task50::UPLOAD_DIR.c_str());\n        }\n        \n        string handleFileUpload(const string& fileName, const vector<unsigned char>& fileData) {\n            try {\n                string filePath = Task50::UPLOAD_DIR + "/" + fileName;\n                ofstream file(filePath, ios::binary);\n                \n                if (!file.is_open()) {\n                    return "ERROR: Failed to create file";\n                }\n                \n                file.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());\n                file.close();\n                \n                return "SUCCESS: File '" + fileName + "' uploaded successfully. Size: " + \n                       to_string(fileData.size()) + " bytes";\n            } catch (const exception& e) {\n                return string("ERROR: Failed to upload file - ") + e.what();\n            }\n        }\n        \n        bool fileExists(const string& fileName) {\n            string filePath = Task50::UPLOAD_DIR + "/" + fileName;\n            ifstream file(filePath);\n            return file.good();\n        }\n    };\n    \n    class FileUploadClient {\n    public:\n        string uploadFile(const string& fileName, const vector<unsigned char>& fileData, \n                         FileUploadServer& server) {\n            return server.handleFileUpload(fileName, fileData);\n        }\n    };\n};\n\nconst string Task50::UPLOAD_DIR = "uploads";\n\nint main() {\n    cout << "File Upload System - Test Cases\
" << endl;\n    \n    Task50::FileUploadServer server;\n    Task50::FileUploadClient client;\n    \n    // Test Case 1: Upload a text file\n    cout << "Test Case 1: Upload text file" << endl;\n    string content1 = "Hello, this is a test file!";\n    vector<unsigned char> data1(content1.begin(), content1.end());\n    string result1 = client.uploadFile("test1.txt", data1, server);\n    cout << result1 << endl;\n    cout << "File exists: " << (server.fileExists("test1.txt") ? "true" : "false") << endl;\n    cout << endl;\n    \n    // Test Case 2: Upload a binary file\n    cout << "Test Case 2: Upload binary file" << endl;\n    vector<unsigned char> binaryData = {0x48, 0x65, 0x6C, 0x6C, 0x6F};\n    string result2 = client.uploadFile("test2.bin", binaryData, server);\n    cout << result2 << endl;\n    cout << "File exists: " << (server.fileExists("test2.bin") ? "true" : "false") << endl;\n    cout << endl;\n    \n    // Test Case 3: Upload a larger file\n    cout << "Test Case 3: Upload larger file" << endl;\n    string largeContent = "";\n    for (int i = 0; i < 1000; i++) {\n        largeContent += "Line " + to_string(i) + ": This is test data.\
";\n    }\n    vector<unsigned char> data3(largeContent.begin(), largeContent.end());\n    string result3 = client.uploadFile("test3.log", data3, server);\n    cout << result3 << endl;\n    cout << "File exists: " << (server.fileExists("test3.log") ? "true" : "false") << endl;\n    cout << endl;\n    \n    // Test Case 4: Upload file with special characters in name\n    cout << "Test Case 4: Upload file with special characters" << endl;\n    string content4 = "Special file content";\n    vector<unsigned char> data4(content4.begin(), content4.end());\n    string result4 = client.uploadFile("test_file-4.txt", data4, server);\n    cout << result4 << endl;\n    cout << "File exists: " << (server.fileExists("test_file-4.txt") ? "true" : "false") << endl;\n    cout << endl;\n    \n    // Test Case 5: Upload empty file\n    cout << "Test Case 5: Upload empty file" << endl;\n    vector<unsigned char> emptyData;\n    string result5 = client.uploadFile("empty.txt", emptyData, server);\n    cout << result5 << endl;\n    cout << "File exists: " << (server.fileExists("empty.txt") ? "true" : "false") << endl;
    cout << endl;
    
    return 0;
}
