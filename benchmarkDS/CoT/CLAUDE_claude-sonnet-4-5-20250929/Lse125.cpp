
#include <iostream>
#include <string>
#include <cstring>

std::string processArgument(const char* arg) {
    if (arg == nullptr) {
        return "";
    }
    
    std::string str(arg);
    // Trim trailing whitespace
    size_t end = str.find_last_not_of(" \\t\\n\\r\\f\\v");
    if (end != std::string::npos) {
        str.erase(end + 1);
    } else {
        str.clear();
    }
    
    return str;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {
        "Hello World   ",
        "NoSpaces",
        "Trailing\\t\\t",
        "   LeadingAndTrailing   ",
        ""
    };
    
    std::cout << "Test cases:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string result = processArgument(testCases[i]);
        std::cout << "Test " << (i + 1) << ": [" << result << "]" << std::endl;
    }
    
    // Process command line argument if provided
    if (argc > 1) {
        std::string trimmed = processArgument(argv[1]);
        std::cout << "\\nCommand line argument: [" << trimmed << "]" << std::endl;
    } else {
        std::cout << "\\nNo command line argument provided" << std::endl;
    }
    
    return 0;
}
