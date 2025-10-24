
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdio>
#include <algorithm>
#include <cctype>

// Security: Validate input to contain only alphanumeric characters and basic safe characters
// This prevents command injection attacks by rejecting shell metacharacters
bool isValidInput(const std::string& input) {
    if (input.empty() || input.length() > 255) {
        return false; // Reject empty or excessively long inputs
    }
    
    // Only allow alphanumeric characters, dash, underscore, and dot
    // This whitelist approach prevents injection of shell metacharacters like ;|&$`()
    for (char c : input) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && 
            c != '-' && c != '_' && c != '.') {
            return false;
        }
    }
    
    // Additional check: prevent directory traversal
    if (input.find("..") != std::string::npos) {
        return false;
    }
    
    return true;
}

// Security: Execute command safely using popen with validated input
// Returns the command output or error message
std::string executeCommand(const std::string& userInput) {
    // Security: Validate input before using in command
    if (!isValidInput(userInput)) {
        return "Error: Invalid input. Only alphanumeric characters, dash, underscore, and dot allowed.";
    }
    
    // Security: Use a fixed command template with the validated input appended
    // Using echo as a safe demonstration command that doesn't modify system state
    std::string command = "echo \\"Processing: " + userInput + "\\"";
    
    // Security: Use popen in read mode only to prevent input injection
    std::array<char, 128> buffer;
    std::string result;
    
    // Security: popen is used here but with strictly validated input
    // The command is constructed safely without shell metacharacters
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "Error: Failed to execute command";
    }
    
    // Security: Read output with bounds checking
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
        // Security: Limit output size to prevent memory exhaustion
        if (result.length() > 4096) {
            result += "\\n[Output truncated]";
            break;
        }
    }
    
    return result;
}

int main() {
    std::cout << "Secure Command Execution Demo\\n";
    std::cout << "==============================\\n\\n";
    
    // Test case 1: Valid alphanumeric input
    std::string test1 = "testfile123";
    std::cout << "Test 1 - Valid input: " << test1 << "\\n";
    std::cout << "Result: " << executeCommand(test1) << "\\n\\n";
    
    // Test case 2: Valid input with allowed characters
    std::string test2 = "my-file_name.txt";
    std::cout << "Test 2 - Valid filename: " << test2 << "\\n";
    std::cout << "Result: " << executeCommand(test2) << "\\n\\n";
    
    // Test case 3: Invalid input with shell metacharacter (injection attempt)
    std::string test3 = "file; rm -rf /";
    std::cout << "Test 3 - Injection attempt: " << test3 << "\\n";
    std::cout << "Result: " << executeCommand(test3) << "\\n\\n";
    
    // Test case 4: Invalid input with pipe character
    std::string test4 = "test|cat /etc/passwd";
    std::cout << "Test 4 - Pipe injection attempt: " << test4 << "\\n";
    std::cout << "Result: " << executeCommand(test4) << "\\n\\n";
    
    // Test case 5: Invalid input with directory traversal
    std::string test5 = "../../../etc/passwd";
    std::cout << "Test 5 - Directory traversal attempt: " << test5 << "\\n";
    std::cout << "Result: " << executeCommand(test5) << "\\n\\n";
    
    return 0;
}
