
#include <iostream>
#include <string>
#include <algorithm>

std::string processAndPrint(const std::string& input) {
    std::string trimmed = input;
    
    // Remove trailing whitespace
    size_t end = trimmed.find_last_not_of(" \\t\\n\\r\\f\\v");
    if (end != std::string::npos) {
        trimmed = trimmed.substr(0, end + 1);
    } else {
        trimmed = "";
    }
    
    std::cout << trimmed << std::endl;
    return trimmed;
}

int main(int argc, char* argv[]) {
    // Test case 1: String with trailing spaces
    std::cout << "Test 1:" << std::endl;
    processAndPrint("Hello World   ");
    
    // Test case 2: String with trailing tabs
    std::cout << "\\nTest 2:" << std::endl;
    processAndPrint("Hello World\\t\\t");
    
    // Test case 3: String with mixed trailing whitespace
    std::cout << "\\nTest 3:" << std::endl;
    processAndPrint("Hello World \\t \\n");
    
    // Test case 4: String with no trailing whitespace
    std::cout << "\\nTest 4:" << std::endl;
    processAndPrint("Hello World");
    
    // Test case 5: Empty string
    std::cout << "\\nTest 5:" << std::endl;
    processAndPrint("");
    
    // If command line arguments are provided
    if (argc > 1) {
        std::cout << "\\nCommand line argument:" << std::endl;
        processAndPrint(argv[1]);
    }
    
    return 0;
}
