
#include <iostream>
#include <string>
#include <cstdlib>
#include <limits>

int* convertStringToInteger(const std::string& input) {
    try {
        size_t pos = 0;
        int value = std::stoi(input, &pos);
        
        // Check if entire string was converted
        while (pos < input.length() && std::isspace(input[pos])) {
            pos++;
        }
        
        if (pos != input.length()) {
            std::cout << "Error: Invalid input - cannot convert '" << input << "' to integer" << std::endl;
            return nullptr;
        }
        
        int* result = new int(value);
        return result;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid input - cannot convert '" << input << "' to integer" << std::endl;
        return nullptr;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: Number out of range - '" << input << "'" << std::endl;
        return nullptr;
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {"123", "-456", "0", "abc", "12.34"};
    
    std::cout << "Running test cases:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "Input: '" << testCases[i] << "' -> ";
        int* result = convertStringToInteger(testCases[i]);
        if (result != nullptr) {
            std::cout << "Output: " << *result << std::endl;
            delete result;
        }
    }
    
    // Command line argument processing
    if (argc > 1) {
        std::cout << "\\nCommand line argument:" << std::endl;
        std::cout << "Input: '" << argv[1] << "' -> ";
        int* result = convertStringToInteger(argv[1]);
        if (result != nullptr) {
            std::cout << "Output: " << *result << std::endl;
            delete result;
        }
    } else {
        std::cout << "\\nNo command line arguments provided." << std::endl;
    }
    
    return 0;
}
