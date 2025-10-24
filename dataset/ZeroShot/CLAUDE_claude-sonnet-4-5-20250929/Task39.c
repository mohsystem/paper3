
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

// INSECURE: Vulnerable to command injection
void displayFileContents(const char* filename) {
    char command[512];
    char buffer[256];
    
#ifdef _WIN32
    snprintf(command, sizeof(command), "type %s", filename);
#else
    snprintf(command, sizeof(command), "cat %s", filename);
#endif
    
    FILE* pipe = POPEN(command, "r");
    if (!pipe) {
        printf("Error: Could not execute command\\n");
        return;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    
    PCLOSE(pipe);
}

int main() {
    printf("WARNING: This code is vulnerable to command injection!\\n");
    printf("Test cases:\\n\\n");
    
    const char* testCases[] = {
        "test.txt",
        "document.log",
        "data.csv",
        "config.ini",
        "readme.md"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d: %s\\n", i + 1, testCases[i]);
        displayFileContents(testCases[i]);
        printf("---\\n");
    }
    
    return 0;
}
