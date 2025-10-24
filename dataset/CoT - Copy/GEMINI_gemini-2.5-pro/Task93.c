#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

// Helper function to free the memory allocated for the records
void freeRecords(KeyValuePair* records, int count) {
    if (!records) return;
    for (int i = 0; i < count; ++i) {
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// Helper function to trim whitespace from a string (in-place)
char* trim(char* str) {
    char* end;
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str; // All spaces

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';
    return str;
}

// Comparison function for qsort
int compareKeyValuePairs(const void* a, const void* b) {
    KeyValuePair* pairA = (KeyValuePair*)a;
    KeyValuePair* pairB = (KeyValuePair*)b;
    return strcmp(pairA->key, pairB->key);
}

/**
 * Reads a file of key-value pairs, sorts them by key.
 *
 * @param filePath The path to the file.
 * @param recordCount A pointer to an integer to store the number of records read.
 * @return A dynamically allocated array of sorted KeyValuePair structs.
 *         The caller is responsible for freeing this memory using freeRecords().
 *         Returns NULL on error.
 */
KeyValuePair* sortFileRecords(const char* filePath, int* recordCount) {
    *recordCount = 0;
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filePath);
        return NULL;
    }

    int capacity = 10;
    KeyValuePair* records = malloc(capacity * sizeof(KeyValuePair));
    if (!records) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    char line[1024]; // Buffer for reading lines
    while (fgets(line, sizeof(line), file)) {
        char* delimiter = strchr(line, ':');
        if (delimiter) {
            *delimiter = '\0'; // Split the string by replacing ':' with null terminator
            
            char* key = trim(line);
            char* value = trim(delimiter + 1);

            if (strlen(key) > 0) { // Ensure key is not empty
                if (*recordCount >= capacity) {
                    capacity *= 2;
                    KeyValuePair* temp = realloc(records, capacity * sizeof(KeyValuePair));
                    if (!temp) {
                        fprintf(stderr, "Error: Memory reallocation failed.\n");
                        freeRecords(records, *recordCount);
                        fclose(file);
                        return NULL;
                    }
                    records = temp;
                }
                
                // strdup allocates memory for the strings
                records[*recordCount].key = strdup(key);
                records[*recordCount].value = strdup(value);

                // Check for strdup allocation failure
                if (!records[*recordCount].key || !records[*recordCount].value) {
                    fprintf(stderr, "Error: strdup memory allocation failed.\n");
                    free(records[*recordCount].key); // free whichever one succeeded
                    free(records[*recordCount].value);
                    freeRecords(records, *recordCount);
                    fclose(file);
                    return NULL;
                }
                (*recordCount)++;
            }
        }
    }
    fclose(file);

    // Sort the records
    qsort(records, *recordCount, sizeof(KeyValuePair), compareKeyValuePairs);

    return records;
}

// Main function with test cases
void createTestFile(const char* fileName, const char* content) {
    FILE* f = fopen(fileName, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    } else {
        fprintf(stderr, "Failed to create test file: %s\n", fileName);
    }
}

void printRecords(KeyValuePair* records, int count) {
    if (!records) {
        printf("Function returned NULL (likely an error occurred).\n");
        return;
    }
    if (count == 0) {
        printf("No valid records found or processed.\n");
    } else {
        for (int i = 0; i < count; ++i) {
            printf("%s: %s\n", records[i].key, records[i].value);
        }
    }
}

int main() {
    const char* testFileName = "test_records.txt";
    int count;

    // Test Case 1: Standard case with unsorted records
    printf("--- Test Case 1: Standard unsorted file ---\n");
    createTestFile(testFileName, "banana: 10\napple: 5\ncherry: 20\ndate: 15");
    KeyValuePair* sortedRecords1 = sortFileRecords(testFileName, &count);
    printRecords(sortedRecords1, count);
    freeRecords(sortedRecords1, count);
    printf("\n");

    // Test Case 2: File with malformed lines, empty lines, and whitespace
    printf("--- Test Case 2: File with malformed lines and whitespace ---\n");
    createTestFile(testFileName, "  fig: 30  \n\ngrape: 25\njust_a_key\n:empty_key\nelderberry:");
    KeyValuePair* sortedRecords2 = sortFileRecords(testFileName, &count);
    printRecords(sortedRecords2, count);
    freeRecords(sortedRecords2, count);
    printf("\n");

    // Test Case 3: Empty file
    printf("--- Test Case 3: Empty file ---\n");
    createTestFile(testFileName, "");
    KeyValuePair* sortedRecords3 = sortFileRecords(testFileName, &count);
    printRecords(sortedRecords3, count);
    freeRecords(sortedRecords3, count);
    printf("\n");

    // Test Case 4: Non-existent file
    printf("--- Test Case 4: Non-existent file ---\n");
    remove(testFileName); // Ensure file does not exist
    KeyValuePair* sortedRecords4 = sortFileRecords(testFileName, &count);
    printRecords(sortedRecords4, count);
    freeRecords(sortedRecords4, count);
    printf("\n");

    // Test Case 5: Already sorted file
    printf("--- Test Case 5: Already sorted file ---\n");
    createTestFile(testFileName, "alpha: 1\nbeta: 2\ngamma: 3");
    KeyValuePair* sortedRecords5 = sortFileRecords(testFileName, &count);
    printRecords(sortedRecords5, count);
    freeRecords(sortedRecords5, count);
    printf("\n");

    // Cleanup the test file
    remove(testFileName);

    return 0;
}