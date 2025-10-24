#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Struct to hold a key-value pair
typedef struct {
    char *key;
    char *value;
} KeyValuePair;

// Comparison function for qsort
static int compareKeyValuePairs(const void *a, const void *b) {
    KeyValuePair *pairA = (KeyValuePair *)a;
    KeyValuePair *pairB = (KeyValuePair *)b;
    return strcmp(pairA->key, pairB->key);
}

// Helper function to free the allocated memory for key-value pairs
void freeKeyValuePairs(KeyValuePair *pairs, int count) {
    if (pairs == NULL) return;
    for (int i = 0; i < count; i++) {
        free(pairs[i].key);   // Free the duplicated string
        free(pairs[i].value); // Free the duplicated string
    }
    free(pairs); // Free the array of structs
}

// Helper function to trim whitespace from a string (in-place)
char *trim(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
}


/**
 * Reads a file of key-value records, sorts them by key, and returns a dynamic array.
 *
 * @param filePath The path to the file.
 * @param count    A pointer to an integer to store the number of records read.
 * @return         A dynamically allocated array of KeyValuePair structs, or NULL on error.
 *                 The caller is responsible for freeing the returned array using freeKeyValuePairs.
 */
KeyValuePair* sortFileRecords(const char* filePath, int* count) {
    *count = 0;
    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    KeyValuePair *records = NULL;
    int capacity = 10;
    records = malloc(capacity * sizeof(KeyValuePair));
    if (!records) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    char line_buffer[1024]; // A buffer for reading lines
    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        char* trimmed_line = trim(line_buffer);
        if (strlen(trimmed_line) == 0) {
            continue; // Skip empty lines
        }

        char *delimiter = strchr(trimmed_line, ':');
        if (delimiter) {
            *delimiter = '\0'; // Null-terminate the key
            char *key_str = trimmed_line;
            char *value_str = delimiter + 1;
            
            char* trimmed_key = trim(key_str);
            char* trimmed_value = trim(value_str);

            if (strlen(trimmed_key) == 0) continue; // Skip if key is empty after trim

            // Resize array if necessary
            if (*count >= capacity) {
                capacity *= 2;
                KeyValuePair *temp = realloc(records, capacity * sizeof(KeyValuePair));
                if (!temp) {
                    perror("Failed to reallocate memory");
                    freeKeyValuePairs(records, *count);
                    fclose(file);
                    return NULL;
                }
                records = temp;
            }
            
            // strdup allocates memory for the strings, which must be freed later
            records[*count].key = strdup(trimmed_key);
            records[*count].value = strdup(trimmed_value);
            if (!records[*count].key || !records[*count].value) {
                perror("Failed to duplicate string");
                free(records[*count].key); // strdup may have succeeded for key
                free(records[*count].value); // or value
                freeKeyValuePairs(records, *count);
                fclose(file);
                return NULL;
            }
            (*count)++;
        } else {
             fprintf(stderr, "Warning: Skipping malformed line: %s\n", trimmed_line);
        }
    }

    fclose(file);
    
    if (*count > 0) {
        qsort(records, *count, sizeof(KeyValuePair), compareKeyValuePairs);
    }
    
    return records;
}

// --- Test Cases ---

void setupTestFiles() {
    FILE *f;
    f = fopen("test_ok.txt", "w");
    if(f) { fprintf(f, "zulu:last\nalpha:first\ncharlie:third\nbravo:second\n"); fclose(f); }
    f = fopen("test_malformed.txt", "w");
    if(f) { fprintf(f, "key1:value1\nmalformedline\nkey2:value2\nanother:malformed:line\n"); fclose(f); }
    f = fopen("test_empty.txt", "w");
    if(f) { fclose(f); }
    f = fopen("test_blank_lines.txt", "w");
    if(f) { fprintf(f, "keyA:valueA\n\n  \nkeyC:valueC\nkeyB:valueB\n"); fclose(f); }
}

void cleanupTestFiles() {
    remove("test_ok.txt");
    remove("test_malformed.txt");
    remove("test_empty.txt");
    remove("test_blank_lines.txt");
}

int main() {
    setupTestFiles();

    const char* testFiles[] = {
        "test_ok.txt",
        "test_malformed.txt",
        "test_empty.txt",
        "test_blank_lines.txt",
        "non_existent_file.txt"
    };
    int num_tests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d: Processing %s ---\n", i + 1, testFiles[i]);
        int count = 0;
        KeyValuePair *result = sortFileRecords(testFiles[i], &count);

        if (result == NULL || count == 0) {
            printf("Result is empty (as expected for empty, malformed, or non-existent files).\n");
        } else {
            for (int j = 0; j < count; j++) {
                printf("%s:%s\n", result[j].key, result[j].value);
            }
        }
        
        freeKeyValuePairs(result, count);
        printf("\n");
    }

    cleanupTestFiles();

    return 0;
}