
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>

void searchPatternInFile(const std::string& patternStr, const std::string& fileName) {
    try {
        std::regex pattern(patternStr);
        std::ifstream file(fileName);
        
        if (!file.is_open()) {
            std::cout << "Error reading file: File not found - " << fileName << std::endl;
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        
        std::cout << "Searching for pattern: " << patternStr << " in file: " << fileName << std::endl;
        
        auto matches_begin = std::sregex_iterator(content.begin(), content.end(), pattern);
        auto matches_end = std::sregex_iterator();
        
        int count = 0;
        for (std::sregex_iterator i = matches_begin; i != matches_end; ++i) {
            count++;
            std::smatch match = *i;
            std::cout << "Match " << count << ": " << match.str() 
                      << " at position " << match.position() << std::endl;
        }
        
        if (count == 0) {
            std::cout << "No matches found." << std::endl;
        } else {
            std::cout << "Total matches: " << count << std::endl;
        }
    } catch (const std::regex_error& e) {
        std::cout << "Invalid regex pattern: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        searchPatternInFile(argv[1], argv[2]);
    } else {
        std::cout << "Running test cases...\\n" << std::endl;
        
        // Create test files
        std::ofstream file1("test1.txt");
        file1 << "Hello World! Hello Java!";
        file1.close();
        
        std::ofstream file2("test2.txt");
        file2 << "abc123def456ghi789";
        file2.close();
        
        std::ofstream file3("test3.txt");
        file3 << "email@example.com test@domain.org";
        file3.close();
        
        std::ofstream file4("test4.txt");
        file4 << "The quick brown fox jumps over the lazy dog";
        file4.close();
        
        std::ofstream file5("test5.txt");
        file5 << "Line1\\nLine2\\nLine3";
        file5.close();
        
        // Test case 1
        std::cout << "Test 1:" << std::endl;
        searchPatternInFile("Hello", "test1.txt");
        std::cout << std::endl;
        
        // Test case 2
        std::cout << "Test 2:" << std::endl;
        searchPatternInFile("\\\\d+", "test2.txt");
        std::cout << std::endl;
        
        // Test case 3
        std::cout << "Test 3:" << std::endl;
        searchPatternInFile("[a-z]+@[a-z]+\\\\.[a-z]+", "test3.txt");
        std::cout << std::endl;
        
        // Test case 4
        std::cout << "Test 4:" << std::endl;
        searchPatternInFile("\\\\b\\\\w{3}\\\\b", "test4.txt");
        std::cout << std::endl;
        
        // Test case 5
        std::cout << "Test 5:" << std::endl;
        searchPatternInFile("Line\\\\d", "test5.txt");
        std::cout << std::endl;
    }
    
    return 0;
}
