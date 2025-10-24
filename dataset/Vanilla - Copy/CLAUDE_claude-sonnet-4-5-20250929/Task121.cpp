
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <dirent.h>
#endif

using namespace std;

const string UPLOAD_DIRECTORY = "uploads";

bool createDirectory(const string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return mkdir(path.c_str(), 0777) == 0;
    }
    return true;
}

bool uploadFileWithContent(const string& fileName, const string& content) {
    try {
        if (!createDirectory(UPLOAD_DIRECTORY)) {
            cout << "Error creating upload directory" << endl;
            return false;
        }
        
        string filePath = UPLOAD_DIRECTORY + "/" + fileName;
        ofstream outFile(filePath);
        
        if (!outFile.is_open()) {
            cout << "Error opening file for writing: " << fileName << endl;
            return false;
        }
        
        outFile << content;
        outFile.close();
        
        cout << "File uploaded successfully: " << fileName << endl;
        return true;
    } catch (const exception& e) {
        cout << "Error uploading file: " << e.what() << endl;
        return false;
    }
}

bool uploadFile(const string& sourceFilePath, const string& destinationFileName) {
    try {
        if (!createDirectory(UPLOAD_DIRECTORY)) {
            cout << "Error creating upload directory" << endl;
            return false;
        }
        
        ifstream source(sourceFilePath, ios::binary);
        if (!source.is_open()) {
            cout << "Source file does not exist: " << sourceFilePath << endl;
            return false;
        }
        
        string destPath = UPLOAD_DIRECTORY + "/" + destinationFileName;
        ofstream dest(destPath, ios::binary);
        
        if (!dest.is_open()) {
            cout << "Error creating destination file" << endl;
            source.close();
            return false;
        }
        
        dest << source.rdbuf();
        source.close();
        dest.close();
        
        cout << "File uploaded successfully: " << destinationFileName << endl;
        return true;
    } catch (const exception& e) {
        cout << "Error uploading file: " << e.what() << endl;
        return false;
    }
}

vector<string> listUploadedFiles() {
    vector<string> files;
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((UPLOAD_DIRECTORY + "/*").c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(findData.cFileName);
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(UPLOAD_DIRECTORY.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {
                files.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

int main() {
    cout << "=== File Upload Program ===" << endl << endl;
    
    // Test Case 1: Upload a file with content
    cout << "Test Case 1: Upload text file" << endl;
    bool result1 = uploadFileWithContent("test1.txt", "Hello, this is test file 1!");
    cout << "Result: " << (result1 ? "true" : "false") << endl << endl;
    
    // Test Case 2: Upload another file with content
    cout << "Test Case 2: Upload document" << endl;
    bool result2 = uploadFileWithContent("document.txt", "This is a sample document with multiple lines.\\nLine 2\\nLine 3");
    cout << "Result: " << (result2 ? "true" : "false") << endl << endl;
    
    // Test Case 3: Upload a file with JSON content
    cout << "Test Case 3: Upload JSON file" << endl;
    bool result3 = uploadFileWithContent("data.json", "{\\"name\\":\\"John\\",\\"age\\":30,\\"city\\":\\"New York\\"}");
    cout << "Result: " << (result3 ? "true" : "false") << endl << endl;
    
    // Test Case 4: Upload a file with special characters
    cout << "Test Case 4: Upload file with special characters" << endl;
    bool result4 = uploadFileWithContent("special.txt", "Special chars: !@#$%^&*()");
    cout << "Result: " << (result4 ? "true" : "false") << endl << endl;
    
    // Test Case 5: List all uploaded files
    cout << "Test Case 5: List all uploaded files" << endl;
    vector<string> uploadedFiles = listUploadedFiles();
    cout << "Uploaded files:" << endl;
    for (const string& file : uploadedFiles) {
        cout << "  - " << file << endl;
    }
    
    return 0;
}
