#include <iostream>
#include <string>
#include <cstdio>  // For snprintf
#include <cstdlib> // For system()

/**
 * The "print function" (snprintf) is used to create the string
 * that is passed to the system function (system).
 * @param format The format string for the command (e.g., "ls %s").
 * @param arg The single argument to be formatted into the string.
 */
void executeCommand(const char* format, const char* arg) {
    char command[256];
    
    // Use snprintf to safely create the command string from the format and argument.
    if (arg != nullptr && std::string(arg).length() > 0) {
        snprintf(command, sizeof(command), format, arg);
    } else {
        // Handle case with no arguments by copying the format string directly.
        snprintf(command, sizeof(command), "%s", format);
    }

    std::cout << "--- Executing command: " << command << " ---" << std::endl;
    
    // The system function executes the command string.
    // The output is printed directly to the console by the command itself.
    int return_code = system(command);
    
    std::cout << "Exited with code: " << return_code << std::endl;
    std::cout << "-----------------------------------------" << std::endl << std::endl;
}

/**
 * Main function with 5 test cases.
 */
int main() {
    // Test Case 1: simple ls
    executeCommand("ls", "");

    // Test Case 2: ls with -l flag
    executeCommand("ls %s", "-l");

    // Test Case 3: ls with -a flag
    executeCommand("ls %s", "-a");

    // Test Case 4: ls a specific existing path
    executeCommand("ls %s", ".");

    // Test Case 5: ls a non-existent path (should produce an error)
    executeCommand("ls %s", "/non_existent_directory_xyz123");
    
    return 0;
}