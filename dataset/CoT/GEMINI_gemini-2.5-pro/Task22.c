#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum to tag the type stored in the union
typedef enum {
    TYPE_INT,
    TYPE_STRING
} ElementType;

// Union to hold either an integer or a string
typedef union {
    int i;
    const char* s;
} ElementValue;

// Struct to represent a mixed-type element
typedef struct {
    ElementType type;
    ElementValue value;
} MixedElement;

/**
 * @brief Filters an array of MixedElement, returning a new array with only the integers.
 * 
 * @param list The input array of MixedElement.
 * @param count The number of elements in the input array.
 * @param result_count A pointer to an integer where the size of the result array will be stored.
 * @return A dynamically allocated array of integers. The caller is responsible for freeing this memory.
 *         Returns NULL on allocation failure.
 */
int* filterList(const MixedElement* list, size_t count, size_t* result_count) {
    *result_count = 0;
    // First pass: count the number of integers to determine required memory size.
    // This prevents buffer overflows by allocating the exact amount of memory needed.
    for (size_t i = 0; i < count; ++i) {
        if (list[i].type == TYPE_INT) {
            (*result_count)++;
        }
    }

    if (*result_count == 0) {
        return NULL; // Return NULL for an empty result to avoid allocating 0 bytes
    }

    // Allocate memory for the result array.
    // Checking the return value of malloc is crucial to prevent crashes if memory is exhausted.
    int* result = (int*)malloc(*result_count * sizeof(int));
    if (result == NULL) {
        // Handle memory allocation failure gracefully.
        fprintf(stderr, "Error: Memory allocation failed.\n");
        *result_count = 0;
        return NULL;
    }

    // Second pass: populate the new array with the integers.
    size_t current_index = 0;
    for (size_t i = 0; i < count; ++i) {
        if (list[i].type == TYPE_INT) {
            result[current_index] = list[i].value.i;
            current_index++;
        }
    }

    return result;
}

// Helper function to print the result array
void print_result(const char* test_name, const int* result, size_t count) {
    printf("%s: [", test_name);
    if (result != NULL) {
        for (size_t i = 0; i < count; ++i) {
            printf("%d%s", result[i], (i == count - 1) ? "" : ", ");
        }
    }
    printf("]\n");
}

int main() {
    size_t result_size;
    int* filtered_list;

    // Test Case 1
    MixedElement list1[] = {
        {TYPE_INT, {.i = 1}},
        {TYPE_INT, {.i = 2}},
        {TYPE_STRING, {.s = "a"}},
        {TYPE_STRING, {.s = "b"}}
    };
    filtered_list = filterList(list1, 4, &result_size);
    print_result("Test Case 1", filtered_list, result_size);
    free(filtered_list); // IMPORTANT: Caller must free the allocated memory.

    // Test Case 2
    MixedElement list2[] = {
        {TYPE_INT, {.i = 1}},
        {TYPE_STRING, {.s = "a"}},
        {TYPE_STRING, {.s = "b"}},
        {TYPE_INT, {.i = 0}},
        {TYPE_INT, {.i = 15}}
    };
    filtered_list = filterList(list2, 5, &result_size);
    print_result("Test Case 2", filtered_list, result_size);
    free(filtered_list);

    // Test Case 3
    MixedElement list3[] = {
        {TYPE_INT, {.i = 1}},
        {TYPE_INT, {.i = 2}},
        {TYPE_STRING, {.s = "aasf"}},
        {TYPE_STRING, {.s = "1"}},
        {TYPE_STRING, {.s = "123"}},
        {TYPE_INT, {.i = 123}}
    };
    filtered_list = filterList(list3, 6, &result_size);
    print_result("Test Case 3", filtered_list, result_size);
    free(filtered_list);

    // Test Case 4: All strings
    MixedElement list4[] = {
        {TYPE_STRING, {.s = "hello"}},
        {TYPE_STRING, {.s = "world"}}
    };
    filtered_list = filterList(list4, 2, &result_size);
    print_result("Test Case 4", filtered_list, result_size);
    free(filtered_list); // free(NULL) is safe.

    // Test Case 5: Empty list
    filtered_list = filterList(NULL, 0, &result_size);
    print_result("Test Case 5", filtered_list, result_size);
    free(filtered_list); // free(NULL) is safe.

    return 0;
}