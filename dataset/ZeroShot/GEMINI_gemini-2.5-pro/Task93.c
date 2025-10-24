#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 16
#define LINE_BUFFER_SIZE 2048

typedef struct {
    char *key;
    char *value;
} KeyValue;

// Comparison function for qsort, sorts by key alphabetically.
int compareKeyValues(const void *a, const void *b) {
    const KeyValue *kv_a = (const KeyValue *)a;
    const KeyValue *kv_b = (const KeyValue *)b;
    return strcmp(kv_a->key, kv_b->key);
}

// Frees the memory allocated for the key-value pairs.
void freeKeyValues(KeyValue *records, size_t count) {
    if (!records) return;
    for (size_t i = 0; i < count; i++) {
        free(records[i].key);   // key and value were allocated with strdup
        free(records[i].value);
    }
    free(records);
}

// Helper to trim leading/trailing whitespace from a string, modifies the string in place
char *trim(char *str) {
    if (!str) return NULL;
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str; // All spaces

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
    return str;
}


/**
 * Reads a file of key-value records, sorts them, and returns them in a dynamic array.
 * @param filepath Path to the file.
 * @param recordCount Pointer to a size_t to store the number of records read.
 * @return A dynamically allocated array of KeyValue structs, or NULL on failure.
 *         The caller is responsible for freeing the returned array and its contents
 *         using the freeKeyValues function.
 */
KeyValue* readAndSortFile(const char *filepath, size_t *recordCount) {
    *recordCount = 0;
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    size_t capacity = INITIAL_CAPACITY;
    KeyValue *records = malloc(capacity * sizeof(KeyValue));
    if (!records) {
        perror("Failed to allocate memory for records");
        fclose(file);
        return NULL;
    }

    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = trim(line);
        if (strlen(trimmed_line) == 0) {
            continue; // Skip empty lines
        }
        
        char *delimiter = strchr(trimmed_line, ':');
        if (delimiter) {
            *delimiter = '\0'; // Split the string into key and value
            char *key = trim(trimmed_line);
            char *value = trim(delimiter + 1);

            if (strlen(key) == 0) { // Ignore records with empty keys
                continue;
            }

            if (*recordCount >= capacity) {
                capacity *= 2;
                KeyValue *temp = realloc(records, capacity * sizeof(KeyValue));
                if (!temp) {
                    perror("Failed to reallocate memory");
                    freeKeyValues(records, *recordCount);
                    fclose(file);
                    return NULL;
                }
                records = temp;
            }

            // strdup allocates memory for the copies
            records[*recordCount].key = strdup(key);
            records[*recordCount].value = strdup(value);

            if (!records[*recordCount].key || !records[*recordCount].value) {
                perror("Failed to allocate memory for key/value string");
                free(records[*recordCount].key); // free whichever one succeeded
                free(records[*recordCount].value);
                freeKeyValues(records, *recordCount);
                fclose(file);
                return NULL;
            }
            (*recordCount)++;
        }
    }

    fclose(file);

    if (*recordCount > 0) {
        qsort(records, *recordCount, sizeof(KeyValue), compareKeyValues);
    }
    
    return records;
}

void runTestCase(int testNum, const char* content) {
    const char* testFileName = "test_case.txt";
    printf("--- Running Test Case %d ---\n", testNum);

    FILE* testFile = fopen(testFileName, "w");
    if (!testFile) {
        perror("Failed to create test file");
        return;
    }
    fputs(content, testFile);
    fclose(testFile);

    size_t recordCount = 0;
    KeyValue* sortedRecords = readAndSortFile(testFileName, &recordCount);

    if (!sortedRecords && recordCount == 0) {
        // This indicates an error occurred, which was already printed by perror.
    } else if (recordCount == 0) {
        printf("No valid records found or file was empty.\n");
    } else {
        for (size_t i = 0; i < recordCount; i++) {
            printf("%s:%s\n", sortedRecords[i].key, sortedRecords[i].value);
        }
    }
    
    freeKeyValues(sortedRecords, recordCount);
    remove(testFileName);
    printf("\n");
}

int main() {
    const char* testContents[] = {
        // Test Case 1: Standard case with mixed order
        "banana:yellow\napple:red\norange:orange\ngrape:purple",
        // Test Case 2: Empty file
        "",
        // Test Case 3: File with malformed lines, empty lines, and extra whitespace
        "name: Alice\n\nage: 30\noccupation :Software Engineer\nlocation\n\ncity:New York",
        // Test Case 4: Case sensitivity test
        "Apple:fruit\napple:fruit\nZebra:animal\nzoo:place",
        // Test Case 5: Special characters in values and keys
        "url:http://example.com?a=1&b=2\n$pecial-key:value with spaces\nemail:test@example.com"
    };
    int numTests = sizeof(testContents) / sizeof(testContents[0]);
    for (int i = 0; i < numTests; i++) {
        runTestCase(i + 1, testContents[i]);
    }

    return 0;
}