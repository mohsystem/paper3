#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Executes an OS command using user-provided input.
 * This function is vulnerable to command injection because it uses the system()
 * call, which passes the constructed command string directly to the system's shell.
 *
 * @param userInput The data provided by the user, intended to be a hostname or IP.
 */
void executeCommand(const char* userInput) {
    printf("--- Executing with input: \"%s\" ---\n", userInput);
    
    // Allocate a buffer to hold the full command string.
    // The size should be large enough to prevent buffer overflows.
    char command[512]; 
    
#ifdef _WIN32
    // Windows-specific ping command.
    // snprintf is used for safe string formatting to prevent buffer overflows.
    snprintf(command, sizeof(command), "ping -n 1 %s", userInput);
#else
    // Unix-like specific ping command.
    snprintf(command, sizeof(command), "ping -c 1 %s", userInput);
#endif

    printf("Executing command: %s\n", command);

    // The system() function is a primary source of command injection vulnerabilities.
    system(command);
    
    printf("--------------------------------------------------\n\n");
}

int main() {
    // 5 test cases to demonstrate the functionality and vulnerability
    const char* testCases[] = {
        "google.com",                       // Case 1: Valid hostname
        "127.0.0.1",                        // Case 2: Valid IP address
        "invalidhostname",                  // Case 3: Invalid input
        "8.8.8.8; whoami",                  // Case 4: Command injection for Unix-like systems
        "8.8.8.8 & ver"                     // Case 5: Command injection for Windows
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        executeCommand(testCases[i]);
    }
    
    return 0;
}