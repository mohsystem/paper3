#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
void freeNameArray(char** names, size_t count);

// Comparator function for qsort
int compareNames(const void* a, const void* b) {
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;

    const char* lastName1 = strrchr(name1, ' ');
    lastName1 = (lastName1 == NULL) ? name1 : lastName1 + 1;

    const char* lastName2 = strrchr(name2, ' ');
    lastName2 = (lastName2 == NULL) ? name2 : lastName2 + 1;

    size_t len1 = strlen(lastName1);
    size_t len2 = strlen(lastName2);

    if (len1 < len2) return -1;
    if (len1 > len2) return 1;

    return strcmp(lastName1, lastName2);
}

// Function to sort names. Returns a new dynamically allocated array.
// The caller is responsible for freeing the memory using freeNameArray.
char** lastNameLensort(const char* const* names, size_t count) {
    char** sortedNames = (char**)malloc(count * sizeof(char*));
    if (sortedNames == NULL && count > 0) {
        return NULL; 
    }

    for (size_t i = 0; i < count; ++i) {
        size_t len = strlen(names[i]) + 1;
        sortedNames[i] = (char*)malloc(len);
        if (sortedNames[i] == NULL) {
            freeNameArray(sortedNames, i); // Free already allocated strings
            return NULL;
        }
        memcpy(sortedNames[i], names[i], len);
    }

    if (count > 0) {
        qsort(sortedNames, count, sizeof(char*), compareNames);
    }

    return sortedNames;
}

// Helper to free the memory allocated by lastNameLensort
void freeNameArray(char** names, size_t count) {
    if (names == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(names[i]);
    }
    free(names);
}

void printNameArray(const char* const* names, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        printf("\"%s\"", names[i]);
        if (i < count - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void runTestCase(const char* testName, const char* const* names, size_t count) {
    printf("%s:\n", testName);
    printf("Input: ");
    printNameArray(names, count);
    printf("\n");

    char** sortedNames = lastNameLensort(names, count);
    
    printf("Output: ");
    if (sortedNames == NULL && count > 0) {
        printf("Memory allocation failed.\n");
    } else {
        printNameArray((const char* const*)sortedNames, count);
    }
    printf("\n\n");
    
    freeNameArray(sortedNames, count);
}

int main() {
    // Test Case 1
    const char* names1[] = {
        "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"
    };
    runTestCase("Test Case 1", names1, sizeof(names1) / sizeof(names1[0]));

    // Test Case 2: Same last name length
    const char* names2[] = {"John Smith", "Jane Doe", "Peter Jones"};
    runTestCase("Test Case 2", names2, sizeof(names2) / sizeof(names2[0]));

    // Test Case 3: Empty array
    const char* names3[] = {};
    runTestCase("Test Case 3", names3, 0);

    // Test Case 4: Single-word names
    const char* names4[] = {"Cher", "Madonna", "Prince"};
    runTestCase("Test Case 4", names4, sizeof(names4) / sizeof(names4[0]));

    // Test Case 5: Names with same last names
    const char* names5[] = {"David Lee", "Bruce Lee", "Sara Lee"};
    runTestCase("Test Case 5", names5, sizeof(names5) / sizeof(names5[0]));

    return 0;
}