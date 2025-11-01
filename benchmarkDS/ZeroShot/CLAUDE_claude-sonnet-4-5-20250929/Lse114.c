
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void executeCommand(const char* directory) {
    char command[256];
    snprintf(command, sizeof(command), "ls %s", directory);
    printf("Executing: %s\\n", command);
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
