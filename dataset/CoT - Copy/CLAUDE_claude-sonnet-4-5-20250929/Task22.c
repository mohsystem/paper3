
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { TYPE_INT, TYPE_STRING } ItemType;

typedef struct {
    ItemType type;
    union {
        int int_val;
        char* str_val;
    } data;
} Item;

typedef struct {
    int* values;
    size_t size;
    size_t capacity;
} IntList;

IntList* filterList(Item* items, size_t count) {
    if (items == NULL) {
        IntList* result = (IntList*)malloc(sizeof(IntList));
        result->values = NULL;
        result->size = 0;
        result->capacity = 0;
        return result;
    }
    
    IntList* result = (IntList*)malloc(sizeof(IntList));
    result->capacity = count;
    result->values = (int*)malloc(sizeof(int) * result->capacity);
    result->size = 0;
    
    for (size_t i = 0; i < count; i++) {
        if (items[i].type == TYPE_INT) {
            result->values[result->size++] = items[i].data.int_val;
        }
    }
    
    return result;
}

void printIntList(IntList* list) {
    printf("[");
    for (size_t i = 0; i < list->size; i++) {
        printf("%d", list->values[i]);
        if (i < list->size - 1) printf(", ");
    }
    printf("]\\n");
}

void freeIntList(IntList* list) {
    if (list) {
        free(list->values);
        free(list);
    }
}

int main() {
    // Test case 1
    Item test1[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_INT, {.int_val = 2}}, 
                    {TYPE_STRING, {.str_val = "a"}}, {TYPE_STRING, {.str_val = "b"}}};
    IntList* result1 = filterList(test1, 4);
    printIntList(result1);
    freeIntList(result1);
    
    // Test case 2
    Item test2[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_STRING, {.str_val = "a"}}, 
                    {TYPE_STRING, {.str_val = "b"}}, {TYPE_INT, {.int_val = 0}}, 
                    {TYPE_INT, {.int_val = 15}}};
    IntList* result2 = filterList(test2, 5);
    printIntList(result2);
    freeIntList(result2);
    
    // Test case 3
    Item test3[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_INT, {.int_val = 2}}, 
                    {TYPE_STRING, {.str_val = "a"}}, {TYPE_STRING, {.str_val = "b"}}, 
                    {TYPE_STRING, {.str_val = "aasf"}}, {TYPE_STRING, {.str_val = "1"}}, 
                    {TYPE_STRING, {.str_val = "123"}}, {TYPE_INT, {.int_val = 123}}};
    IntList* result3 = filterList(test3, 8);
    printIntList(result3);
    freeIntList(result3);
    
    // Test case 4
    IntList* result4 = filterList(NULL, 0);
    printIntList(result4);
    freeIntList(result4);
    
    // Test case 5
    Item test5[] = {{TYPE_INT, {.int_val = 100}}, {TYPE_INT, {.int_val = 200}}, 
                    {TYPE_STRING, {.str_val = "test"}}, {TYPE_INT, {.int_val = 300}}};
    IntList* result5 = filterList(test5, 4);
    printIntList(result5);
    freeIntList(result5);
    
    return 0;
}
