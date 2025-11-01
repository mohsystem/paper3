
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>

void executeCommand(const char* command) {
    system(command);
}

void executeCommandFormatted(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    system(buffer);
}

int main() {
    // Test case 1: Execute ls command
    executeCommand("ls");
    
    // Test case 2: Execute ls with -l flag
    executeCommand("ls -l");
    
    // Test case 3: Execute ls with -a flag using formatted string
    executeCommandFormatted("ls %s", "-a");
    
    // Test case 4: Execute ls with multiple parameters
    executeCommandFormatted("ls %s %s", "-l", "-h");
    
    // Test case 5: Execute ls for specific directory
    executeCommandFormatted("ls %s", "/tmp");
    
    return 0;
}
