
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Define a tagged union for type-safe variant storage
// Security: Explicit type tagging prevents type confusion vulnerabilities
typedef enum {
    TYPE_INT,
    TYPE_STRING
} ItemType;

typedef struct {
    ItemType type;
    union {
        int int_value;
        char* str_value;  // Pointer must be properly managed
    } data;
} Item;

typedef struct {
    int* values;      // Pointer to integer array, must be NULL-checked
    size_t size;      // Current number of elements
    size_t capacity;  // Allocated capacity
} IntList;

// Security: Initialize all struct members to safe defaults
IntList* createIntList(size_t initial_capacity) {
    // Validate capacity to prevent integer overflow in malloc
    const size_t MAX_CAPACITY = SIZE_MAX / sizeof(int);
    if (initial_capacity > MAX_CAPACITY) {
        return NULL;  // Fail safely
    }
    
    IntList* list = (IntList*)malloc(sizeof(IntList));
    if (list == NULL) {
        return NULL;  // Check malloc return value
    }
    
    // Initialize all members
    list->values = NULL;
    list->size = 0;
    list->capacity = 0;
    
    if (initial_capacity > 0) {
        list->values = (int*)calloc(initial_capacity, sizeof(int));
        if (list->values == NULL) {
            free(list);  // Clean up on allocation failure
            return NULL;
        }
        list->capacity = initial_capacity;
    }
    
    return list;
}

// Security: Bounds checking before array access, overflow prevention
int appendInt(IntList* list, int value) {
    if (list == NULL) {
        return -1;  // Validate pointer
    }
    
    // Check for non-negative constraint
    if (value < 0) {
        return -1;  // Reject negative values per specification
    }
    
    // Need to grow the array
    if (list->size >= list->capacity) {
        // Prevent integer overflow in capacity calculation
        const size_t MAX_CAPACITY = SIZE_MAX / sizeof(int) / 2;
        size_t new_capacity = (list->capacity == 0) ? 8 : list->capacity * 2;
        
        if (new_capacity > MAX_CAPACITY) {
            return -1;  // Prevent excessive allocation
        }
        
        // Use realloc safely with temporary pointer
        int* new_values = (int*)realloc(list->values, new_capacity * sizeof(int));
        if (new_values == NULL) {
            return -1;  // Check realloc return value
        }
        
        list->values = new_values;
        list->capacity = new_capacity;
    }
    
    // Bounds check before write
    if (list->size < list->capacity) {
        list->values[list->size] = value;
        list->size++;
        return 0;
    }
    
    return -1;
}

// Security: Proper memory cleanup to prevent leaks
void freeIntList(IntList* list) {
    if (list != NULL) {
        if (list->values != NULL) {
            free(list->values);
            list->values = NULL;  // Prevent double-free
        }
        free(list);
    }
}

// Filter function that extracts integers from mixed item list
// Security: Input validation, bounds checking, safe memory management
IntList* filterList(const Item* items, size_t item_count) {
    // Validate input parameters
    if (items == NULL && item_count > 0) {
        return NULL;  // Null pointer with non-zero count is invalid
    }
    
    // Prevent excessive memory allocation (DoS protection)
    const size_t MAX_ITEMS = 1000000;
    if (item_count > MAX_ITEMS) {
        return NULL;
    }
    
    // Create result list with appropriate initial capacity
    IntList* result = createIntList(item_count > 0 ? item_count : 1);
    if (result == NULL) {
        return NULL;  // Check allocation
    }
    
    // Process each item with bounds checking
    for (size_t i = 0; i < item_count; i++) {
        // Bounds check for array access
        if (i >= item_count) {
            freeIntList(result);
            return NULL;
        }
        
        if (items[i].type == TYPE_INT) {
            if (appendInt(result, items[i].data.int_value) != 0) {
                freeIntList(result);
                return NULL;  // Handle append failure
            }
        }
        // Strings are filtered out silently
    }
    
    return result;
}

int main(void) {
    // Test case 1: Mixed integers and strings
    Item test1[] = {
        {TYPE_INT, {.int_value = 1}},
        {TYPE_INT, {.int_value = 2}},
        {TYPE_STRING, {.str_value = "a"}},
        {TYPE_STRING, {.str_value = "b"}}
    };
    IntList* result1 = filterList(test1, 4);
    if (result1 != NULL) {
        printf("Test 1: ");
        for (size_t i = 0; i < result1->size; i++) {
            printf("%d ", result1->values[i]);
        }
        printf("\\n");
        freeIntList(result1);
    }
    
    // Test case 2: Integers with zero
    Item test2[] = {
        {TYPE_INT, {.int_value = 1}},
        {TYPE_STRING, {.str_value = "a"}},
        {TYPE_STRING, {.str_value = "b"}},
        {TYPE_INT, {.int_value = 0}},
        {TYPE_INT, {.int_value = 15}}
    };
    IntList* result2 = filterList(test2, 5);
    if (result2 != NULL) {
        printf("Test 2: ");
        for (size_t i = 0; i < result2->size; i++) {
            printf("%d ", result2->values[i]);
        }
        printf("\\n");
        freeIntList(result2);
    }
    
    // Test case 3: Multiple strings and integers
    Item test3[] = {
        {TYPE_INT, {.int_value = 1}},
        {TYPE_INT, {.int_value = 2}},
        {TYPE_STRING, {.str_value = "a"}},
        {TYPE_STRING, {.str_value = "b"}},
        {TYPE_STRING, {.str_value = "aasf"}},
        {TYPE_STRING, {.str_value = "1"}},
        {TYPE_STRING, {.str_value = "123"}},
        {TYPE_INT, {.int_value = 123}}
    };
    IntList* result3 = filterList(test3, 8);
    if (result3 != NULL) {
        printf("Test 3: ");
        for (size_t i = 0; i < result3->size; i++) {
            printf("%d ", result3->values[i]);
        }
        printf("\\n");
        freeIntList(result3);
    }
    
    // Test case 4: Only integers
    Item test4[] = {
        {TYPE_INT, {.int_value = 5}},
        {TYPE_INT, {.int_value = 10}},
        {TYPE_INT, {.int_value = 15}},
        {TYPE_INT, {.int_value = 20}}
    };
    IntList* result4 = filterList(test4, 4);
    if (result4 != NULL) {
        printf("Test 4: ");
        for (size_t i = 0; i < result4->size; i++) {
            printf("%d ", result4->values[i]);
        }
        printf("\\n");
        freeIntList(result4);
    }
    
    // Test case 5: Empty list
    IntList* result5 = filterList(NULL, 0);
    if (result5 != NULL) {
        printf("Test 5: ");
        for (size_t i = 0; i < result5->size; i++) {
            printf("%d ", result5->values[i]);
        }
        printf("\\n");
        freeIntList(result5);
    }
    
    return 0;
}
