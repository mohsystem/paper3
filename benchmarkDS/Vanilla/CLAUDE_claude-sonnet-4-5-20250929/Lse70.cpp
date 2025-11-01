
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#endif

using namespace std;

class Lse70 {
private:
    static const string UPLOAD_DIR;

public:
    static void createUploadDirectory() {
        #ifdef _WIN32
        mkdir(UPLOAD_DIR.c_str());
        #else
        mkdir(UPLOAD_DIR.c_str(), 0777);
        #endif
    }

    static bool upload_file(const string& filename, const string& content) {
        createUploadDirectory();
        
        string filepath = UPLOAD_DIR + "/" + filename;
        ofstream outFile(filepath, ios::binary);
        
        if (!outFile) {
            cerr << "Error: Could not open file for writing: " << filepath << endl;
            return false;
        }
        
        outFile.write(content.c_str(), content.length());
        outFile.close();
        
        cout << "File uploaded successfully: " << filename << endl;
        return true;
    }

    static void runTests() {
        cout << "Running 5 test cases for file upload..." << endl;
        
        string testCases[5] = {
            "test_file_1.txt",
            "test_file_2.txt",
            "test_file_3.txt",
            "test_file_4.txt",
            "test_file_5.txt"
        };
        
        string testContents[5] = {
            "This is test file 1 content.",
            "This is test file 2 content.",
            "This is test file 3 content.",
            "This is test file 4 content.",
            "This is test file 5 content."
        };
        
        for (int i = 0; i < 5; i++) {
            cout << "\\nTest case " << (i + 1) << ": ";
            if (upload_file(testCases[i], testContents[i])) {
                cout << "SUCCESS" << endl;
            } else {
                cout << "FAILED" << endl;
            }
        }
    }
};

const string Lse70::UPLOAD_DIR = "uploads";

int main() {
    cout << "File Upload Application" << endl;
    cout << "Upload directory: " << Lse70::UPLOAD_DIR << endl << endl;
    
    Lse70::runTests();
    
    return 0;
}
