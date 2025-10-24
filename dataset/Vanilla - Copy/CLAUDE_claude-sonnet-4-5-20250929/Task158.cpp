
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>

std::string executeCommand(const std::string& userInput) {
    // Vulnerable: Direct use of user input in OS command
    std::string command = "echo " + userInput;
    std::array<char, 128> buffer;
    std::string result;
    
    try {
        #ifdef _WIN32
            std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
        #else
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        #endif
        
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // Remove trailing newline if present
        if (!result.empty() && result[result.length()-1] == '\\n') {
            result.erase(result.length()-1);
        }
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
    
    return result;
}

int main() {
    std::cout << "=== OS Command Injection Demo ===\\n\\n";
    
    // Test case 1: Normal input
    std::cout << "Test 1 - Normal input:\\n";
    std::cout << "Input: 'Hello World'\\n";
    std::cout << "Output: " << executeCommand("Hello World") << "\\n\\n";
    
    // Test case 2: Simple text
    std::cout << "Test 2 - Simple text:\\n";
    std::cout << "Input: 'Testing123'\\n";
    std::cout << "Output: " << executeCommand("Testing123") << "\\n\\n";
    
    // Test case 3: Number input
    std::cout << "Test 3 - Number input:\\n";
    std::cout << "Input: '42'\\n";
    std::cout << "Output: " << executeCommand("42") << "\\n\\n";
    
    // Test case 4: Special characters
    std::cout << "Test 4 - Special characters:\\n";
    std::cout << "Input: 'test@example.com'\\n";
    std::cout << "Output: " << executeCommand("test@example.com") << "\\n\\n";
    
    // Test case 5: Multiple words
    std::cout << "Test 5 - Multiple words:\\n";
    std::cout << "Input: 'This is a test'\\n";
    std::cout << "Output: " << executeCommand("This is a test") << "\\n\\n";
    
    return 0;
}
