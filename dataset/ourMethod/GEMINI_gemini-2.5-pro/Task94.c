#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Struct to hold a key-value record
typedef struct {
    char* key;
    char* value;
} Record;

// Comparison function for qsort
static int compareRecords(const void* a, const void* b) {
    const Record* recA = (const Record*)a;
    const Record* recB = (const Record*)b;
    return strcmp(recA->key, recB->key);
}

// Function to free the memory allocated for records
void freeRecords(Record* records, size_t count) {
    if (records == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(records[i].key);
        free(records[i].value);
    }
    free(records);
}

// Helper to duplicate a string safely
static char* safe_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char* new_str = (char*)malloc(len + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memcpy(new_str, s, len + 1);
    return new_str;
}

// Helper to trim whitespace from a string
static char* trim_whitespace(char* str) {
    if (str == NULL) return NULL;
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}


/**
 * Reads records from a file, sorts them, and returns them via out-parameters.
 *
 * @param filename Path to the file. Must be a relative path.
 * @param out_records Pointer to be filled with the array of records. Caller must free this memory.
 * @param out_count Pointer to be filled with the number of records.
 * @return 0 on success, -1 on failure.
 */
int sortRecordsFromFile(const char* filename, Record** out_records, size_t* out_count) {
    // Rule #7: Validate file path input
    if (filename == NULL || strstr(filename, "..") != NULL || filename[0] == '/' || filename[0] == '\\') {
        fprintf(stderr, "Error: Invalid file path.\n");
        return -1;
    }

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    size_t capacity = 10;
    size_t count = 0;
    Record* records = (Record*)malloc(capacity * sizeof(Record));
    if (records == NULL) {
        perror("Failed to allocate memory for records");
        fclose(file);
        return -1;
    }

    // Rule #1 & #2: Use bounded buffer read with fgets
    char line[1024];
    int lineNumber = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;
        char* trimmed_line = trim_whitespace(line);

        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') {
            continue;
        }

        char* separator = strchr(trimmed_line, ':');
        // Rule #4: Validate input format
        if (separator == NULL || separator == trimmed_line || *(separator + 1) == '\0') {
            fprintf(stderr, "Warning: Malformed line %d: %s\n", lineNumber, trimmed_line);
            continue;
        }
        
        *separator = '\0'; // Null-terminate the key part
        char* key_str = trim_whitespace(trimmed_line);
        char* val_str = trim_whitespace(separator + 1);

        if (strlen(key_str) == 0) {
            fprintf(stderr, "Warning: Malformed line (empty key) %d\n", lineNumber);
            continue;
        }
        
        if (count >= capacity) {
            capacity *= 2;
            Record* temp = (Record*)realloc(records, capacity * sizeof(Record));
            if (temp == NULL) {
                perror("Failed to reallocate memory");
                freeRecords(records, count);
                fclose(file);
                return -1;
            }
            records = temp;
        }
        
        records[count].key = safe_strdup(key_str);
        records[count].value = safe_strdup(val_str);
        if (records[count].key == NULL || records[count].value == NULL) {
            perror("Failed to duplicate string");
            freeRecords(records, count + 1); // Free potentially allocated strings too
            fclose(file);
            return -1;
        }
        count++;
    }

    fclose(file);

    qsort(records, count, sizeof(Record), compareRecords);

    *out_records = records;
    *out_count = count;
    
    return 0; // Success
}

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
    const char* testFiles[] = {"test1.txt", "test2.txt", "test3.txt"};

    createTestFile("test1.txt", "c:3\n a: 1 \n b:2\n");
    createTestFile("test2.txt", "");
    createTestFile("test3.txt", "apple:red\n:empty_key\nbanana:\n# a comment\nmalformed line\norange: fruit\n");

    const char* testCases[] = {
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "non_existent_file.txt",
        "../forbidden.txt"
    };

    for (int i = 0; i < 5; ++i) {
        printf("--- Test Case %d: %s ---\n", i + 1, testCases[i]);
        Record* records = NULL;
        size_t count = 0;
        if (sortRecordsFromFile(testCases[i], &records, &count) == 0) {
            if (count == 0) {
                printf("No records found or file was empty/invalid.\n");
            } else {
                for (size_t j = 0; j < count; ++j) {
                    printf("%s:%s\n", records[j].key, records[j].value);
                }
            }
            freeRecords(records, count);
        }
        printf("\n");
    }

    // Cleanup
    for (int i = 0; i < 3; ++i) {
        remove(testFiles[i]);
    }

    return 0;
}