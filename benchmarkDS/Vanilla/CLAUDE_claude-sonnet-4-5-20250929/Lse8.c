
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getValueAtIndex(char* args[], int argc, int index) {
    if (args == NULL || index < 0 || index >= argc) {
        return NULL;
    }
    return args[index];
}

int main() {
    // Test case 1
    char* test1[] = {"program", "1", "hello", "world"};
    int argc1 = 4;
    int index1 = atoi(test1[1]);
    char* result1 = getValueAtIndex(test1, argc1, index1);
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    
    // Test case 2
    char* test2[] = {"program", "2", "apple", "banana", "cherry"};
    int argc2 = 5;
    int index2 = atoi(test2[1]);
    char* result2 = getValueAtIndex(test2, argc2, index2);
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    
    // Test case 3
    char* test3[] = {"program", "0", "first"};
    int argc3 = 3;
    int index3 = atoi(test3[1]);
    char* result3 = getValueAtIndex(test3, argc3, index3);
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    
    // Test case 4
    char* test4[] = {"program", "3", "a", "b", "c", "d"};
    int argc4 = 6;
    int index4 = atoi(test4[1]);
    char* result4 = getValueAtIndex(test4, argc4, index4);
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    
    // Test case 5
    char* test5[] = {"program", "1", "test"};
    int argc5 = 3;
    int index5 = atoi(test5[1]);
    char* result5 = getValueAtIndex(test5, argc5, index5);
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    
    return 0;
}
