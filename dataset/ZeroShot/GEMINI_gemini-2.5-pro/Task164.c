#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Helper function to find the start of the last name in a full name string.
 * It is secure against NULL pointers and empty strings.
 * @param fullName The C-style string containing the full name.
 * @return A const pointer to the beginning of the last name, or to an empty string if input is invalid.
 */
const char* getLastName(const char* fullName) {
    if (fullName == NULL || *fullName == '\0') {
        return ""; // Return pointer to a safe, empty string
    }
    const char* last_space = strrchr(fullName, ' ');
    if (last_space == NULL) {
        return fullName; // No space found, the whole name is the last name
    }
    // Skip over the space itself to point to the start of the last name
    return last_space + 1;
}

/**
 * @brief Comparison function for qsort. Compares two full names based on last name rules.
 */
int compareNames(const void* a, const void* b) {
    // Cast void pointers to the actual type (pointer to a char pointer)
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;

    const char* lastName1 = getLastName(name1);
    const char* lastName2 = getLastName(name2);

    size_t len1 = strlen(lastName1);
    size_t len2 = strlen(lastName2);

    // Primary sort: by length of the last name
    if (len1 < len2) return -1;
    if (len1 > len2) return 1;

    // Secondary sort: alphabetically by last name
    return strcmp(lastName1, lastName2);
}

/**
 * @brief Sorts an array of names in place using the specified last name logic.
 * @param names An array of C-style strings.
 * @param count The number of elements in the array.
 */
void lastNameLensort(char** names, int count) {
    if (names == NULL || count <= 1) {
        return; // Nothing to sort or invalid input
    }
    qsort(names, count, sizeof(char*), compareNames);
}

// Helper function to print an array of strings for testing
void print_array(const char* label, char** arr, int count) {
    printf("%s", label);
    printf("[");
    for (int i = 0; i < count; ++i) {
        printf("\"%s\"", arr[i]);
        if (i < count - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Example from prompt
    printf("Test Case 1:\n");
    char* test1[] = {"Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"};
    int count1 = sizeof(test1) / sizeof(test1[0]);
    print_array("Input: ", test1, count1);
    lastNameLensort(test1, count1);
    print_array("Output: ", test1, count1);
    printf("\n");

    // Test Case 2: Last names with same length
    printf("Test Case 2:\n");
    char* test2[] = {"Steven King", "John Smith", "Jane Doe"};
    int count2 = sizeof(test2) / sizeof(test2[0]);
    print_array("Input: ", test2, count2);
    lastNameLensort(test2, count2);
    print_array("Output: ", test2, count2);
    printf("\n");

    // Test Case 3: Names without spaces
    printf("Test Case 3:\n");
    char* test3[] = {"Beyonce", "Adele", "Cher"};
    int count3 = sizeof(test3) / sizeof(test3[0]);
    print_array("Input: ", test3, count3);
    lastNameLensort(test3, count3);
    print_array("Output: ", test3, count3);
    printf("\n");

    // Test Case 4: Mixed names (with and without spaces)
    printf("Test Case 4:\n");
    char* test4[] = {"David Copperfield", "David Blaine", "Penn"};
    int count4 = sizeof(test4) / sizeof(test4[0]);
    print_array("Input: ", test4, count4);
    lastNameLensort(test4, count4);
    print_array("Output: ", test4, count4);
    printf("\n");
    
    // Test Case 5: Edge cases
    printf("Test Case 5:\n");
    char* test5[] = {"", " ", "Single", "Peter Pan"};
    int count5 = sizeof(test5) / sizeof(test5[0]);
    print_array("Input: ", test5, count5);
    lastNameLensort(test5, count5);
    print_array("Output: ", test5, count5);
    printf("\n");
    
    return 0;
}