
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <regex>
#include <cstdio>
#include <cstring>

// Security: Pattern to validate username
const std::regex USERNAME_PATTERN("^[a-zA-Z0-9._-]{1,32}$");

/**
 * Executes getent passwd command for a given username.
 * Uses popen with validated input to prevent command injection.
 * Validates username format before execution.
 * 
 * @param username The username to query
 * @return The output from getent passwd command, or error message
 */
std::string getUserInfo(const std::string& username) {
    // Security: Validate username is not empty
    if (username.empty()) {
        return "Error: Username cannot be empty";
    }
    
    // Security: Validate username format to prevent command injection
    if (!std::regex_match(username, USERNAME_PATTERN)) {
        return "Error: Invalid username format";
    }
    
    // Security: Build command with validated input only
    // Using separate arguments would be safer but popen doesn't support it directly\n    std::string command = "getent passwd ";\n    command += username;  // Safe because username is validated\n    \n    // Security: Use popen in read mode only\n    FILE* pipe = popen(command.c_str(), "r");\n    if (!pipe) {\n        return "Error: Failed to execute command";\n    }\n    \n    std::string result;\n    std::array<char, 256> buffer;\n    const size_t MAX_OUTPUT_SIZE = 4096; // Security: Limit output size\n    \n    // Security: Read with bounds checking\n    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {\n        result += buffer.data();\n        \n        // Security: Enforce maximum output size to prevent DoS\n        if (result.size() > MAX_OUTPUT_SIZE) {\n            result = result.substr(0, MAX_OUTPUT_SIZE);\n            break;\n        }\n    }\n    \n    // Clean up\n    int status = pclose(pipe);\n    if (status == -1) {\n        return "Error: Failed to close pipe";\n    }\n    \n    // Check command exit status\n    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {\n        return "Error: Command failed with exit code " + std::to_string(WEXITSTATUS(status));\n    }\n    \n    return result;\n}\n\nint main() {\n    // Test case 1: Valid username\n    std::cout << "Test 1 - Valid user 'root':" << std::endl;\n    std::cout << getUserInfo("root") << std::endl;\n    \n    // Test case 2: Valid username\n    std::cout << "\\nTest 2 - Valid user 'daemon':" << std::endl;\n    std::cout << getUserInfo("daemon") << std::endl;\n    \n    // Test case 3: Non-existent user\n    std::cout << "\
Test 3 - Non-existent user:" << std::endl;\n    std::cout << getUserInfo("nonexistentuser12345") << std::endl;\n    \n    // Test case 4: Invalid username with shell metacharacter (injection attempt)\n    std::cout << "\
Test 4 - Invalid username (injection attempt):" << std::endl;\n    std::cout << getUserInfo("root; rm -rf /") << std::endl;\n    \n    // Test case 5: Empty username\n    std::cout << "\
Test 5 - Empty username:" << std::endl;\n    std::cout << getUserInfo("") << std::endl;
    
    return 0;
}
