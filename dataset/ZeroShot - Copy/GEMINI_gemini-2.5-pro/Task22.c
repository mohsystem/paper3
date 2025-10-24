#include <stdio.h>
#include <stdlib.h>

// Enum to identify the type of data in the union
typedef enum {
    TYPE_INT,
    TYPE_STRING
} DataType;

// A struct to hold a mixed-type value
typedef struct {
    DataType type;
    union {
        int i;
        const char* s;
    } value;
} MixedType;

/**
 * Filters an array of MixedType, returning a new dynamically allocated
 * array containing only the non-negative integers.
 *
 * @param list Pointer to the input array of MixedType.
 * @param size The number of elements in the input array.
 * @param new_size Pointer to an integer where the size of the new array will be stored.
 * @return A pointer to the new dynamically allocated array of integers.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if no integers are found or if input is invalid.
 */
int* filter_list(const MixedType* list, size_t size, size_t* new_size) {
    if (list == NULL || new_size == NULL) {
        if (new_size) *new_size = 0;
        return NULL;
    }

    // First pass: count the number of non-negative integers
    size_t count = 0;
    for (size_t i = 0; i < size; ++i) {
        if (list[i].type == TYPE_INT && list[i].value.i >= 0) {
            count++;
        }
    }

    *new_size = count;
    if (count == 0) {
        return NULL;
    }

    // Allocate memory for the new array
    int* filtered_array = (int*)malloc(count * sizeof(int));
    if (filtered_array == NULL) {
        // Memory allocation failed
        *new_size = 0;
        return NULL;
    }

    // Second pass: populate the new array
    size_t current_index = 0;
    for (size_t i = 0; i < size; ++i) {
        if (list[i].type == TYPE_INT && list[i].value.i >= 0) {
            filtered_array[current_index++] = list[i].value.i;
        }
    }

    return filtered_array;
}

void print_array(const char* prefix, const int* arr, size_t size) {
    printf("%s", prefix);
    printf("[");
    for (size_t i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    MixedType list1[] = {
        {.type = TYPE_INT, .value.i = 1},
        {.type = TYPE_INT, .value.i = 2},
        {.type = TYPE_STRING, .value.s = "a"},
        {.type = TYPE_STRING, .value.s = "b"}
    };
    size_t new_size1;
    printf("Test 1 Input: [1, 2, \"a\", \"b\"]\n");
    int* filtered1 = filter_list(list1, 4, &new_size1);
    print_array("Test 1 Output: ", filtered1, new_size1);
    free(filtered1);

    // Test Case 2
    MixedType list2[] = {
        {.type = TYPE_INT, .value.i = 1},
        {.type = TYPE_STRING, .value.s = "a"},
        {.type = TYPE_STRING, .value.s = "b"},
        {.type = TYPE_INT, .value.i = 0},
        {.type = TYPE_INT, .value.i = 15}
    };
    size_t new_size2;
    printf("Test 2 Input: [1, \"a\", \"b\", 0, 15]\n");
    int* filtered2 = filter_list(list2, 5, &new_size2);
    print_array("Test 2 Output: ", filtered2, new_size2);
    free(filtered2);
    
    // Test Case 3
    MixedType list3[] = {
        {.type = TYPE_INT, .value.i = 1},
        {.type = TYPE_INT, .value.i = 2},
        {.type = TYPE_STRING, .value.s = "aasf"},
        {.type = TYPE_STRING, .value.s = "1"},
        {.type = TYPE_STRING, .value.s = "123"},
        {.type = TYPE_INT, .value.i = 123}
    };
    size_t new_size3;
    printf("Test 3 Input: [1, 2, \"aasf\", \"1\", \"123\", 123]\n");
    int* filtered3 = filter_list(list3, 6, &new_size3);
    print_array("Test 3 Output: ", filtered3, new_size3);
    free(filtered3);

    // Test Case 4: With negative numbers
    MixedType list4[] = {
        {.type = TYPE_INT, .value.i = -1},
        {.type = TYPE_STRING, .value.s = "c"},
        {.type = TYPE_INT, .value.i = 5},
        {.type = TYPE_INT, .value.i = -9},
        {.type = TYPE_STRING, .value.s = "d"}
    };
    size_t new_size4;
    printf("Test 4 Input: [-1, \"c\", 5, -9, \"d\"]\n");
    int* filtered4 = filter_list(list4, 5, &new_size4);
    print_array("Test 4 Output: ", filtered4, new_size4);
    free(filtered4);

    // Test Case 5: Empty list
    MixedType list5[] = {};
    size_t new_size5;
    printf("Test 5 Input: []\n");
    int* filtered5 = filter_list(list5, 0, &new_size5);
    print_array("Test 5 Output: ", filtered5, new_size5);
    free(filtered5); // free(NULL) is safe

    return 0;
}