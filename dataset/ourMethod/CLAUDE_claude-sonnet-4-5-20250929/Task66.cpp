
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <memory>
#include <array>
#include <algorithm>

// Security: Execute shell command safely with validation
// This function validates the command against a whitelist and executes it
class CommandExecutor {
private:
    // Whitelist of allowed commands to prevent arbitrary code execution
    static const std::vector<std::string> ALLOWED_COMMANDS;
    
    // Maximum output size to prevent memory exhaustion
    static const size_t MAX_OUTPUT_SIZE = 1024 * 1024; // 1MB
    
    // Validate command against whitelist and dangerous patterns
    static bool validateCommand(const std::string& command) {
        // Security: Reject empty commands
        if (command.empty() || command.length() > 1000) {
            return false;
        }
        
        // Security: Check for dangerous characters that could lead to command injection
        const std::string dangerous_chars = ";|&$`<>(){}[]!";
        if (command.find_first_of(dangerous_chars) != std::string::npos) {
            return false;
        }
        
        // Security: Extract the base command (first word)
        size_t space_pos = command.find(' ');
        std::string base_command = (space_pos != std::string::npos) 
            ? command.substr(0, space_pos) 
            : command;
        
        // Security: Check if command is in whitelist
        bool found = std::find(ALLOWED_COMMANDS.begin(), ALLOWED_COMMANDS.end(), 
                               base_command) != ALLOWED_COMMANDS.end();
        
        return found;
    }
    
public:
    // Execute validated command and return output
    static std::string executeCommand(const std::string& command) {
        // Security: Validate command before execution
        if (!validateCommand(command)) {
            return "ERROR: Command not allowed or contains dangerous characters";
        }
        
        // Security: Use popen with "r" mode only (read-only)
        // This prevents writing to the command pipeline
        std::array<char, 128> buffer;
        std::string result;
        
        // Security: Execute command in controlled manner
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return "ERROR: Failed to execute command";
        }
        
        // Security: Read output with size limits to prevent memory exhaustion
        try {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
                
                // Security: Enforce maximum output size
                if (result.size() > MAX_OUTPUT_SIZE) {
                    pclose(pipe);
                    return "ERROR: Command output exceeds maximum size limit";
                }
            }
        } catch (...) {
            pclose(pipe);
            return "ERROR: Exception during command execution";
        }
        
        // Security: Check return code
        int return_code = pclose(pipe);
        if (return_code != 0) {
            return "ERROR: Command execution failed with code " + 
                   std::to_string(return_code) + "\\n" + result;
        }
        
        return result;
    }
};

// Security: Whitelist of safe commands only
const std::vector<std::string> CommandExecutor::ALLOWED_COMMANDS = {
    "echo", "date", "pwd", "whoami", "hostname", "uname", "ls"
};

int main() {
    std::cout << "=== Secure Command Executor ===" << std::endl;
    std::cout << "Allowed commands: echo, date, pwd, whoami, hostname, uname, ls" << std::endl;
    std::cout << "Note: Commands with special characters (;|&$`<>(){}[]!) are blocked" << std::endl;
    std::cout << std::endl;
    
    // Test case 1: Valid echo command
    std::cout << "Test 1 - echo command:" << std::endl;
    std::string cmd1 = "echo Hello World";
    std::cout << "Input: " << cmd1 << std::endl;
    std::cout << "Output: " << CommandExecutor::executeCommand(cmd1) << std::endl;
    
    // Test case 2: Valid date command
    std::cout << "Test 2 - date command:" << std::endl;
    std::string cmd2 = "date";
    std::cout << "Input: " << cmd2 << std::endl;
    std::cout << "Output: " << CommandExecutor::executeCommand(cmd2) << std::endl;
    
    // Test case 3: Invalid command (not in whitelist)
    std::cout << "Test 3 - invalid command (not whitelisted):" << std::endl;
    std::string cmd3 = "rm -rf test";
    std::cout << "Input: " << cmd3 << std::endl;
    std::cout << "Output: " << CommandExecutor::executeCommand(cmd3) << std::endl;
    
    // Test case 4: Command injection attempt with semicolon
    std::cout << "Test 4 - command injection attempt:" << std::endl;
    std::string cmd4 = "echo test; rm -rf /";
    std::cout << "Input: " << cmd4 << std::endl;
    std::cout << "Output: " << CommandExecutor::executeCommand(cmd4) << std::endl;
    
    // Test case 5: Command with pipe attempt
    std::cout << "Test 5 - pipe injection attempt:" << std::endl;
    std::string cmd5 = "echo test | cat";
    std::cout << "Input: " << cmd5 << std::endl;
    std::cout << "Output: " << CommandExecutor::executeCommand(cmd5) << std::endl;
    
    return 0;
}
