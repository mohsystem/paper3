
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int key;
    int value;
    int used;
} HashEntry;

typedef struct {
    HashEntry* entries;
    int capacity;
} HashMap;

HashMap* createHashMap(int capacity) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) return NULL;
    
    map->capacity = capacity;
    map->entries = (HashEntry*)calloc(capacity, sizeof(HashEntry));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    
    return map;
}

void freeHashMap(HashMap* map) {
    if (map) {
        if (map->entries) free(map->entries);
        free(map);
    }
}

int hashFunction(int key, int capacity) {
    int hash = abs(key) % capacity;
    return hash;
}

void put(HashMap* map, int key, int value) {
    int index = hashFunction(key, map->capacity);
    int original = index;
    
    while (map->entries[index].used && map->entries[index].key != key) {
        index = (index + 1) % map->capacity;
        if (index == original) return;
    }
    
    map->entries[index].key = key;
    map->entries[index].value = value;
    map->entries[index].used = 1;
}

int get(HashMap* map, int key, int* found) {
    int index = hashFunction(key, map->capacity);
    int original = index;
    
    while (map->entries[index].used) {
        if (map->entries[index].key == key) {
            *found = 1;
            return map->entries[index].value;
        }
        index = (index + 1) % map->capacity;
        if (index == original) break;
    }
    
    *found = 0;
    return -1;
}

int* twoSum(int* numbers, int numbersSize, int target, int* returnSize) {
    *returnSize = 0;
    
    if (!numbers || numbersSize < 2) {
        return NULL;
    }
    
    HashMap* map = createHashMap(numbersSize * 2);
    if (!map) return NULL;
    
    int* result = (int*)malloc(2 * sizeof(int));
    if (!result) {
        freeHashMap(map);
        return NULL;
    }
    
    for (int i = 0; i < numbersSize; i++) {
        int complement = target - numbers[i];
        int found = 0;
        int index = get(map, complement, &found);
        
        if (found) {
            result[0] = index;
            result[1] = i;
            *returnSize = 2;
            freeHashMap(map);
            return result;
        }
        
        put(map, numbers[i], i);
    }
    
    freeHashMap(map);
    free(result);
    return NULL;
}

int main() {
    int returnSize;
    
    // Test case 1
    int arr1[] = {1, 2, 3};
    int* result1 = twoSum(arr1, 3, 4, &returnSize);
    if (result1) {
        printf("Test 1: [%d, %d]\\n", result1[0], result1[1]);
        free(result1);
    }
    
    // Test case 2
    int arr2[] = {3, 2, 4};
    int* result2 = twoSum(arr2, 3, 6, &returnSize);
    if (result2) {
        printf("Test 2: [%d, %d]\\n", result2[0], result2[1]);
        free(result2);
    }
    
    // Test case 3
    int arr3[] = {1, 5, 3, 7, 9};
    int* result3 = twoSum(arr3, 5, 12, &returnSize);
    if (result3) {
        printf("Test 3: [%d, %d]\\n", result3[0], result3[1]);
        free(result3);
    }
    
    // Test case 4
    int arr4[] = {10, 20, 30, 40};
    int* result4 = twoSum(arr4, 4, 50, &returnSize);
    if (result4) {
        printf("Test 4: [%d, %d]\\n", result4[0], result4[1]);
        free(result4);
    }
    
    // Test case 5
    int arr5[] = {-1, -2, -3, -4};
    int* result5 = twoSum(arr5, 4, -6, &returnSize);
    if (result5) {
        printf("Test 5: [%d, %d]\\n", result5[0], result5[1]);
        free(result5);
    }
    
    return 0;
}
