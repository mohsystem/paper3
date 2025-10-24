
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_INT,
    TYPE_STRING
} ItemType;

typedef struct {
    ItemType type;
    union {
        int intValue;
        char* stringValue;
    } value;
} Item;

typedef struct {
    int* data;
    int size;
    int capacity;
} IntArray;

IntArray* createIntArray() {
    IntArray* arr = (IntArray*)malloc(sizeof(IntArray));
    arr->capacity = 10;
    arr->size = 0;
    arr->data = (int*)malloc(arr->capacity * sizeof(int));
    return arr;
}

void addInt(IntArray* arr, int value) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = (int*)realloc(arr->data, arr->capacity * sizeof(int));
    }
    arr->data[arr->size++] = value;
}

IntArray* filterList(Item* items, int length) {
    IntArray* result = createIntArray();
    
    for (int i = 0; i < length; i++) {
        if (items[i].type == TYPE_INT) {
            addInt(result, items[i].value.intValue);
        }
    }
    
    return result;
}

void printIntArray(IntArray* arr) {
    printf("[");
    for (int i = 0; i < arr->size; i++) {
        printf("%d", arr->data[i]);
        if (i < arr->size - 1) printf(", ");
    }
    printf("]\\n");
}

void freeIntArray(IntArray* arr) {
    free(arr->data);
    free(arr);
}

int main() {
    // Test case 1
    Item test1[] = {{TYPE_INT, {.intValue = 1}}, {TYPE_INT, {.intValue = 2}}, 
                    {TYPE_STRING, {.stringValue = "a"}}, {TYPE_STRING, {.stringValue = "b"}}};
    IntArray* result1 = filterList(test1, 4);
    printf("Test 1: ");
    printIntArray(result1);
    freeIntArray(result1);
    
    // Test case 2
    Item test2[] = {{TYPE_INT, {.intValue = 1}}, {TYPE_STRING, {.stringValue = "a"}}, 
                    {TYPE_STRING, {.stringValue = "b"}}, {TYPE_INT, {.intValue = 0}}, 
                    {TYPE_INT, {.intValue = 15}}};
    IntArray* result2 = filterList(test2, 5);
    printf("Test 2: ");
    printIntArray(result2);
    freeIntArray(result2);
    
    // Test case 3
    Item test3[] = {{TYPE_INT, {.intValue = 1}}, {TYPE_INT, {.intValue = 2}}, 
                    {TYPE_STRING, {.stringValue = "a"}}, {TYPE_STRING, {.stringValue = "b"}},
                    {TYPE_STRING, {.stringValue = "aasf"}}, {TYPE_STRING, {.stringValue = "1"}},
                    {TYPE_STRING, {.stringValue = "123"}}, {TYPE_INT, {.intValue = 123}}};
    IntArray* result3 = filterList(test3, 8);
    printf("Test 3: ");
    printIntArray(result3);
    freeIntArray(result3);
    
    // Test case 4
    Item test4[] = {{TYPE_INT, {.intValue = 5}}, {TYPE_STRING, {.stringValue = "test"}}, 
                    {TYPE_INT, {.intValue = 10}}, {TYPE_STRING, {.stringValue = "string"}},
                    {TYPE_INT, {.intValue = 20}}};
    IntArray* result4 = filterList(test4, 5);
    printf("Test 4: ");
    printIntArray(result4);
    freeIntArray(result4);
    
    // Test case 5
    Item test5[] = {{TYPE_INT, {.intValue = 100}}, {TYPE_INT, {.intValue = 200}}, 
                    {TYPE_INT, {.intValue = 300}}};
    IntArray* result5 = filterList(test5, 3);
    printf("Test 5: ");
    printIntArray(result5);
    freeIntArray(result5);
    
    return 0;
}
