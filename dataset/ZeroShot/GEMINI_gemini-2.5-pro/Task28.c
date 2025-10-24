#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates the display text for people who liked an item.
 * IMPORTANT: The returned string is dynamically allocated and must be freed by the caller.
 *
 * @param names An array of C-style strings of people who liked the item.
 * @param count The number of names in the array.
 * @return A dynamically allocated string with the formatted text, or NULL on error.
 */
char* whoLikesIt(const char* const names[], size_t count) {
    char* result = NULL;
    int required_size = 0;

    switch (count) {
        case 0:
            // For a constant string, we can use strdup for consistency in memory management.
            result = (char*)malloc(strlen("no one likes this") + 1);
            if (result == NULL) return NULL; // Allocation check
            strcpy(result, "no one likes this");
            break;
        case 1:
            // Using snprintf is crucial for security. It prevents buffer overflows.
            // First, calculate the required buffer size.
            required_size = snprintf(NULL, 0, "%s likes this", names[0]);
            if (required_size < 0) return NULL; // Encoding error check
            result = (char*)malloc(required_size + 1); // +1 for null terminator
            if (result == NULL) return NULL; // Allocation check
            snprintf(result, required_size + 1, "%s likes this", names[0]);
            break;
        case 2:
            required_size = snprintf(NULL, 0, "%s and %s like this", names[0], names[1]);
            if (required_size < 0) return NULL;
            result = (char*)malloc(required_size + 1);
            if (result == NULL) return NULL;
            snprintf(result, required_size + 1, "%s and %s like this", names[0], names[1]);
            break;
        case 3:
            required_size = snprintf(NULL, 0, "%s, %s and %s like this", names[0], names[1], names[2]);
            if (required_size < 0) return NULL;
            result = (char*)malloc(required_size + 1);
            if (result == NULL) return NULL;
            snprintf(result, required_size + 1, "%s, %s and %s like this", names[0], names[1], names[2]);
            break;
        default:
            required_size = snprintf(NULL, 0, "%s, %s and %zu others like this", names[0], names[1], count - 2);
            if (required_size < 0) return NULL;
            result = (char*)malloc(required_size + 1);
            if (result == NULL) return NULL;
            snprintf(result, required_size + 1, "%s, %s and %zu others like this", names[0], names[1], count - 2);
            break;
    }
    return result;
}

// Helper function to run a test case and manage memory
void run_test(const char* test_name, const char* const names[], size_t count, const char* expected) {
    printf("%s\n", test_name);
    printf("Expected: %s\n", expected);
    char* actual = whoLikesIt(names, count);
    if (actual) {
        printf("Actual:   %s\n", actual);
        free(actual); // Caller is responsible for freeing memory
    } else {
        printf("Actual:   (null) - memory allocation failed\n");
    }
    printf("--------------------\n");
}

int main() {
    // Test Cases
    run_test("Test Case 1: []", NULL, 0, "no one likes this");

    const char* names2[] = {"Peter"};
    run_test("Test Case 2: [\"Peter\"]", names2, 1, "Peter likes this");

    const char* names3[] = {"Jacob", "Alex"};
    run_test("Test Case 3: [\"Jacob\", \"Alex\"]", names3, 2, "Jacob and Alex like this");

    const char* names4[] = {"Max", "John", "Mark"};
    run_test("Test Case 4: [\"Max\", \"John\", \"Mark\"]", names4, 3, "Max, John and Mark like this");
    
    const char* names5[] = {"Alex", "Jacob", "Mark", "Max"};
    run_test("Test Case 5: [\"Alex\", \"Jacob\", \"Mark\", \"Max\"]", names5, 4, "Alex, Jacob and 2 others like this");

    return 0;
}