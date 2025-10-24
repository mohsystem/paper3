
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 256
#define MAX_NAMES 100

char* extractLastName(const char* fullName, char* lastName) {
    if (fullName == NULL || lastName == NULL) {
        if (lastName != NULL) lastName[0] = '\\0';
        return lastName;
    }
    
    // Find start and end of trimmed string
    const char* start = fullName;
    while (*start && isspace(*start)) start++;
    
    if (*start == '\\0') {
        lastName[0] = '\\0';
        return lastName;
    }
    
    const char* end = fullName + strlen(fullName) - 1;
    while (end > start && isspace(*end)) end--;
    
    // Find last space
    const char* lastSpace = NULL;
    for (const char* p = start; p <= end; p++) {
        if (*p == ' ') {
            lastSpace = p;
        }
    }
    
    if (lastSpace == NULL) {
        // No space found, copy entire trimmed string
        size_t len = end - start + 1;
        strncpy(lastName, start, len);
        lastName[len] = '\\0';
    } else {
        // Copy from last space to end
        size_t len = end - lastSpace;
        strncpy(lastName, lastSpace + 1, len);
        lastName[len] = '\\0';
    }
    
    return lastName;
}

int compareNames(const void* a, const void* b) {
    char lastName1[MAX_NAME_LENGTH];
    char lastName2[MAX_NAME_LENGTH];
    
    extractLastName(*(const char**)a, lastName1);
    extractLastName(*(const char**)b, lastName2);
    
    size_t len1 = strlen(lastName1);
    size_t len2 = strlen(lastName2);
    
    // Compare by length first
    if (len1 != len2) {
        return len1 - len2;
    }
    
    // If lengths are equal, compare alphabetically
    return strcmp(lastName1, lastName2);
}

void lastNameLensort(char** names, int count) {
    if (names == NULL || count <= 0) {
        return;
    }
    
    qsort(names, count, sizeof(char*), compareNames);
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
    int count1 = 5;
    lastNameLensort(test1, count1);
    printf("Test 1: ");
    for (int i = 0; i < count1; i++) {
        printf("%s, ", test1[i]);
    }
    printf("\\n");
    
    // Test case 2
    char* test2[] = {"John Doe", "Jane Smith", "Bob Lee"};
    int count2 = 3;
    lastNameLensort(test2, count2);
    printf("Test 2: ");
    for (int i = 0; i < count2; i++) {
        printf("%s, ", test2[i]);
    }
    printf("\\n");
    
    // Test case 3
    char* test3[] = {"Alice Brown", "Charlie Brown", "David Green"};
    int count3 = 3;
    lastNameLensort(test3, count3);
    printf("Test 3: ");
    for (int i = 0; i < count3; i++) {
        printf("%s, ", test3[i]);
    }
    printf("\\n");
    
    // Test case 4 - Empty array
    char** test4 = NULL;
    lastNameLensort(test4, 0);
    printf("Test 4: (empty)\\n");
    
    // Test case 5 - Single name
    char* test5[] = {"Michael Jackson"};
    int count5 = 1;
    lastNameLensort(test5, count5);
    printf("Test 5: ");
    for (int i = 0; i < count5; i++) {
        printf("%s, ", test5[i]);
    }
    printf("\\n");
    
    return 0;
}
