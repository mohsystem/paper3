
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 1000
#define MAX_ARRAY_SIZE 100000

// Structure to hold name array
typedef struct {
    char** names;
    size_t size;
    size_t capacity;
} NameArray;

// Initialize name array - all pointers set to NULL initially
NameArray* createNameArray(size_t capacity) {
    // Validate capacity to prevent excessive memory allocation
    if (capacity == 0 || capacity > MAX_ARRAY_SIZE) {
        return NULL;
    }
    
    NameArray* arr = (NameArray*)calloc(1, sizeof(NameArray));
    if (arr == NULL) {
        return NULL;
    }
    
    arr->names = (char**)calloc(capacity, sizeof(char*));
    if (arr->names == NULL) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

// Free name array and all allocated strings
void freeNameArray(NameArray* arr) {
    if (arr == NULL) {
        return;
    }
    
    if (arr->names != NULL) {
        for (size_t i = 0; i < arr->size; i++) {
            if (arr->names[i] != NULL) {
                free(arr->names[i]);
                arr->names[i] = NULL;
            }
        }
        free(arr->names);
        arr->names = NULL;
    }
    
    free(arr);
}

// Extract last name from full name - returns NULL on error
char* getLastName(const char* fullName) {
    // Input validation: check for NULL and length
    if (fullName == NULL || strlen(fullName) == 0 || strlen(fullName) > MAX_NAME_LENGTH) {
        return NULL;
    }
    
    // Find last space
    const char* lastSpace = strrchr(fullName, ' ');
    
    // Validate format: must have space and not at boundaries
    if (lastSpace == NULL || lastSpace == fullName || *(lastSpace + 1) == '\\0') {
        return NULL;
    }
    
    // Calculate last name length
    size_t lastNameLen = strlen(lastSpace + 1);
    if (lastNameLen == 0 || lastNameLen > MAX_NAME_LENGTH) {
        return NULL;
    }
    
    // Allocate memory for last name (+1 for null terminator)
    char* lastName = (char*)malloc(lastNameLen + 1);
    if (lastName == NULL) {
        return NULL;
    }
    
    // Safe copy with bounds check
    strncpy(lastName, lastSpace + 1, lastNameLen);
    lastName[lastNameLen] = '\\0'; // Ensure null termination
    
    return lastName;
}

// Comparison function for qsort
int compareNames(const void* a, const void* b) {
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;
    
    // Extract last names
    char* lastName1 = getLastName(name1);
    char* lastName2 = getLastName(name2);
    
    int result = 0;
    
    // Handle NULL cases (invalid entries go to end)
    if (lastName1 == NULL && lastName2 == NULL) {
        result = 0;
    } else if (lastName1 == NULL) {
        result = 1;
    } else if (lastName2 == NULL) {
        result = -1;
    } else {
        size_t len1 = strlen(lastName1);
        size_t len2 = strlen(lastName2);
        
        // Primary sort: by length
        if (len1 != len2) {
            result = (len1 < len2) ? -1 : 1;
        } else {
            // Secondary sort: alphabetically
            result = strcmp(lastName1, lastName2);
        }
    }
    
    // Clean up allocated memory
    if (lastName1 != NULL) {
        free(lastName1);
    }
    if (lastName2 != NULL) {
        free(lastName2);
    }
    
    return result;
}

// Main sorting function
NameArray* lastNameLensort(const char** names, size_t count) {
    // Input validation
    if (names == NULL || count == 0 || count > MAX_ARRAY_SIZE) {
        return NULL;
    }
    
    // Create result array
    NameArray* result = createNameArray(count);
    if (result == NULL) {
        return NULL;
    }
    
    // Copy and validate each name
    for (size_t i = 0; i < count; i++) {
        if (names[i] == NULL || strlen(names[i]) == 0 || strlen(names[i]) > MAX_NAME_LENGTH) {
            freeNameArray(result);
            return NULL;
        }
        
        size_t nameLen = strlen(names[i]);
        result->names[i] = (char*)malloc(nameLen + 1);
        if (result->names[i] == NULL) {
            freeNameArray(result);
            return NULL;
        }
        
        strncpy(result->names[i], names[i], nameLen);
        result->names[i][nameLen] = '\\0';
        result->size++;
    }
    
    // Sort using qsort (safe standard library function)
    qsort(result->names, result->size, sizeof(char*), compareNames);
    
    return result;
}

int main() {
    // Test case 1: Example from prompt
    const char* test1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    NameArray* result1 = lastNameLensort(test1, 5);
    printf("Test 1: ");
    if (result1 != NULL) {
        for (size_t i = 0; i < result1->size; i++) {
            printf("%s, ", result1->names[i]);
        }
        freeNameArray(result1);
    }
    printf("\\n");
    
    // Test case 2: Same length last names
    const char* test2[] = {
        "John Smith",
        "Jane Abbot",
        "Bob Jones"
    };
    NameArray* result2 = lastNameLensort(test2, 3);
    printf("Test 2: ");
    if (result2 != NULL) {
        for (size_t i = 0; i < result2->size; i++) {
            printf("%s, ", result2->names[i]);
        }
        freeNameArray(result2);
    }
    printf("\\n");
    
    // Test case 3: Single name
    const char* test3[] = {"Alice Johnson"};
    NameArray* result3 = lastNameLensort(test3, 1);
    printf("Test 3: ");
    if (result3 != NULL) {
        for (size_t i = 0; i < result3->size; i++) {
            printf("%s, ", result3->names[i]);
        }
        freeNameArray(result3);
    }
    printf("\\n");
    
    // Test case 4: Various lengths
    const char* test4[] = {
        "Michael Anderson",
        "Sarah Lee",
        "David Wu",
        "Emma Thompson"
    };
    NameArray* result4 = lastNameLensort(test4, 4);
    printf("Test 4: ");
    if (result4 != NULL) {
        for (size_t i = 0; i < result4->size; i++) {
            printf("%s, ", result4->names[i]);
        }
        freeNameArray(result4);
    }
    printf("\\n");
    
    // Test case 5: Two names
    const char* test5[] = {
        "Robert Williams",
        "Mary Johnson"
    };
    NameArray* result5 = lastNameLensort(test5, 2);
    printf("Test 5: ");
    if (result5 != NULL) {
        for (size_t i = 0; i < result5->size; i++) {
            printf("%s, ", result5->names[i]);
        }
        freeNameArray(result5);
    }
    printf("\\n");
    
    return 0;
}
