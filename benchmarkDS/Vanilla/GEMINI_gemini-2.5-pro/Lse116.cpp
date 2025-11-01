#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <cstdio>  // For snprintf()

/**
 * @brief Executes a system command.
 * 
 * @param command The command to execute.
 * @return The exit code of the command.
 */
int executeCommand(const std::string& command) {
    // The system() function passes the command string to the host
    // environment's command processor (e.g., /bin/sh on Linux).
    int exit_code = system(command.c_str());
    return exit_code;
}

int main() {
    std::vector<std::string> testCommands = {
        "ls",
        "ls -l",
        "ls -a",
        "ls -al",
        "ls non_existent_directory"
    };

    for (size_t i = 0; i < testCommands.size(); ++i) {
        // Using snprintf to demonstrate creating a formatted string, as per the prompt.
        char header[256];
        snprintf(header, sizeof(header), "--- Test Case %zu: Executing '%s' ---", i + 1, testCommands[i].c_str());
        std::cout << header << std::endl;

        int exit_code = executeCommand(testCommands[i]);
        
        // The return value of system() is implementation-defined, but on POSIX systems,
        // we can use WEXITSTATUS to get the actual exit code if the command terminated normally.
        // For simplicity here, we print the raw value returned by system().
        std::cout << "--- Command exited with raw status: " << exit_code << " ---\n" << std::endl;
    }

    return 0;
}