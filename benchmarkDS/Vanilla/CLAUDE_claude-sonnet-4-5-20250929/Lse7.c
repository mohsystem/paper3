
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printArgumentAtIndex(char* args[], int argc, const char* indexStr) {
    int index = atoi(indexStr);
    if (index >= 0 && index < argc) {
        printf("%s\\n", args[index]);
    } else {
        printf("Index out of bounds\\n");
    }
}

int main(int argc, char* argv[]) {
    // Test case 1
    char* test1[] = {"program", "2", "hello", "world", "c"};
    printf("Test 1: ");
    printArgumentAtIndex(test1, 5, test1[1]);
    
    // Test case 2
    char* test2[] = {"program", "0", "first", "second", "third"};
    printf("Test 2: ");
    printArgumentAtIndex(test2, 5, test2[1]);
    
    // Test case 3
    char* test3[] = {"program", "3", "alpha", "beta", "gamma", "delta"};
    printf("Test 3: ");
    printArgumentAtIndex(test3, 6, test3[1]);
    
    // Test case 4
    char* test4[] = {"program", "1", "testing"};
    printf("Test 4: ");
    printArgumentAtIndex(test4, 3, test4[1]);
    
    // Test case 5
    char* test5[] = {"program", "5", "one", "two", "three"};
    printf("Test 5: ");
    printArgumentAtIndex(test5, 5, test5[1]);
    
    return 0;
}
