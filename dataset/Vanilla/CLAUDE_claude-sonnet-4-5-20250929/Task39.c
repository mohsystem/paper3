
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void displayFileContents(const char* filename) {
    char command[512];
    
    #ifdef _WIN32
        snprintf(command, sizeof(command), "type \\"%s\\"", filename);
    #else
        snprintf(command, sizeof(command), "cat \\"%s\\"", filename);
    #endif
    
    int result = system(command);
    
    if (result != 0) {
        fprintf(stderr, "Error executing command\\n");
    }
}

void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    }
}

int main() {
    // Create test files
    const char* testFiles[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    const char* testContents[] = {
        "Hello World!",
        "This is test file 2.\\nWith multiple lines.",
        "Test file 3 contents",
        "Line 1\\nLine 2\\nLine 3",
        "Final test file"
    };
    
    int i;
    
    // Create test files
    for (i = 0; i < 5; i++) {
        createTestFile(testFiles[i], testContents[i]);
    }
    
    // Test cases
    for (i = 0; i < 5; i++) {
        printf("\\n--- Test Case %d: %s ---\\n", i + 1, testFiles[i]);
        displayFileContents(testFiles[i]);
    }
    
    // Clean up test files
    for (i = 0; i < 5; i++) {
        remove(testFiles[i]);
    }
    
    return 0;
}
