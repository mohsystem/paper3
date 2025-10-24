#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates a display text for who likes an item based on an array of names.
 * Note: The caller is responsible for freeing the memory of the returned string.
 * @param names An array of C-strings containing the names.
 * @param count The number of names in the array.
 * @return A dynamically allocated string with the formatted text, or NULL on failure.
 */
char* whoLikesIt(const char* names[], int count) {
    char* result = NULL;
    size_t size;

    switch (count) {
        case 0:
            size = strlen("no one likes this") + 1;
            result = (char*)malloc(size);
            if (result == NULL) return NULL;
            strcpy(result, "no one likes this");
            break;
        case 1: 
            size = strlen(names[0]) + strlen(" likes this") + 1;
            result = (char*)malloc(size);
            if (result == NULL) return NULL;
            snprintf(result, size, "%s likes this", names[0]);
            break;
        case 2:
            size = strlen(names[0]) + strlen(names[1]) + strlen(" and  like this") + 1;
            result = (char*)malloc(size);
            if (result == NULL) return NULL;
            snprintf(result, size, "%s and %s like this", names[0], names[1]);
            break;
        case 3:
            size = strlen(names[0]) + strlen(names[1]) + strlen(names[2]) + strlen(",  and  like this") + 1;
            result = (char*)malloc(size);
            if (result == NULL) return NULL;
            snprintf(result, size, "%s, %s and %s like this", names[0], names[1], names[2]);
            break;
        default: {
            int others = count - 2;
            // Allocate enough space for names, fixed text, and the integer part (10 digits is safe)
            size = strlen(names[0]) + strlen(names[1]) + strlen(",  and  others like this") + 10 + 1;
            result = (char*)malloc(size);
            if (result == NULL) return NULL;
            snprintf(result, size, "%s, %s and %d others like this", names[0], names[1], others);
            break;
        }
    }
    return result;
}

void run_test_and_free(const char* names[], int count) {
    char* output = whoLikesIt(names, count);
    if (output) {
        printf("%s\n", output);
        free(output); // Free the memory allocated by whoLikesIt
    } else {
        fprintf(stderr, "Memory allocation failed.\n");
    }
}

int main() {
    // Test Case 1: 0 names
    const char* names1[] = {};
    run_test_and_free(names1, 0);

    // Test Case 2: 1 name
    const char* names2[] = {"Peter"};
    run_test_and_free(names2, 1);
    
    // Test Case 3: 2 names
    const char* names3[] = {"Jacob", "Alex"};
    run_test_and_free(names3, 2);

    // Test Case 4: 3 names
    const char* names4[] = {"Max", "John", "Mark"};
    run_test_and_free(names4, 3);
    
    // Test Case 5: 4 or more names
    const char* names5[] = {"Alex", "Jacob", "Mark", "Max"};
    run_test_and_free(names5, 4);

    return 0;
}