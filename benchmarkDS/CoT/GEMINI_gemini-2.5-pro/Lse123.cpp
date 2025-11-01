#include <iostream>
#include <string>
#include <vector>
#include <cctype>

class Lse123 {
public:
    static std::string processString(std::string inputStr) {
        // The argument is already a std::string.
        
        // The for loop trims the trailing whitespace in the string.
        int lastNonWhitespaceIndex = -1;
        for (int i = inputStr.length() - 1; i >= 0; --i) {
            if (!isspace(static_cast<unsigned char>(inputStr[i]))) {
                lastNonWhitespaceIndex = i;
                break;
            }
        }
        if (lastNonWhitespaceIndex == -1) {
            inputStr.clear(); // String was all whitespace
        } else {
            inputStr.resize(lastNonWhitespaceIndex + 1);
        }

        // The second for loop trims the newlines.
        std::string result = "";
        result.reserve(inputStr.length()); // Optimization
        for (char c : inputStr) {
            if (c != '\n' && c != '\r') {
                result += c;
            }
        }
        
        // The last line null terminates the string.
        // std::string objects in C++ are automatically null-terminated
        // when using methods like .c_str(), but this is managed internally.
        return result;
    }
};

int main(int argc, char* argv[]) {
    // The prompt implies command-line argument handling, but also requires
    // 5 test cases in main. We will run the test cases directly.
    // For command-line:
    // if (argc != 2) {
    //     std::cerr << "Error: Exactly one argument is required." << std::endl;
    //     return 1;
    // }
    
    std::vector<std::string> testCases = {
        "  hello world   \t ",
        "test with\n newlines \n and spaces   ",
        "   \n\t\r  ", // all whitespace
        "no_trailing_whitespace",
        "another\r\ncase with a final newline\n"
    };

    std::cout << "Running 5 C++ test cases:" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::string original = testCases[i];
        std::string processed = Lse123::processString(original);
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "  Original:  \"" << original << "\"" << std::endl;
        std::cout << "  Processed: \"" << processed << "\"" << std::endl << std::endl;
    }

    return 0;
}