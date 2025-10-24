#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Gets a pointer to the last name part of a full name string.
 * If no space is found, returns a pointer to the original string.
 * @param fullName The full name string.
 * @return Pointer to the beginning of the last name.
 */
const char* getLastName(const char* fullName) {
    if (fullName == NULL) {
        return "";
    }
    const char* lastSpace = strrchr(fullName, ' ');
    if (lastSpace == NULL) {
        return fullName; // No space, treat whole name as last name
    }
    return lastSpace + 1; // Pointer to char after space
}

/**
 * Comparison function for qsort.
 * Compares two names based on last name length, then alphabetically.
 */
int compareNames(const void* a, const void* b) {
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;

    const char* lastName1 = getLastName(name1);
    const char* lastName2 = getLastName(name2);

    size_t len1 = strlen(lastName1);
    size_t len2 = strlen(lastName2);

    if (len1 != len2) {
        return (len1 > len2) - (len1 < len2); // return -1, 0, or 1
    } else {
        return strcmp(lastName1, lastName2);
    }
}

/**
 * Sorts an array of names in-place using qsort.
 * @param names Array of C strings.
 * @param size Number of elements in the array.
 */
void lastNameLensort(const char* names[], int size) {
    if (names == NULL || size <= 1) {
        return;
    }
    qsort(names, size, sizeof(const char*), compareNames);
}

void printArray(const char* arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("\"%s\"", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1
    const char* test1[] = {
        "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz",
        "Nicole Yoder", "Melissa Hoffman"
    };
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1:\n");
    printf("Original: ");
    printArray(test1, size1);
    printf("\n");
    lastNameLensort(test1, size1);
    printf("Sorted:   ");
    printArray(test1, size1);
    printf("\n\n");

    // Test Case 2
    const char* test2[] = {
        "John Smith", "Jane Doe", "Peter Jones", "Mary Johnson"
    };
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2:\n");
    printf("Original: ");
    printArray(test2, size2);
    printf("\n");
    lastNameLensort(test2, size2);
    printf("Sorted:   ");
    printArray(test2, size2);
    printf("\n\n");

    // Test Case 3
    const char* test3[] = {
        "alpha beta", "gamma delta"
    };
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3:\n");
    printf("Original: ");
    printArray(test3, size3);
    printf("\n");
    lastNameLensort(test3, size3);
    printf("Sorted:   ");
    printArray(test3, size3);
    printf("\n\n");
    
    // Test Case 4
    const char* test4[] = {
        "A B", "C D", "E F"
    };
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4:\n");
    printf("Original: ");
    printArray(test4, size4);
    printf("\n");
    lastNameLensort(test4, size4);
    printf("Sorted:   ");
    printArray(test4, size4);
    printf("\n\n");
    
    // Test Case 5
    const char* test5[] = {
        "SingleName", "Another SingleName"
    };
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5:\n");
    printf("Original: ");
    printArray(test5, size5);
    printf("\n");
    lastNameLensort(test5, size5);
    printf("Sorted:   ");
    printArray(test5, size5);
    printf("\n\n");

    return 0;
}