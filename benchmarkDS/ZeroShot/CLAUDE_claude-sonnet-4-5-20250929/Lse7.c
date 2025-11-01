
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processCommandLineArguments(char** args, int argc, int index) {
    if (args == NULL || argc == 0) {
        printf("No arguments provided\\n");
        return;
    }
    
    if (index < 0 || index >= argc) {
        printf("Index out of bounds\\n");
        return;
    }
    
    printf("%s\\n", args[index]);
}

int main() {
    // Test case 1: Normal case
    char* test1[] = {"arg0", "2", "arg2", "arg3"};
    printf("Test 1:\\n");
    int index1 = atoi(test1[1]);
    processCommandLineArguments(test1, 4, index1);
    
    // Test case 2: Index at boundary
    char* test2[] = {"arg0", "0", "arg2"};
    printf("\\nTest 2:\\n");
    int index2 = atoi(test2[1]);
    processCommandLineArguments(test2, 3, index2);
    
    // Test case 3: Last element
    char* test3[] = {"arg0", "3", "arg2", "arg3"};
    printf("\\nTest 3:\\n");
    int index3 = atoi(test3[1]);
    processCommandLineArguments(test3, 4, index3);
    
    // Test case 4: Out of bounds
    char* test4[] = {"arg0", "5", "arg2"};
    printf("\\nTest 4:\\n");
    int index4 = atoi(test4[1]);
    processCommandLineArguments(test4, 3, index4);
    
    // Test case 5: Empty array handling
    printf("\\nTest 5:\\n");
    processCommandLineArguments(NULL, 0, 0);
    
    return 0;
}
