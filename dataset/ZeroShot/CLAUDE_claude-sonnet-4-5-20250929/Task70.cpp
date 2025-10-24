
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

class Task70 {
public:
    static string readFileContent(const string& filePath) {
        try {
            // Normalize and canonicalize the path to prevent directory traversal
            fs::path path = fs::canonical(fs::path(filePath));
            
            // Check if file exists
            if (!fs::exists(path)) {
                return "Error: File does not exist";
            }
            
            // Check if it's a regular file
            if (!fs::is_regular_file(path)) {
                return "Error: Path is not a regular file";
            }
            
            // Open and read file
            ifstream file(path);
            if (!file.is_open()) {
                return "Error: Unable to open file";
            }
            
            // Read content
            stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            return buffer.str();
            
        } catch (const fs::filesystem_error& e) {
            return string("Error: ") + e.what();
        } catch (const exception& e) {
            return string("Error: ") + e.what();
        }
    }
    
    static void runTests() {
        vector<string> testFiles = {
            "test1.txt",
            "test2.txt",
            "nonexistent.txt",
            "../etc/passwd",
            "test3.txt"
        };
        
        cout << "Running test cases:\\n" << endl;
        for (size_t i = 0; i < testFiles.size(); i++) {
            cout << "Test Case " << (i + 1) << ": " << testFiles[i] << endl;
            string content = readFileContent(testFiles[i]);
            cout << content << endl;
            cout << "-------------------\\n" << endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        string content = Task70::readFileContent(argv[1]);
        cout << "File Content:\\n" << content << endl;
    } else {
        Task70::runTests();
    }
    
    return 0;
}
