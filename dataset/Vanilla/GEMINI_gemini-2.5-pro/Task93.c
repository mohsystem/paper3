#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Struct to hold a key-value pair
typedef struct {
    char* key;
    char* value;
} KeyValue;

// Function to free the memory used by an array of KeyValue
void freeRecords(KeyValue* records, size_t count) {
    if (records == NULL) return;
    for (size_t i = 0; i < count; i++) {
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// Comparison function for qsort, sorts by key
int compareKeyValues(const void* a, const void* b) {
    const KeyValue* kv_a = (const KeyValue*)a;
    const KeyValue* kv_b = (const KeyValue*)b;
    return strcmp(kv_a->key, kv_b->key);
}

// Helper to duplicate a trimmed string. The caller must free the returned string.
char* strdup_trimmed(const char* str) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    const char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    size_t len = (end < str) ? 0 : (size_t)(end - str) + 1;
    char* new_str = (char*)malloc(len + 1);
    if (new_str) {
        strncpy(new_str, str, len);
        new_str[len] = '\0';
    }
    return new_str;
}

// Reads records from a file, sorts them by key, and returns a dynamic array.
// The number of records read is stored in recordCount.
// The caller is responsible for freeing the returned array.
KeyValue* sortFileRecords(const char* filePath, size_t* recordCount) {
    *recordCount = 0;
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filePath);
        return NULL;
    }

    size_t capacity = 10;
    size_t count = 0;
    KeyValue* records = (KeyValue*)malloc(capacity * sizeof(KeyValue));
    if (!records) {
        fprintf(stderr, "Error: Failed to allocate memory for records.\n");
        fclose(file);
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity *= 2;
            KeyValue* temp = (KeyValue*)realloc(records, capacity * sizeof(KeyValue));
            if (!temp) {
                fprintf(stderr, "Error: Failed to reallocate memory.\n");
                freeRecords(records, count);
                fclose(file);
                return NULL;
            }
            records = temp;
        }

        char* delimiter = strchr(line, ':');
        if (delimiter) {
            *delimiter = '\0';
            records[count].key = strdup_trimmed(line);
            records[count].value = strdup_trimmed(delimiter + 1);

            if (!records[count].key || !records[count].value) {
                fprintf(stderr, "Error: Memory allocation failed for key/value.\n");
                free(records[count].key);
                free(records[count].value);
                freeRecords(records, count);
                fclose(file);
                return NULL;
            }
            count++;
        }
    }
    fclose(file);
    
    qsort(records, count, sizeof(KeyValue), compareKeyValues);
    
    *recordCount = count;
    return records;
}

void runTestCase(int testNum, const char* fileName, const char* content) {
    printf("--- Test Case %d (%s) ---\n", testNum, fileName);
    
    FILE* f = fopen(fileName, "w");
    if (!f) {
        fprintf(stderr, "Failed to create test file.\n");
        return;
    }
    fprintf(f, "%s", content);
    fclose(f);

    size_t recordCount = 0;
    KeyValue* sortedRecords = sortFileRecords(fileName, &recordCount);

    if (sortedRecords == NULL && recordCount == 0) {
        // Error messages are printed inside the sort function
    } else if (recordCount == 0) {
        printf("No records found or file is empty.\n");
    } else {
        for (size_t i = 0; i < recordCount; i++) {
            printf("%s:%s\n", sortedRecords[i].key, sortedRecords[i].value);
        }
    }
    
    freeRecords(sortedRecords, recordCount);
    remove(fileName);
    printf("\n");
}

int main() {
    runTestCase(1, "test1.txt", "banana:fruit\napple:fruit\n carrot : vegetable ");
    runTestCase(2, "test2.txt", "zeta:26\nalpha:1\nbeta:2");
    runTestCase(3, "test3.txt", "");
    runTestCase(4, "test4.txt", "10:ten\n2:two\n1:one");
    runTestCase(5, "test5.txt", "apple:red\nApple:green\napple:yellow");
    return 0;
}