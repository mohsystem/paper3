
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>

void processInput(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        std::cerr << "Error: Exactly one argument required" << std::endl;
        exit(1);
    }
    
    // Get the argument from command line
    std::string input = argv[1];
    
    // Print the argument
    std::cout << "Original input: " << input << std::endl;
    
    // Trim trailing whitespace
    size_t end = input.find_last_not_of(" \\t\\n\\r\\f\\v");
    if (end != std::string::npos) {
        input = input.substr(0, end + 1);
    } else {
        input = "";
    }
    
    // Print the argument again
    std::cout << "Trimmed input: " << input << std::endl;
}

int main() {
    // Test cases
    const char* testCases[][2] = {
        {"prog", "Hello"},
        {"prog", "World   "},
        {"prog", "Test\\t\\t"},
        {"prog", "NoWhitespace"},
        {"prog", "Multiple Words  "}
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        char* argv[2];
        argv[0] = const_cast<char*>(testCases[i][0]);
        argv[1] = const_cast<char*>(testCases[i][1]);
        processInput(2, argv);
        std::cout << std::endl;
    }
    
    return 0;
}
