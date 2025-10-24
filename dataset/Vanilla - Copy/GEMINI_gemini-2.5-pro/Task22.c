#include <stdio.h>
#include <stdlib.h>

// Enum to define the type of data in our variant struct
typedef enum {
    INTEGER,
    STRING
} DataType;

// Union to hold either an integer or a string pointer
typedef union {
    int i;
    const char* s;
} Value;

// Struct to represent a mixed-type element
typedef struct {
    DataType type;
    Value data;
} MixedType;

/**
 * @brief Filters an array of MixedType, keeping only integers.
 * @param list The input array of MixedType elements.
 * @param count The number of elements in the input array.
 * @param result_count A pointer to an integer that will be updated with the size of the returned array.
 * @return A dynamically allocated array of integers. The caller is responsible for freeing this memory.
 */
int* filter_list(const MixedType* list, int count, int* result_count) {
    // First pass: count the number of integers to allocate memory only once.
    int int_count = 0;
    for (int i = 0; i < count; ++i) {
        if (list[i].type == INTEGER) {
            int_count++;
        }
    }

    *result_count = int_count;
    if (int_count == 0) {
        return NULL; // No integers found, return NULL
    }

    // Allocate memory for the result array
    int* result = (int*)malloc(int_count * sizeof(int));
    if (result == NULL) {
        // Handle memory allocation failure
        *result_count = 0;
        return NULL;
    }

    // Second pass: populate the result array
    int current_index = 0;
    for (int i = 0; i < count; ++i) {
        if (list[i].type == INTEGER) {
            result[current_index++] = list[i].data.i;
        }
    }

    return result;
}

// Helper function to print an array of integers
void print_int_array(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

// Helper function to print an array of MixedType for testing
void print_mixed_array(const MixedType* arr, int size) {
    printf("[");
    for(int i = 0; i < size; i++) {
        if (arr[i].type == INTEGER) {
            printf("%d", arr[i].data.i);
        } else {
            printf("\"%s\"", arr[i].data.s);
        }
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1
    MixedType list1[] = {{INTEGER, .data.i = 1}, {INTEGER, .data.i = 2}, {STRING, .data.s = "a"}, {STRING, .data.s = "b"}};
    int count1 = sizeof(list1) / sizeof(list1[0]);
    int result_count1 = 0;
    printf("Test Case 1 Input: ");
    print_mixed_array(list1, count1);
    printf("\n");
    int* filtered1 = filter_list(list1, count1, &result_count1);
    printf("Filtered Output: ");
    print_int_array(filtered1, result_count1);
    printf("\n\n");
    free(filtered1);

    // Test Case 2
    MixedType list2[] = {{INTEGER, .data.i = 1}, {STRING, .data.s = "a"}, {STRING, .data.s = "b"}, {INTEGER, .data.i = 0}, {INTEGER, .data.i = 15}};
    int count2 = sizeof(list2) / sizeof(list2[0]);
    int result_count2 = 0;
    printf("Test Case 2 Input: ");
    print_mixed_array(list2, count2);
    printf("\n");
    int* filtered2 = filter_list(list2, count2, &result_count2);
    printf("Filtered Output: ");
    print_int_array(filtered2, result_count2);
    printf("\n\n");
    free(filtered2);

    // Test Case 3
    MixedType list3[] = {{INTEGER, .data.i = 1}, {INTEGER, .data.i = 2}, {STRING, .data.s = "aasf"}, {STRING, .data.s = "1"}, {STRING, .data.s = "123"}, {INTEGER, .data.i = 123}};
    int count3 = sizeof(list3) / sizeof(list3[0]);
    int result_count3 = 0;
    printf("Test Case 3 Input: ");
    print_mixed_array(list3, count3);
    printf("\n");
    int* filtered3 = filter_list(list3, count3, &result_count3);
    printf("Filtered Output: ");
    print_int_array(filtered3, result_count3);
    printf("\n\n");
    free(filtered3);

    // Test Case 4
    MixedType list4[] = {{STRING, .data.s = "hello"}, {STRING, .data.s = "world"}, {INTEGER, .data.i = 2024}};
    int count4 = sizeof(list4) / sizeof(list4[0]);
    int result_count4 = 0;
    printf("Test Case 4 Input: ");
    print_mixed_array(list4, count4);
    printf("\n");
    int* filtered4 = filter_list(list4, count4, &result_count4);
    printf("Filtered Output: ");
    print_int_array(filtered4, result_count4);
    printf("\n\n");
    free(filtered4);
    
    // Test Case 5
    MixedType list5[] = {{STRING, .data.s = "a"}, {STRING, .data.s = "b"}, {STRING, .data.s = "c"}};
    int count5 = sizeof(list5) / sizeof(list5[0]);
    int result_count5 = 0;
    printf("Test Case 5 Input: ");
    print_mixed_array(list5, count5);
    printf("\n");
    int* filtered5 = filter_list(list5, count5, &result_count5);
    printf("Filtered Output: ");
    print_int_array(filtered5, result_count5);
    printf("\n\n");
    free(filtered5);

    return 0;
}