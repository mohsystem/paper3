
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C does not have native support for mixed-type lists like Python or Java.
// This implementation uses a struct to represent items that can be either int or string.

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
    int size;
} IntList;

IntList filterList(Item* items, int count) {
    IntList result;
    result.values = (int*)malloc(count * sizeof(int));
    result.size = 0;
    
    for (int i = 0; i < count; i++) {
        if (items[i].type == TYPE_INT) {
            result.values[result.size++] = items[i].data.int_val;
        }
    }
    
    return result;
}

void printIntList(IntList list) {
    printf("[");
    for (int i = 0; i < list.size; i++) {
        printf("%d", list.values[i]);
        if (i < list.size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    Item test1[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_INT, {.int_val = 2}}, 
                    {TYPE_STRING, {.str_val = "a"}}, {TYPE_STRING, {.str_val = "b"}}};
    IntList result1 = filterList(test1, 4);
    printIntList(result1);
    free(result1.values);
    
    // Test case 2
    Item test2[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_STRING, {.str_val = "a"}}, 
                    {TYPE_STRING, {.str_val = "b"}}, {TYPE_INT, {.int_val = 0}}, 
                    {TYPE_INT, {.int_val = 15}}};
    IntList result2 = filterList(test2, 5);
    printIntList(result2);
    free(result2.values);
    
    // Test case 3
    Item test3[] = {{TYPE_INT, {.int_val = 1}}, {TYPE_INT, {.int_val = 2}}, 
                    {TYPE_STRING, {.str_val = "a"}}, {TYPE_STRING, {.str_val = "b"}}, 
                    {TYPE_STRING, {.str_val = "aasf"}}, {TYPE_STRING, {.str_val = "1"}}, 
                    {TYPE_STRING, {.str_val = "123"}}, {TYPE_INT, {.int_val = 123}}};
    IntList result3 = filterList(test3, 8);
    printIntList(result3);
    free(result3.values);
    
    // Test case 4
    Item test4[] = {{TYPE_INT, {.int_val = 10}}, {TYPE_INT, {.int_val = 20}}, 
                    {TYPE_INT, {.int_val = 30}}, {TYPE_STRING, {.str_val = "test"}}, 
                    {TYPE_STRING, {.str_val = "hello"}}, {TYPE_INT, {.int_val = 40}}};
    IntList result4 = filterList(test4, 6);
    printIntList(result4);
    free(result4.values);
    
    // Test case 5
    Item test5[] = {{TYPE_STRING, {.str_val = "only"}}, {TYPE_STRING, {.str_val = "strings"}}, 
                    {TYPE_STRING, {.str_val = "here"}}};
    IntList result5 = filterList(test5, 3);
    printIntList(result5);
    free(result5.values);
    
    return 0;
}
