
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <sstream>
#include <array>
#include <memory>
#include <cstdio>

class Task66 {
private:
    static const std::set<std::string> ALLOWED_COMMANDS;
    
    static std::vector<std::string> splitString(const std::string& str) {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    static bool hasDangerousCharacters(const std::string& command) {
        std::regex dangerous_pattern(".*[;&|`$(){}\\\\[\\\\]<>].*|.*\\\\.\\\\..*");
        return std::regex_match(command, dangerous_pattern);
    }
    
public:
    static std::string executeCommand(const std::string& command) {
        if (command.empty() || command.find_first_not_of(' ') == std::string::npos) {
            return "Error: Command cannot be empty";
        }
        
        std::string trimmedCommand = command;
        trimmedCommand.erase(0, trimmedCommand.find_first_not_of(' '));
        trimmedCommand.erase(trimmedCommand.find_last_not_of(' ') + 1);
        
        // Check for dangerous characters
        if (hasDangerousCharacters(trimmedCommand)) {
            return "Error: Command contains dangerous characters";
        }
        
        // Parse command
        std::vector<std::string> parts = splitString(trimmedCommand);
        if (parts.empty()) {
            return "Error: Command cannot be empty";
        }
        
        std::string baseCommand = parts[0];
        
        // Validate against whitelist
        if (ALLOWED_COMMANDS.find(baseCommand) == ALLOWED_COMMANDS.end()) {
            return "Error: Command not allowed. Allowed commands: ls, pwd, date, whoami, echo";
        }
        
        // Validate arguments
        for (size_t i = 1; i < parts.size(); i++) {
            if (parts[i].find('/') != std::string::npos || 
                parts[i].find('\\\\') != std::string::npos) {\n                return "Error: Path traversal not allowed in arguments";\n            }\n        }\n        \n        // Execute command\n        std::string fullCommand = trimmedCommand + " 2>&1";\n        std::array<char, 128> buffer;\n        std::string result;\n        \n        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);\n        if (!pipe) {\n            return "Error: Failed to execute command";\n        }\n        \n        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {\n            result += buffer.data();\n        }\n        \n        // Remove trailing newline\n        if (!result.empty() && result.back() == '\
') {\n            result.pop_back();\n        }\n        \n        return result;\n    }\n};\n\nconst std::set<std::string> Task66::ALLOWED_COMMANDS = {"ls", "pwd", "date", "whoami", "echo"};\n\nint main() {\n    std::cout << "=== Test Case 1: Valid command 'pwd' ===" << std::endl;\n    std::cout << Task66::executeCommand("pwd") << std::endl << std::endl;\n    \n    std::cout << "=== Test Case 2: Valid command 'date' ===" << std::endl;\n    std::cout << Task66::executeCommand("date") << std::endl << std::endl;\n    \n    std::cout << "=== Test Case 3: Valid command 'echo' with arguments ===" << std::endl;\n    std::cout << Task66::executeCommand("echo Hello World") << std::endl << std::endl;\n    \n    std::cout << "=== Test Case 4: Invalid command with dangerous characters ===" << std::endl;\n    std::cout << Task66::executeCommand("ls; rm -rf /") << std::endl << std::endl;\n    \n    std::cout << "=== Test Case 5: Unauthorized command ===" << std::endl;\n    std::cout << Task66::executeCommand("rm testfile.txt") << std::endl << std::endl;
    
    return 0;
}
