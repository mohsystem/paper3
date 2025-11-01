#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <sstream> // For building the string

/**
 * Executes the 'ls' command with specified arguments by building a command string.
 * WARNING: This function uses std::system(), which executes a command in a subshell.
 * Passing unsanitized user input to this function is extremely dangerous and can
 * lead to command injection vulnerabilities. This code is for demonstrating the
 * described pattern only and should NOT be used in production with untrusted input.
 *
 * @param args A vector of string arguments for the 'ls' command.
 */
void executeLsCommand(const std::vector<std::string>& args) {
    // Use a "print" style mechanism (stringstream) to create the command string.
    std::stringstream ss;
    ss << "ls";
    for (const auto& arg : args) {
        ss << " " << arg;
    }
    std::string command = ss.str();

    std::cout << "--- Executing command: '" << command << "' ---" << std::endl;
    int exit_code = std::system(command.c_str());
    std::cout << "--- Command finished with exit code: " << exit_code << " ---\n" << std::endl;
}

int main() {
    // Test Case 1: ls
    std::cout << ">>> Test Case 1: ls (no arguments)" << std::endl;
    executeLsCommand({});

    // Test Case 2: ls -l
    std::cout << ">>> Test Case 2: ls -l" << std::endl;
    executeLsCommand({"-l"});

    // Test Case 3: ls -a -h
    std::cout << ">>> Test Case 3: ls -a -h" << std::endl;
    executeLsCommand({"-a", "-h"});

    // Test Case 4: ls -l /tmp
    std::cout << ">>> Test Case 4: ls -l /tmp" << std::endl;
    executeLsCommand({"-l", "/tmp"});

    // Test Case 5: ls on a non-existent directory
    std::cout << ">>> Test Case 5: ls /non_existent_dir_12345" << std::endl;
    executeLsCommand({"/non_existent_dir_12345"});

    return 0;
}