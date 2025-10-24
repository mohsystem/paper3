#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Helper to get a pointer to the last name in a full name string.
 * @param fullName The full name C-style string.
 * @return A pointer to the beginning of the last name.
 */
const char* getLastName(const char* fullName) {
    const char* lastName = strrchr(fullName, ' ');
    if (lastName == NULL) {
        return fullName; // No space found, treat the whole string as the last name
    }
    return lastName + 1; // Return pointer to the character after the space
}

/**
 * Comparison function for qsort to sort names.
 */
int compareNames(const void* a, const void* b) {
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;

    const char* lastName1 = getLastName(name1);
    const char* lastName2 = getLastName(name2);

    size_t len1 = strlen(lastName1);
    size_t len2 = strlen(lastName2);

    if (len1 < len2) return -1;
    if (len1 > len2) return 1;
    
    // If lengths are equal, sort alphabetically by last name
    return strcmp(lastName1, lastName2);
}

/**
 * Sorts an array of names based on the length of the last name, then alphabetically by last name.
 * This function sorts the array in-place.
 * @param names An array of C-style strings.
 * @param count The number of elements in the array.
 */
void lastNameLensort(char* names[], int count) {
    qsort(names, count, sizeof(char*), compareNames);
}

/**
 * Helper function to print an array of C-style strings.
 * @param arr The array to print.
 * @param size The number of elements in the array.
 */
void printArray(char* arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("\"%s\"", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    char* test1[] = {"Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"};
    int count1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1:\n");
    printf("Input: ");
    printArray(test1, count1);
    lastNameLensort(test1, count1);
    printf("Output: ");
    printArray(test1, count1);
    printf("\n");
    
    // Test Case 2
    char* test2[] = {"A B", "C D", "E F"};
    int count2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2:\n");
    printf("Input: ");
    printArray(test2, count2);
    lastNameLensort(test2, count2);
    printf("Output: ");
    printArray(test2, count2);
    printf("\n");

    // Test Case 3
    char* test3[] = {"Morty Smith", "Rick Sanchez", "Jerry Smith"};
    int count3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3:\n");
    printf("Input: ");
    printArray(test3, count3);
    lastNameLensort(test3, count3);
    printf("Output: ");
    printArray(test3, count3);
    printf("\n");

    // Test Case 4
    char* test4[] = {"Peter Parker", "Clark Kent", "Bruce Wayne"};
    int count4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4:\n");
    printf("Input: ");
    printArray(test4, count4);
    lastNameLensort(test4, count4);
    printf("Output: ");
    printArray(test4, count4);
    printf("\n");
    
    // Test Case 5
    char* test5[] = {"Leo Tolstoy", "Jane Austen", "Charles Dickens"};
    int count5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5:\n");
    printf("Input: ");
    printArray(test5, count5);
    lastNameLensort(test5, count5);
    printf("Output: ");
    printArray(test5, count5);
    printf("\n");

    return 0;
}