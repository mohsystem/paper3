
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getLastName(const char* fullName) {
    const char* lastSpace = strrchr(fullName, ' ');
    if (lastSpace != NULL) {
        return (char*)(lastSpace + 1);
    }
    return (char*)fullName;
}

int compareNames(const void* a, const void* b) {
    const char* nameA = *(const char**)a;
    const char* nameB = *(const char**)b;
    
    char* lastNameA = getLastName(nameA);
    char* lastNameB = getLastName(nameB);
    
    int lenA = strlen(lastNameA);
    int lenB = strlen(lastNameB);
    
    if (lenA != lenB) {
        return lenA - lenB;
    }
    return strcmp(lastNameA, lastNameB);
}

void lastNameLensort(char** names, int size) {
    if (names == NULL || size <= 0) {
        return;
    }
    qsort(names, size, sizeof(char*), compareNames);
}

void printArray(char** arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    char* test1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    int size1 = 5;
    lastNameLensort(test1, size1);
    printf("Test 1: ");
    printArray(test1, size1);
    
    // Test case 2
    char* test2[] = {
        "John Smith",
        "Jane Doe",
        "Bob Lee"
    };
    int size2 = 3;
    lastNameLensort(test2, size2);
    printf("Test 2: ");
    printArray(test2, size2);
    
    // Test case 3
    char* test3[] = {
        "Alice Johnson",
        "Charlie Brown",
        "David Anderson"
    };
    int size3 = 3;
    lastNameLensort(test3, size3);
    printf("Test 3: ");
    printArray(test3, size3);
    
    // Test case 4
    char* test4[] = {
        "Emily White",
        "Michael Black",
        "Sarah Green"
    };
    int size4 = 3;
    lastNameLensort(test4, size4);
    printf("Test 4: ");
    printArray(test4, size4);
    
    // Test case 5
    char* test5[] = {
        "Tom Martinez",
        "Lisa Garcia",
        "Kevin Rodriguez"
    };
    int size5 = 3;
    lastNameLensort(test5, size5);
    printf("Test 5: ");
    printArray(test5, size5);
    
    return 0;
}
