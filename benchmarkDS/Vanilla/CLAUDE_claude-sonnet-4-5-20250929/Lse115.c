
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char* executeCommand(const char* command) {
    static char result[4096];
    result[0] = '\\0';
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        strcpy(result, "Error executing command");
        return result;
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strcat(result, buffer);
    }
    pclose(pipe);
    return result;
}

void formatCommand(char* output, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(output, 256, format, args);
    va_end(args);
}

int main() {
    char cmd[256];

    // Test case 1: List current directory
    formatCommand(cmd, "ls %s", ".");
    printf("Test 1: %s\\n", cmd);
    printf("%s\\n", executeCommand(cmd));

    // Test case 2: List with -l flag
    formatCommand(cmd, "ls %s", "-l");
    printf("Test 2: %s\\n", cmd);
    printf("%s\\n", executeCommand(cmd));

    // Test case 3: List with -a flag
    formatCommand(cmd, "ls %s", "-a");
    printf("Test 3: %s\\n", cmd);
    printf("%s\\n", executeCommand(cmd));

    // Test case 4: List with -la flags
    formatCommand(cmd, "ls %s %s", "-l", "-a");
    printf("Test 4: %s\\n", cmd);
    printf("%s\\n", executeCommand(cmd));

    // Test case 5: List specific directory
    formatCommand(cmd, "ls %s", "/tmp");
    printf("Test 5: %s\\n", cmd);
    printf("%s\\n", executeCommand(cmd));

    return 0;
}
