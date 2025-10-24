#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024

// Structure to hold a key-value pair
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

// Comparison function for qsort
int compareKeyValuePairs(const void* a, const void* b) {
    KeyValuePair* pairA = (KeyValuePair*)a;
    KeyValuePair* pairB = (KeyValuePair*)b;
    return strcmp(pairA->key, pairB->key);
}

// Function to trim leading/trailing whitespace from a string in-place
char* trim(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

// Reads, sorts, and prints key-value pairs from a file.
void sortKeyValueFile(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filePath);
        return;
    }

    KeyValuePair* records = NULL;
    size_t count = 0;
    size_t capacity = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file)) {
        char* delimiter = strchr(line, ':');
        if (delimiter != NULL) {
            *delimiter = '\0'; // Split the string
            char* key_raw = line;
            char* value_raw = delimiter + 1;
            
            char* key = trim(key_raw);
            char* value = trim(value_raw);

            if (strlen(key) == 0) { // Skip if key is empty after trimming
                continue;
            }

            // Check for duplicate keys and update if found
            int found = 0;
            for(size_t i = 0; i < count; ++i) {
                if(strcmp(records[i].key, key) == 0) {
                    free(records[i].value); // Free old value
                    records[i].value = strdup(value);
                    if (records[i].value == NULL) {
                        fprintf(stderr, "Memory allocation failed for value.\n");
                        // In a real app, handle this more gracefully
                        fclose(file);
                        return;
                    }
                    found = 1;
                    break;
                }
            }
            
            if(!found) {
                // Grow the dynamic array if needed
                if (count >= capacity) {
                    capacity = (capacity == 0) ? 8 : capacity * 2;
                    KeyValuePair* temp = realloc(records, capacity * sizeof(KeyValuePair));
                    if (temp == NULL) {
                        fprintf(stderr, "Memory reallocation failed.\n");
                        fclose(file);
                        // Free existing records before exiting
                        for (size_t i = 0; i < count; ++i) {
                            free(records[i].key);
                            free(records[i].value);
                        }
                        free(records);
                        return;
                    }
                    records = temp;
                }
                
                // strdup is not standard C99, but common. Using malloc+strcpy for portability.
                records[count].key = malloc(strlen(key) + 1);
                records[count].value = malloc(strlen(value) + 1);
                
                if (records[count].key == NULL || records[count].value == NULL) {
                    fprintf(stderr, "Memory allocation failed.\n");
                    // Clean up and exit
                    free(records[count].key);
                    free(records[count].value);
                    fclose(file);
                    // Free all previous records
                    for(size_t i=0; i < count; ++i) {
                        free(records[i].key);
                        free(records[i].value);
                    }
                    free(records);
                    return;
                }

                strcpy(records[count].key, key);
                strcpy(records[count].value, value);
                count++;
            }
        }
    }
    fclose(file);

    // Sort the records
    if (count > 0) {
        qsort(records, count, sizeof(KeyValuePair), compareKeyValuePairs);
    }
    
    // Print and free memory
    for (size_t i = 0; i < count; ++i) {
        printf("%s: %s\n", records[i].key, records[i].value);
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// Helper function to create a test file
void createTestFile(const char* fileName, const char* content) {
    FILE* f = fopen(fileName, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    } else {
        fprintf(stderr, "Failed to create test file: %s\n", fileName);
    }
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Normal file with unsorted data
    printf("--- Test Case 1: Normal File ---\n");
    const char* testFile1 = "test1.txt";
    createTestFile(testFile1, "banana:fruit\napple:fruit\ncarrot:vegetable\nzucchini:vegetable\n");
    sortKeyValueFile(testFile1);
    printf("\n");

    // Test Case 2: Empty file
    printf("--- Test Case 2: Empty File ---\n");
    const char* testFile2 = "test2.txt";
    createTestFile(testFile2, "");
    sortKeyValueFile(testFile2);
    printf("\n");

    // Test Case 3: File with malformed lines
    printf("--- Test Case 3: Malformed Lines ---\n");
    const char* testFile3 = "test3.txt";
    createTestFile(testFile3, "one:1\ntwo_malformed\nthree:3\n :missing_key\nfour::4\n");
    sortKeyValueFile(testFile3);
    printf("\n");

    // Test Case 4: Non-existent file
    printf("--- Test Case 4: Non-existent File ---\n");
    const char* testFile4 = "non_existent_file.txt";
    sortKeyValueFile(testFile4);
    printf("\n");

    // Test Case 5: File with duplicate keys (last one should win)
    printf("--- Test Case 5: Duplicate Keys ---\n");
    const char* testFile5 = "test5.txt";
    createTestFile(testFile5, "key1:valueA\nkey2:valueB\nkey1:valueC\n");
    sortKeyValueFile(testFile5);
    printf("\n");

    // Cleanup test files
    remove(testFile1);
    remove(testFile2);
    remove(testFile3);
    remove(testFile5);
    
    return 0;
}