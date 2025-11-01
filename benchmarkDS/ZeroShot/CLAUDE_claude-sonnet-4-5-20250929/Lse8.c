
#include <stdio.h>
#include <string.h>

const char* getValueAtIndex(char** args, int argc, int index) {
    if (args == NULL || index < 0 || index >= argc) {
        return NULL;
    }
    return args[index];
}

int main() {
    // Test case 1: Normal case with valid index
    char* test1[] = {"arg0", "arg1", "arg2", "arg3"};
    const char* result1 = getValueAtIndex(test1, 4, 1);
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    
    // Test case 2: Index 0
    char* test2[] = {"first", "second", "third"};
    const char* result2 = getValueAtIndex(test2, 3, 0);
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    
    // Test case 3: Last index
    char* test3[] = {"a", "b", "c", "d", "e"};
    const char* result3 = getValueAtIndex(test3, 5, 4);
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    
    // Test case 4: Out of bounds index
    char* test4[] = {"x", "y", "z"};
    const char* result4 = getValueAtIndex(test4, 3, 5);
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    
    // Test case 5: Negative index
    char* test5[] = {"hello", "world"};
    const char* result5 = getValueAtIndex(test5, 2, -1);
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    
    return 0;
}
