#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_MAX_LEN 4096

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

// Comparison function for qsort
static int comparePairs(const void* a, const void* b) {
    const KeyValuePair* pairA = (const KeyValuePair*)a;
    const KeyValuePair* pairB = (const KeyValuePair*)b;
    return strcmp(pairA->key, pairB->key);
}

// Function to trim leading/trailing whitespace from a string (in-place)
static char* trim(char* str) {
    if (str == NULL) return NULL;
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

// Helper to free the memory allocated for records
static void freeRecords(KeyValuePair* records, size_t count) {
    if (records == NULL) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// POSIX `strdup` is not in standard C, so we provide an implementation for portability.
static char* my_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = malloc(len);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    return new_s;
}

KeyValuePair* readFileRecords(const char* filename, size_t* count) {
    *count = 0;
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    size_t capacity = 10;
    KeyValuePair* records = malloc(capacity * sizeof(KeyValuePair));
    if (!records) {
        perror("Failed to allocate memory for records");
        fclose(file);
        return NULL;
    }

    char line[LINE_MAX_LEN];
    while (fgets(line, sizeof(line), file) != NULL) {
        char* separator = strchr(line, ':');
        if (separator == NULL) continue;

        *separator = '\0';
        char* key_trimmed = trim(line);
        char* value_trimmed = trim(separator + 1);
        
        if (strlen(key_trimmed) == 0 || strlen(value_trimmed) == 0) continue;

        if (*count >= capacity) {
            capacity *= 2;
            KeyValuePair* temp = realloc(records, capacity * sizeof(KeyValuePair));
            if (!temp) {
                perror("Failed to reallocate memory");
                freeRecords(records, *count);
                fclose(file);
                return NULL;
            }
            records = temp;
        }

        records[*count].key = my_strdup(key_trimmed);
        records[*count].value = my_strdup(value_trimmed);

        if (records[*count].key == NULL || records[*count].value == NULL) {
             perror("Failed to duplicate string");
             free(records[*count].key);
             free(records[*count].value);
             freeRecords(records, *count);
             fclose(file);
             return NULL;
        }
        (*count)++;
    }

    fclose(file);
    return records;
}

void sortAndPrintRecords(KeyValuePair* records, size_t count) {
    if (records == NULL || count == 0) {
        printf("No records to display.\n");
        return;
    }

    qsort(records, count, sizeof(KeyValuePair), comparePairs);

    for (size_t i = 0; i < count; i++) {
        printf("%s: %s\n", records[i].key, records[i].value);
    }
}

static void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
    } else {
        perror("Failed to create test file");
    }
}

static void runTestCase(int testNum, const char* filename, const char* content) {
    printf("----- Test Case %d: %s -----\n", testNum, filename);
    
    createTestFile(filename, content);

    size_t count = 0;
    KeyValuePair* records = readFileRecords(filename, &count);
    
    sortAndPrintRecords(records, count);

    freeRecords(records, count);
    remove(filename);
    
    printf("\n");
}

int main(void) {
    runTestCase(1, "test1.txt", "c:3\na:1\nb:2");
    runTestCase(2, "test2.txt", "");
    runTestCase(3, "test3.txt", "z:26");
    runTestCase(4, "test4.txt", "b:1\na:2\nb:3");
    runTestCase(5, "test5.txt", "key1:value1\n:onlyvalue\nkeyonly:\nmalformed\nkey2:value2");

    return 0;
}