#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 512

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

// Comparison function for qsort
int compareKeyValue(const void *a, const void *b) {
    const KeyValuePair *pairA = (const KeyValuePair *)a;
    const KeyValuePair *pairB = (const KeyValuePair *)b;
    return strcmp(pairA->key, pairB->key);
}

// Function to free the memory allocated for the records
void freeRecords(KeyValuePair *records, int count) {
    if (records == NULL) return;
    for (int i = 0; i < count; i++) {
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// The main logic function
KeyValuePair* sortFileRecords(const char *filename, int *recordCount) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        *recordCount = 0;
        return NULL;
    }

    int capacity = 10;
    int count = 0;
    KeyValuePair *records = malloc(capacity * sizeof(KeyValuePair));
    if (!records) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        *recordCount = 0;
        return NULL;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity *= 2;
            KeyValuePair *temp = realloc(records, capacity * sizeof(KeyValuePair));
            if (!temp) {
                fprintf(stderr, "Memory reallocation failed\n");
                freeRecords(records, count);
                fclose(file);
                *recordCount = 0;
                return NULL;
            }
            records = temp;
        }

        line[strcspn(line, "\r\n")] = 0;
        
        char *delimiter = strchr(line, ':');
        if (delimiter) {
            *delimiter = '\0';
            char *key_str = line;
            char *value_str = delimiter + 1;
            
            // strdup is POSIX but widely available. It allocates memory and copies the string.
            records[count].key = strdup(key_str);
            records[count].value = strdup(value_str);
            
            if (!records[count].key || !records[count].value) {
                 fprintf(stderr, "Memory allocation failed for key/value\n");
                 free(records[count].key); // one of them might have succeeded
                 free(records[count].value);
                 freeRecords(records, count);
                 fclose(file);
                 *recordCount = 0;
                 return NULL;
            }
            count++;
        }
    }
    fclose(file);

    qsort(records, count, sizeof(KeyValuePair), compareKeyValue);
    
    *recordCount = count;
    return records;
}

// Helper to create a file for testing
void createTestFile(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    }
}

void runTestCase(int testNum, const char* content) {
    char filename[50];
    sprintf(filename, "test_file_c_%d.txt", testNum);

    createTestFile(filename, content);

    printf("--- Test Case %d (C) ---\n", testNum);
    printf("Input from %s:\n", filename);
    printf("%s\n", strlen(content) > 0 ? content : "<empty>");

    int recordCount = 0;
    KeyValuePair* sortedRecords = sortFileRecords(filename, &recordCount);

    printf("\nSorted Output:\n");
    if (sortedRecords != NULL && recordCount > 0) {
        for (int i = 0; i < recordCount; i++) {
            printf("%s:%s\n", sortedRecords[i].key, sortedRecords[i].value);
        }
    } else {
        printf("<no records to sort>\n");
    }
    printf("\n");

    freeRecords(sortedRecords, recordCount);
    remove(filename);
}

int main() {
    const char* testContents[] = {
        "banana:fruit\napple:fruit\ncarrot:vegetable",
        "alpha:1\nbeta:2\ngamma:3",
        "zulu:end\nyankee:middle\nxray:start",
        "",
        "one:1"
    };
    
    int numTests = sizeof(testContents) / sizeof(testContents[0]);
    for(int i = 0; i < numTests; i++) {
        runTestCase(i + 1, testContents[i]);
    }

    return 0;
}