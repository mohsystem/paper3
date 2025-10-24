
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void whoLikesIt(char** names, int size, char* result) {
    if (size == 0) {
        strcpy(result, "no one likes this");
    } else if (size == 1) {
        sprintf(result, "%s likes this", names[0]);
    } else if (size == 2) {
        sprintf(result, "%s and %s like this", names[0], names[1]);
    } else if (size == 3) {
        sprintf(result, "%s, %s and %s like this", names[0], names[1], names[2]);
    } else {
        int others = size - 2;
        sprintf(result, "%s, %s and %d others like this", names[0], names[1], others);
    }
}

int main() {
    char result[256];
    
    // Test case 1
    char* test1[] = {};
    whoLikesIt(test1, 0, result);
    printf("%s\\n", result);
    
    // Test case 2
    char* test2[] = {"Peter"};
    whoLikesIt(test2, 1, result);
    printf("%s\\n", result);
    
    // Test case 3
    char* test3[] = {"Jacob", "Alex"};
    whoLikesIt(test3, 2, result);
    printf("%s\\n", result);
    
    // Test case 4
    char* test4[] = {"Max", "John", "Mark"};
    whoLikesIt(test4, 3, result);
    printf("%s\\n", result);
    
    // Test case 5
    char* test5[] = {"Alex", "Jacob", "Mark", "Max"};
    whoLikesIt(test5, 4, result);
    printf("%s\\n", result);
    
    return 0;
}
