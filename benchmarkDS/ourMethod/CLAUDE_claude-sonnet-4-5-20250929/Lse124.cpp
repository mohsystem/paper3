
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

// Function to trim trailing whitespace
std::string trimTrailing(const std::string& str) {
    size_t end = str.find_last_not_of(" \\t\\n\\r\\f\\v");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

void processArgument(int argc, char* argv[]) {
    // First if statement: check that exactly one argument is provided
    // argc includes program name, so we check for argc == 2
    if (argc != 2) {
        std::cerr << "Error: exactly one argument required" << std::endl;
        std::exit(1);
    }
    
    // Second line: get the argument from command line and store in variable called input
    // Input validation: check for null pointer and reasonable length
    if (argv[1] == nullptr) {
        std::cerr << "Error: invalid argument" << std::endl;
        std::exit(1);
    }
    
    size_t argLen = std::strlen(argv[1]);
    // Limit input size to prevent memory exhaustion (max 10KB)
    if (argLen > 10240) {
        std::cerr << "Error: argument too long" << std::endl;
        std::exit(1);
    }
    
    std::string input(argv[1]);
    
    // Third line: print the argument
    // std::cout safely handles the string without injection concerns
    std::cout << input << std::endl;
    
    // Fourth line: trim trailing whitespace from the argument
    input = trimTrailing(input);
    
    // Fifth line: print the argument again
    std::cout << input << std::endl;
    
    // Sixth line: exit the program
    std::exit(0);
}

int main(int argc, char* argv[]) {
    // Test cases - create test argv arrays
    // Test 1: Normal input
    char* test1[] = {(char*)"program", (char*)"hello"};
    // processArgument(2, test1);
    
    // Test 2: Input with trailing spaces
    char* test2[] = {(char*)"program", (char*)"hello   "};
    // processArgument(2, test2);
    
    // Test 3: Input with trailing tabs
    char* test3[] = {(char*)"program", (char*)"world\\t\\t"};
    // processArgument(2, test3);
    
    // Test 4: Input with mixed trailing whitespace
    char* test4[] = {(char*)"program", (char*)"test \\t \\n"};
    // processArgument(2, test4);
    
    // Test 5: Empty string
    char* test5[] = {(char*)"program", (char*)""};
    // processArgument(2, test5);
    
    // Run with actual command line arguments
    processArgument(argc, argv);
    
    return 0;
}
