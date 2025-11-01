
#include <cstdio>
#include <cstdlib>
#include <iostream>

// WARNING: Using system() is inherently dangerous and should be avoided.
// This example uses a hardcoded command only. Never pass user input to system().
// For production code, use safer alternatives like fork/exec or platform-specific APIs.

void executeCommand() {
    // system() executes via shell and is vulnerable to command injection
    // Only safe when the command is completely hardcoded with no user input
    const char* command = "ls";
    
    // Execute the hardcoded command
    int result = system(command);
    
    if (result == -1) {
        std::cerr << "Error: Failed to execute command" << std::endl;
    } else {
        std::cout << "Command exited with code: " << (result >> 8) << std::endl;
    }
}

int main() {
    // Test cases - all execute the same hardcoded safe command
    std::cout << "Test 1:" << std::endl;
    executeCommand();
    
    std::cout << "\\nTest 2:" << std::endl;
    executeCommand();
    
    std::cout << "\\nTest 3:" << std::endl;
    executeCommand();
    
    std::cout << "\\nTest 4:" << std::endl;
    executeCommand();
    
    std::cout << "\\nTest 5:" << std::endl;
    executeCommand();
    
    return 0;
}
