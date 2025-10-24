
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <sys/stat.h>

using namespace std;

class Task100 {
public:
    static string searchPatternInFile(const string& pattern, const string& filename) {
        string result;
        
        try {
            // Validate input parameters
            if (pattern.empty()) {
                return "Error: Pattern cannot be empty";
            }
            if (filename.empty()) {
                return "Error: Filename cannot be empty";
            }
            
            // Check if file exists and is readable
            struct stat buffer;
            if (stat(filename.c_str(), &buffer) != 0) {
                return "Error: File does not exist";
            }
            
            // Check file size (max 10MB)
            if (buffer.st_size > 10 * 1024 * 1024) {
                return "Error: File too large (max 10MB)";
            }
            
            // Compile the regular expression
            regex regexPattern;
            try {
                regexPattern = regex(pattern);
            } catch (const regex_error& e) {
                return "Error: Invalid regex pattern - " + string(e.what());
            }
            
            // Read file content
            ifstream file(filename);
            if (!file.is_open()) {
                return "Error: Cannot open file";
            }
            
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            
            // Search for matches
            auto words_begin = sregex_iterator(content.begin(), content.end(), regexPattern);
            auto words_end = sregex_iterator();
            
            int matchCount = 0;
            for (sregex_iterator i = words_begin; i != words_end && matchCount < 1000; ++i) {
                smatch match = *i;
                result += "Match found at position " + to_string(match.position()) + 
                         ": " + match.str() + "\\n";
                matchCount++;
            }
            
            if (matchCount == 0) {
                result += "No matches found\\n";
            } else {
                result += "Total matches: " + to_string(matchCount) + "\\n";
            }
            
        } catch (const exception& e) {
            return "Error: " + string(e.what());
        }
        
        return result;
    }
    
private:
    static void createTestFile(const string& filename, const string& content) {
        ofstream file(filename);
        if (file.is_open()) {
            file << content;
            file.close();
        }
    }
    
    static void deleteTestFile(const string& filename) {
        remove(filename.c_str());
    }
    
public:
    static void runTests() {
        cout << "=== Test Case 1: Search for word pattern ===" << endl;
        createTestFile("test1.txt", "Hello world! Hello Java programming.");
        cout << searchPatternInFile("Hello", "test1.txt") << endl;
        
        cout << "=== Test Case 2: Search for email pattern ===" << endl;
        createTestFile("test2.txt", "Contact: user@example.com or admin@test.org");
        cout << searchPatternInFile(R"(\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b)", "test2.txt") << endl;
        
        cout << "=== Test Case 3: Search for digit pattern ===" << endl;
        createTestFile("test3.txt", "The numbers are 123, 456, and 789.");
        cout << searchPatternInFile(R"(\\d+)", "test3.txt") << endl;
        
        cout << "=== Test Case 4: Invalid pattern ===" << endl;
        cout << searchPatternInFile("[invalid(", "test1.txt") << endl;
        
        cout << "=== Test Case 5: Non-existent file ===" << endl;
        cout << searchPatternInFile("test", "nonexistent.txt") << endl;
        
        // Cleanup
        deleteTestFile("test1.txt");
        deleteTestFile("test2.txt");
        deleteTestFile("test3.txt");
    }
};

int main() {
    Task100::runTests();
    return 0;
}
