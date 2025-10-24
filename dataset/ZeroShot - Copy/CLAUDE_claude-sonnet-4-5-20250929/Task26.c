
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int key;
    int value;
} KeyValue;

int findOdd(int* array, int size) {
    KeyValue* countMap = (KeyValue*)calloc(size, sizeof(KeyValue));
    int mapSize = 0;
    
    for (int i = 0; i < size; i++) {
        int found = 0;
        for (int j = 0; j < mapSize; j++) {
            if (countMap[j].key == array[i]) {
                countMap[j].value++;
                found = 1;
                break;
            }
        }
        if (!found) {
            countMap[mapSize].key = array[i];
            countMap[mapSize].value = 1;
            mapSize++;
        }
    }
    
    int result = 0;
    for (int i = 0; i < mapSize; i++) {
        if (countMap[i].value % 2 != 0) {
            result = countMap[i].key;
            break;
        }
    }
    
    free(countMap);
    return result;
}

int main() {
    // Test case 1
    int test1[] = {7};
    printf("Test 1: %d\\n", findOdd(test1, 1)); // Expected: 7
    
    // Test case 2
    int test2[] = {0};
    printf("Test 2: %d\\n", findOdd(test2, 1)); // Expected: 0
    
    // Test case 3
    int test3[] = {1, 1, 2};
    printf("Test 3: %d\\n", findOdd(test3, 3)); // Expected: 2
    
    // Test case 4
    int test4[] = {0, 1, 0, 1, 0};
    printf("Test 4: %d\\n", findOdd(test4, 5)); // Expected: 0
    
    // Test case 5
    int test5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    printf("Test 5: %d\\n", findOdd(test5, 13)); // Expected: 4
    
    return 0;
}
