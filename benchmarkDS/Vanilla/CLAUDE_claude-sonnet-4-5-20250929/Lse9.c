
#include <stdio.h>
#include <string.h>

void printArgumentAtIndex(char* args[], int argc, int index) {
    if (index >= 0 && index < argc) {
        printf("%s\\n", args[index]);
    } else {
        printf("Index out of bounds\\n");
    }
}

int main() {
    // Test case 1: Index 0
    char* test1[] = {"first", "second", "third"};
    printf("Test 1 (index 0): ");
    printArgumentAtIndex(test1, 3, 0);
    
    // Test case 2: Index 1
    char* test2[] = {"apple", "banana", "cherry"};
    printf("Test 2 (index 1): ");
    printArgumentAtIndex(test2, 3, 1);
    
    // Test case 3: Index 2
    char* test3[] = {"hello", "world", "c", "programming"};
    printf("Test 3 (index 2): ");
    printArgumentAtIndex(test3, 4, 2);
    
    // Test case 4: Index out of bounds
    char* test4[] = {"one", "two"};
    printf("Test 4 (index 5): ");
    printArgumentAtIndex(test4, 2, 5);
    
    // Test case 5: Index 3
    char* test5[] = {"a", "b", "c", "d", "e"};
    printf("Test 5 (index 3): ");
    printArgumentAtIndex(test5, 5, 3);
    
    return 0;
}
