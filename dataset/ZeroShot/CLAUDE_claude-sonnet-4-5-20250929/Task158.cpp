
#include <iostream>
#include <string>
#include <set>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <regex>

class Task158 {
private:
    static const std::set<std::string> ALLOWED_COMMANDS;
    
    static std::string sanitizeInput(const std::string& input) {
        // Remove potentially dangerous characters
        std::regex dangerous_chars(R"([;&|`$()<>\\\\'"])");\n        return std::regex_replace(input, dangerous_chars, "");\n    }\n    \npublic:\n    static std::string executeCommand(const std::string& command, const std::string& argument) {\n        try {\n            // Validate command against whitelist\n            if (ALLOWED_COMMANDS.find(command) == ALLOWED_COMMANDS.end()) {\n                return "Error: Command not allowed. Allowed commands: echo, date, pwd, whoami, hostname";\n            }\n            \n            // Sanitize argument\n            std::string sanitizedArg = sanitizeInput(argument);\n            \n            // Build command string safely\n            std::string fullCommand = command;\n            if (!sanitizedArg.empty()) {\n                fullCommand += " " + sanitizedArg;\n            }\n            fullCommand += " 2>&1";  // Redirect stderr to stdout\n            \n            // Execute command using popen\n            std::array<char, 128> buffer;\n            std::string result;\n            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);\n            \n            if (!pipe) {\n                return "Error: Failed to execute command";\n            }\n            \n            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {\n                result += buffer.data();\n            }\n            \n            // Remove trailing newline\n            if (!result.empty() && result.back() == '\\n') {\n                result.pop_back();\n            }\n            \n            return result;\n        } catch (const std::exception& e) {\n            return std::string("Error: ") + e.what();\n        }\n    }\n};\n\nconst std::set<std::string> Task158::ALLOWED_COMMANDS = {\n    "echo", "date", "pwd", "whoami", "hostname"\n};\n\nint main() {\n    std::cout << "=== Secure OS Command Execution ===" << std::endl << std::endl;\n    \n    // Test case 1: Valid echo command\n    std::cout << "Test 1 - Echo command:" << std::endl;\n    std::cout << Task158::executeCommand("echo", "Hello World") << std::endl << std::endl;\n    \n    // Test case 2: Date command\n    std::cout << "Test 2 - Date command:" << std::endl;\n    std::cout << Task158::executeCommand("date", "") << std::endl << std::endl;\n    \n    // Test case 3: Invalid command (blocked)\n    std::cout << "Test 3 - Invalid command (should be blocked):" << std::endl;\n    std::cout << Task158::executeCommand("rm", "-rf /") << std::endl << std::endl;\n    \n    // Test case 4: Command injection attempt (sanitized)\n    std::cout << "Test 4 - Command injection attempt:" << std::endl;\n    std::cout << Task158::executeCommand("echo", "test; rm -rf /") << std::endl << std::endl;\n    \n    // Test case 5: Hostname command\n    std::cout << "Test 5 - Hostname command:" << std::endl;\n    std::cout << Task158::executeCommand("hostname", "") << std::endl;
    
    return 0;
}
