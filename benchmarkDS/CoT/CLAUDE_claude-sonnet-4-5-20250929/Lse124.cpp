
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        std::cerr << "Error: Exactly one argument is required." << std::endl;
        return 1;
    }
    
    // Get the argument from command line
    std::string input = argv[1];
    
    // Print the argument
    std::cout << "Original: " << input << std::endl;
    
    // Trim trailing whitespace
    size_t end = input.find_last_not_of(" \\t\\n\\r\\f\\v");
    if (end != std::string::npos) {
        input = input.substr(0, end + 1);
    } else {
        input.clear();
    }
    
    // Print the trimmed argument
    std::cout << "Trimmed: " << input << std::endl;
    
    // Exit the program
    return 0;
}
