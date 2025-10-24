
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <array>
#include <memory>
#include <cstdio>
#include <algorithm>

class Task158 {
private:
    static const std::vector<std::string> ALLOWED_COMMANDS;
    static const std::regex SAFE_PATTERN;
    
    static bool isCommandAllowed(const std::string& command) {
        return std::find(ALLOWED_COMMANDS.begin(), ALLOWED_COMMANDS.end(), command) != ALLOWED_COMMANDS.end();
    }
    
    static bool isArgumentSafe(const std::string& argument) {
        return argument.empty() || std::regex_match(argument, SAFE_PATTERN);
    }
    
public:
    static std::string executeCommand(const std::string& command, const std::string& argument = "") {
        // Validate command is in whitelist
        if (!isCommandAllowed(command)) {
            return "Error: Command not allowed. Allowed commands: echo, date, whoami";
        }
        
        // Validate argument (no special characters for shell injection)
        if (!isArgumentSafe(argument)) {
            return "Error: Invalid argument. Only alphanumeric characters, dots, hyphens, and underscores allowed.";
        }
        
        // Build safe command string
        std::string fullCommand;
        if (!argument.empty()) {
            fullCommand = command + " " + argument + " 2>&1";
        } else {
            fullCommand = command + " 2>&1";
        }
        
        // Execute command safely
        std::array<char, 128> buffer;
        std::string result;
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);
        
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // Remove trailing newline
        if (!result.empty() && result.back() == '\\n') {
            result.pop_back();
        }
        
        return result;
    }
};

const std::vector<std::string> Task158::ALLOWED_COMMANDS = {"echo", "date", "whoami"};
const std::regex Task158::SAFE_PATTERN("^[a-zA-Z0-9._-]+$");

int main() {
    std::cout << "=== Secure OS Command Execution Demo ===\\n\\n";
    
    // Test Case 1: Valid echo command with safe argument
    std::cout << "Test 1 - Echo command:\\n";
    std::cout << Task158::executeCommand("echo", "Hello_World") << "\\n\\n";
    
    // Test Case 2: Date command (no argument)
    std::cout << "Test 2 - Date command:\\n";
    std::cout << Task158::executeCommand("date", "") << "\\n\\n";
    
    // Test Case 3: Invalid command (not in whitelist)
    std::cout << "Test 3 - Invalid command:\\n";
    std::cout << Task158::executeCommand("rm", "file.txt") << "\\n\\n";
    
    // Test Case 4: Command injection attempt (should be blocked)
    std::cout << "Test 4 - Injection attempt:\\n";
    std::cout << Task158::executeCommand("echo", "test; rm -rf /") << "\\n\\n";
    
    // Test Case 5: Whoami command
    std::cout << "Test 5 - Whoami command:\\n";
    std::cout << Task158::executeCommand("whoami", "") << "\\n\\n";
    
    return 0;
}
