#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Generates a display text for who likes an item.
 *
 * @param names An array of C-strings containing the names of people.
 * @param num_names The number of names in the array.
 * @return A dynamically allocated string with the formatted display text.
 *         The caller is responsible for freeing this memory. Returns NULL on failure.
 */
char* likes(const char* const* names, size_t num_names) {
    char* result = NULL;
    int len = 0;

    if (num_names > 0 && names == NULL) {
        // Invalid input, but we'll handle it gracefully by treating it as 0 names
        num_names = 0;
    }

    switch (num_names) {
        case 0:
            len = snprintf(NULL, 0, "no one likes this");
            break;
        case 1:
            len = snprintf(NULL, 0, "%s likes this", names[0]);
            break;
        case 2:
            len = snprintf(NULL, 0, "%s and %s like this", names[0], names[1]);
            break;
        case 3:
            len = snprintf(NULL, 0, "%s, %s and %s like this", names[0], names[1], names[2]);
            break;
        default: // 4 or more
            len = snprintf(NULL, 0, "%s, %s and %zu others like this", names[0], names[1], num_names - 2);
            break;
    }
    
    // Check for snprintf error
    if (len < 0) {
        return NULL;
    }

    result = (char*)malloc(len + 1);
    if (result == NULL) {
        return NULL; // Allocation failed
    }

    switch (num_names) {
        case 0:
            snprintf(result, len + 1, "no one likes this");
            break;
        case 1:
            snprintf(result, len + 1, "%s likes this", names[0]);
            break;
        case 2:
            snprintf(result, len + 1, "%s and %s like this", names[0], names[1]);
            break;
        case 3:
            snprintf(result, len + 1, "%s, %s and %s like this", names[0], names[1], names[2]);
            break;
        default: // 4 or more
            snprintf(result, len + 1, "%s, %s and %zu others like this", names[0], names[1], num_names - 2);
            break;
    }

    return result;
}

/**
 * @brief Helper function to run a test case and print the result.
 * 
 * @param names The array of names for the test case.
 * @param num_names The number of names in the array.
 * @param label A string label to describe the test case input.
 */
void run_test_case(const char* const* names, size_t num_names, const char* label) {
    char* result = likes(names, num_names);
    printf("%s -> \"", label);
    if (result) {
        printf("%s", result);
        free(result); // Free the allocated memory
    } else {
        printf("Error: Memory allocation failed");
    }
    printf("\"\n");
}


int main() {
    // Test case 1: 0 names
    run_test_case(NULL, 0, "[]");

    // Test case 2: 1 name
    const char* names2[] = {"Peter"};
    run_test_case(names2, 1, "[\"Peter\"]");

    // Test case 3: 2 names
    const char* names3[] = {"Jacob", "Alex"};
    run_test_case(names3, 2, "[\"Jacob\", \"Alex\"]");

    // Test case 4: 3 names
    const char* names4[] = {"Max", "John", "Mark"};
    run_test_case(names4, 3, "[\"Max\", \"John\", \"Mark\"]");

    // Test case 5: 4 names
    const char* names5[] = {"Alex", "Jacob", "Mark", "Max"};
    run_test_case(names5, 4, "[\"Alex\", \"Jacob\", \"Mark\", \"Max\"]");

    return 0;
}