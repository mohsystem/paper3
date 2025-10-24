
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <cstring>
#include <array>
#include <memory>
#include <stdexcept>
#include <algorithm>

class Task66 {
private:
    static const std::set<std::string> ALLOWED_COMMANDS;
    static const int TIMEOUT_SECONDS = 5;
    static const int MAX_OUTPUT_LENGTH = 10000;
    
    static std::vector<std::string> splitCommand(const std::string& cmd) {
        std::vector<std::string> parts;
        std::istringstream iss(cmd);
        std::string part;
        while (iss >> part) {
            parts.push_back(part);
        }
        return parts;
    }
    
    static bool containsForbiddenChars(const std::string& cmd) {
        const std::string forbidden = ";|&><`$\\\\\\n\\r";
        return cmd.find_first_of(forbidden) != std::string::npos;
    }
    
public:
    static std::string executeCommand(const std::string& userInput) {
        if (userInput.empty()) {
            return "Error: Command cannot be empty";
        }
        
        std::string trimmed = userInput;
        trimmed.erase(0, trimmed.find_first_not_of(" \\t\\n\\r"));
        trimmed.erase(trimmed.find_last_not_of(" \\t\\n\\r") + 1);
        
        if (trimmed.empty()) {
            return "Error: Command cannot be empty";
        }
        
        // Validate: no dangerous characters
        if (containsForbiddenChars(trimmed)) {
            return "Error: Command contains forbidden characters";
        }
        
        // Parse command
        std::vector<std::string> parts = splitCommand(trimmed);
        if (parts.empty()) {
            return "Error: No command provided";
        }
        
        std::string command = parts[0];
        
        // Whitelist validation
        if (ALLOWED_COMMANDS.find(command) == ALLOWED_COMMANDS.end()) {
            return "Error: Command '" + command + "' is not in the allowed list";
        }
        
        // Execute command using popen (safer than system())
        std::string fullCommand = trimmed;
        std::array<char, 128> buffer;
        std::string result;
        
        try {
            FILE* pipe = popen(fullCommand.c_str(), "r");
            if (!pipe) {
                return "Error: Failed to execute command";
            }
            
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
                if (result.length() > MAX_OUTPUT_LENGTH) {
                    result = result.substr(0, MAX_OUTPUT_LENGTH) + "\\n[Output truncated]";
                    break;
                }
            }
            
            int status = pclose(pipe);
            
            if (result.empty()) {
                return "Command executed successfully (no output)";
            }
            
            return result;
            
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    static void runTests() {
        std::cout << "=== Secure Command Executor Test Cases ===\\n\\n";
        
        std::vector<std::string> testCases = {
            "ls",
            "pwd",
            "echo Hello World",
            "ls; rm -rf /",
            "cat /etc/passwd"
        };
        
        for (size_t i = 0; i < testCases.size(); i++) {
            std::cout << "Test Case " << (i + 1) << ": " << testCases[i] << "\\n";
            std::cout << "Result: " << executeCommand(testCases[i]) << "\\n";
            std::cout << "---\\n";
        }
    }
};

const std::set<std::string> Task66::ALLOWED_COMMANDS = {"ls", "pwd", "date", "whoami", "echo"};

int main() {
    Task66::runTests();
    return 0;
}
