#include <iostream>
#include <string>
#include <cstdlib>

/**
 * Executes an OS command using user-provided input.
 * This function is vulnerable to command injection because it uses the system()
 * call, which passes the constructed command string directly to the system's shell.
 *
 * @param userInput The data provided by the user, intended to be a hostname or IP.
 */
void executeCommand(const std::string& userInput) {
    std::cout << "--- Executing with input: \"" << userInput << "\" ---" << std::endl;
    
    std::string command;
    
#ifdef _WIN32
    // Windows-specific ping command
    command = "ping -n 1 " + userInput;
#else
    // Unix-like specific ping command
    command = "ping -c 1 " + userInput;
#endif

    std::cout << "Executing command: " << command << std::endl;

    // The system() function is a primary source of command injection vulnerabilities.
    system(command.c_str());

    std::cout << "--------------------------------------------------" << std::endl << std::endl;
}

int main() {
    // 5 test cases to demonstrate the functionality and vulnerability
    std::string testCases[] = {
        "google.com",                       // Case 1: Valid hostname
        "127.0.0.1",                        // Case 2: Valid IP address
        "invalidhostname",                  // Case 3: Invalid input
        "8.8.8.8; whoami",                  // Case 4: Command injection for Unix-like systems
        "8.8.8.8 & ver"                     // Case 5: Command injection for Windows
    };

    for (const auto& testCase : testCases) {
        executeCommand(testCase);
    }

    return 0;
}