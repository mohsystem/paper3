#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum to identify the type of data in the union
typedef enum {
    TYPE_INT,
    TYPE_STRING
} ElementType;

// A variant-like struct to hold either an integer or a string
typedef struct {
    ElementType type;
    union {
        int i_val;
        const char* s_val;
    } value;
} Variant;

// Struct to hold the result array of integers and its size
typedef struct {
    int* data;
    size_t size;
} IntArray;

/**
 * @brief Filters an array of Variant elements, returning only the integers.
 *
 * This function takes an array of Variant structs and its size. It dynamically
 * allocates a new array to store only the integer elements.
 *
 * @param list A constant pointer to the input array of Variant structs.
 * @param count The number of elements in the input array.
 * @return An IntArray struct containing a pointer to the newly allocated
 *         array of integers and its size. The caller is responsible for freeing
 *         the 'data' member of the returned struct. If an error occurs during
 *         memory allocation, the 'data' member will be NULL and size will be 0.
 */
IntArray filter_list(const Variant* list, size_t count) {
    IntArray result = {NULL, 0};
    size_t capacity = 0;

    for (size_t i = 0; i < count; ++i) {
        if (list[i].type == TYPE_INT) {
            if (result.size >= capacity) {
                // Grow the buffer dynamically. Start with a small capacity.
                size_t new_capacity = (capacity == 0) ? 8 : capacity * 2;
                // Prevent integer overflow for capacity calculation
                if (new_capacity < capacity) { 
                    fprintf(stderr, "Error: Capacity calculation overflow.\n");
                    free(result.data);
                    result.data = NULL;
                    result.size = 0;
                    return result;
                }
                int* new_data = (int*)realloc(result.data, new_capacity * sizeof(int));
                if (new_data == NULL) {
                    // realloc failed, clean up and return error state
                    fprintf(stderr, "Error: Memory reallocation failed.\n");
                    free(result.data);
                    result.data = NULL;
                    result.size = 0;
                    return result;
                }
                result.data = new_data;
                capacity = new_capacity;
            }
            result.data[result.size] = list[i].value.i_val;
            result.size++;
        }
    }
    
    // Optional: shrink the allocated memory to fit the actual size
    if (result.size > 0 && result.size < capacity) {
        int* final_data = (int*)realloc(result.data, result.size * sizeof(int));
        if (final_data != NULL) {
            result.data = final_data;
        }
        // If realloc fails here, we can still use the old buffer, so it's not a critical error.
    }


    return result;
}

void print_array(const char* prefix, const IntArray* arr) {
    printf("%s", prefix);
    for (size_t i = 0; i < arr->size; ++i) {
        printf("%d%s", arr->data[i], (i == arr->size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    Variant list1[] = {
        {.type = TYPE_INT, .value = {.i_val = 1}},
        {.type = TYPE_INT, .value = {.i_val = 2}},
        {.type = TYPE_STRING, .value = {.s_val = "a"}},
        {.type = TYPE_STRING, .value = {.s_val = "b"}}
    };
    size_t count1 = sizeof(list1) / sizeof(list1[0]);
    IntArray result1 = filter_list(list1, count1);
    printf("Test Case 1: [1, 2, \"a\", \"b\"] -> ");
    print_array("[", &result1);
    free(result1.data);

    // Test Case 2
    Variant list2[] = {
        {.type = TYPE_INT, .value = {.i_val = 1}},
        {.type = TYPE_STRING, .value = {.s_val = "a"}},
        {.type = TYPE_STRING, .value = {.s_val = "b"}},
        {.type = TYPE_INT, .value = {.i_val = 0}},
        {.type = TYPE_INT, .value = {.i_val = 15}}
    };
    size_t count2 = sizeof(list2) / sizeof(list2[0]);
    IntArray result2 = filter_list(list2, count2);
    printf("Test Case 2: [1, \"a\", \"b\", 0, 15] -> ");
    print_array("[", &result2);
    free(result2.data);

    // Test Case 3
    Variant list3[] = {
        {.type = TYPE_INT, .value = {.i_val = 1}},
        {.type = TYPE_INT, .value = {.i_val = 2}},
        {.type = TYPE_STRING, .value = {.s_val = "aasf"}},
        {.type = TYPE_STRING, .value = {.s_val = "1"}},
        {.type = TYPE_STRING, .value = {.s_val = "123"}},
        {.type = TYPE_INT, .value = {.i_val = 123}}
    };
    size_t count3 = sizeof(list3) / sizeof(list3[0]);
    IntArray result3 = filter_list(list3, count3);
    printf("Test Case 3: [1, 2, \"aasf\", \"1\", \"123\", 123] -> ");
    print_array("[", &result3);
    free(result3.data);

    // Test Case 4: Empty list
    Variant list4[] = {};
    size_t count4 = 0;
    IntArray result4 = filter_list(list4, count4);
    printf("Test Case 4: [] -> ");
    print_array("[", &result4);
    free(result4.data);

    // Test Case 5: List with only strings
    Variant list5[] = {
        {.type = TYPE_STRING, .value = {.s_val = "hello"}},
        {.type = TYPE_STRING, .value = {.s_val = "world"}},
        {.type = TYPE_STRING, .value = {.s_val = "!"}}
    };
    size_t count5 = sizeof(list5) / sizeof(list5[0]);
    IntArray result5 = filter_list(list5, count5);
    printf("Test Case 5: [\"hello\", \"world\", \"!\"] -> ");
    print_array("[", &result5);
    free(result5.data);

    return 0;
}