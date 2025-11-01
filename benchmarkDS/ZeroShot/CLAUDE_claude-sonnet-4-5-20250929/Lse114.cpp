
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>

void executeCommand(const char* directory) {
    char command[256];
    snprintf(command, sizeof(command), "ls %s", directory);
    std::cout << "Executing: " << command << std::endl;
    system(command);
}

int main() {
    // Test cases - INSECURE
    executeCommand("/tmp");
    executeCommand("/home");
    executeCommand("/var");
    executeCommand("/usr");
    executeCommand("/etc");
    return 0;
}
