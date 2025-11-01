#include <iostream>
#include <string>
#include <vector>
#include <cstdio>  // For snprintf
#include <cstdlib> // For system

/**
 * Executes the 'ls' command with the given arguments.
 * This function uses snprintf (a "print" function) to create the command string
 * and the system() function to execute it.
 *
 * @param args A string containing the arguments for the 'ls' command.
 * @return The exit code of the command.
 */
int executeLsCommand(const std::string& args) {
    char command_buffer[512];

    // The snprintf function is used to create the string that is passed to system.
    // It takes a format string and a variable number of arguments.
    snprintf(command_buffer, sizeof(command_buffer), "ls %s", args.c_str());
    
    std::cout << "Executing: " << command_buffer << std::endl;

    // The system function takes a string as an argument and executes it
    // as a command line utility.
    int exit_code = system(command_buffer);
    
    // The return value of system() is implementation-defined, but on POSIX systems,
    // we can use WEXITSTATUS to get the actual exit code.
    #if defined(__linux__) || defined(__APPLE__)
        if (WIFEXITED(exit_code)) {
            return WEXITSTATUS(exit_code);
        }
        return -1; // Indicate that it terminated abnormally
    #else
        return exit_code; // For other systems like Windows
    #endif
}

int main() {
    std::vector<std::string> testCases = {
        "-l",
        "-a .",
        "/dev/null",
        "/tmp",
        "-lh /"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Arguments: '" << testCases[i] << "'" << std::endl;
        int exitCode = executeLsCommand(testCases[i]);
        std::cout << "\nExit Code: " << exitCode << std::endl;
        std::cout << "--- End Test Case " << (i + 1) << " ---\n" << std::endl;
    }

    return 0;
}