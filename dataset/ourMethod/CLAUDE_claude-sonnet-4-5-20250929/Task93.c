
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 4096
#define MAX_RECORDS 100000
#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 1024

/* Structure to hold key-value pairs */
typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

/* Trim whitespace from both ends of string */
void trim(char* str) {
    if (str == NULL || *str == '\\0') {
        return;
    }
    
    char* end;
    
    /* Trim leading space */
    while (isspace((unsigned char)*str)) {
        str++;
    }
    
    if (*str == '\\0') {
        return;
    }
    
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    
    /* Null terminate */
    end[1] = '\\0';
    
    /* Move trimmed string to beginning if needed */
    if (str != NULL) {
        size_t len = strlen(str);
        memmove(str - (str - (char*)str), str, len + 1);
    }
}

/* Validate path to prevent directory traversal */
bool isPathSafe(const char* filepath) {
    if (filepath == NULL || filepath[0] == '\\0') {
        return false;
    }
    
    /* Reject parent directory references */
    if (strstr(filepath, "..") != NULL) {
        return false;
    }
    
    /* Reject absolute paths */
    if (filepath[0] == '/' || (strlen(filepath) > 1 && filepath[1] == ':')) {
        return false;
    }
    
    return true;
}

/* Parse a single line into key-value pair */
bool parseLine(const char* line, KeyValue* kv) {
    if (line == NULL || kv == NULL || line[0] == '\\0' || line[0] == '#') {
        return false;
    }
    
    /* Find delimiter (= or :) */
    const char* delim = strchr(line, '=');
    if (delim == NULL) {
        delim = strchr(line, ':');
    }
    
    if (delim == NULL || delim == line) {
        return false; /* Invalid format */
    }
    
    /* Calculate lengths with bounds checking */
    size_t keyLen = delim - line;
    size_t valueLen = strlen(delim + 1);
    
    if (keyLen >= MAX_KEY_LENGTH || valueLen >= MAX_VALUE_LENGTH) {
        return false; /* Exceeds buffer size */
    }
    
    /* Copy key with bounds checking */
    memset(kv->key, 0, MAX_KEY_LENGTH);
    memset(kv->value, 0, MAX_VALUE_LENGTH);
    
    strncpy(kv->key, line, keyLen);
    kv->key[keyLen] = '\\0';
    
    /* Copy value with bounds checking */
    if (valueLen > 0) {
        strncpy(kv->value, delim + 1, valueLen);
        kv->value[valueLen] = '\\0';
    }
    
    /* Trim whitespace */
    char* keyPtr = kv->key;
    char* valuePtr = kv->value;
    
    /* Manual trim for key */
    while (*keyPtr && isspace((unsigned char)*keyPtr)) keyPtr++;
    char* keyEnd = keyPtr + strlen(keyPtr) - 1;
    while (keyEnd > keyPtr && isspace((unsigned char)*keyEnd)) keyEnd--;
    keyEnd[1] = '\\0';
    if (keyPtr != kv->key) {
        memmove(kv->key, keyPtr, strlen(keyPtr) + 1);
    }
    
    /* Manual trim for value */
    while (*valuePtr && isspace((unsigned char)*valuePtr)) valuePtr++;
    char* valueEnd = valuePtr + strlen(valuePtr) - 1;
    while (valueEnd > valuePtr && isspace((unsigned char)*valueEnd)) valueEnd--;
    valueEnd[1] = '\\0';
    if (valuePtr != kv->value) {
        memmove(kv->value, valuePtr, strlen(valuePtr) + 1);
    }
    
    return kv->key[0] != '\\0';
}

/* Comparison function for qsort */
int compareKeyValue(const void* a, const void* b) {
    const KeyValue* kv1 = (const KeyValue*)a;
    const KeyValue* kv2 = (const KeyValue*)b;
    return strcmp(kv1->key, kv2->key);
}

/* Read and sort key-value pairs from file */
KeyValue* readAndSortFile(const char* filepath, size_t* count) {
    if (filepath == NULL || count == NULL) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return NULL;
    }
    
    *count = 0;
    
    /* Validate path safety */
    if (!isPathSafe(filepath)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return NULL;
    }
    
    /* Open file with read-only mode */
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\\n", filepath);
        return NULL;
    }
    
    /* Allocate initial buffer */
    size_t capacity = 100;
    KeyValue* records = (KeyValue*)calloc(capacity, sizeof(KeyValue));
    if (records == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(file);
        return NULL;
    }
    
    char line[MAX_LINE_LENGTH];
    memset(line, 0, sizeof(line));
    
    /* Read file line by line */
    while (fgets(line, sizeof(line), file) != NULL) {
        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\\n') {
            line[len - 1] = '\\0';
        }
        if (len > 1 && line[len - 2] == '\\r') {
            line[len - 2] = '\\0';
        }
        
        /* Bounds check: prevent excessive memory usage */
        if (*count >= MAX_RECORDS) {
            fprintf(stderr, "Warning: Maximum record limit reached\\n");
            break;
        }
        
        /* Grow buffer if needed */
        if (*count >= capacity) {
            size_t newCapacity = capacity * 2;
            if (newCapacity > MAX_RECORDS) {
                newCapacity = MAX_RECORDS;
            }
            
            KeyValue* temp = (KeyValue*)realloc(records, newCapacity * sizeof(KeyValue));
            if (temp == NULL) {
                fprintf(stderr, "Error: Memory reallocation failed\\n");
                free(records);
                fclose(file);
                return NULL;
            }
            records = temp;
            
            /* Zero out new memory */
            memset(records + capacity, 0, (newCapacity - capacity) * sizeof(KeyValue));
            capacity = newCapacity;
        }
        
        KeyValue kv;
        memset(&kv, 0, sizeof(KeyValue));
        
        if (parseLine(line, &kv)) {
            memcpy(&records[*count], &kv, sizeof(KeyValue));
            (*count)++;
        }
        
        /* Clear line buffer for next iteration */
        memset(line, 0, sizeof(line));
    }
    
    /* Check for read errors */
    if (ferror(file)) {
        fprintf(stderr, "Error: Failed to read file\\n");
        free(records);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    
    /* Sort records by key */
    if (*count > 0) {
        qsort(records, *count, sizeof(KeyValue), compareKeyValue);
    }
    
    return records;
}

int main(void) {
    /* Test case 1: Basic key-value pairs */
    {
        FILE* testFile = fopen("test1.txt", "w");
        if (testFile != NULL) {
            fprintf(testFile, "name=John\\n");
            fprintf(testFile, "age=30\\n");
            fprintf(testFile, "city=NewYork\\n");
            fclose(testFile);
            
            printf("Test 1: Basic key-value pairs\\n");
            size_t count = 0;
            KeyValue* result = readAndSortFile("test1.txt", &count);
            if (result != NULL) {
                for (size_t i = 0; i < count; i++) {
                    printf("%s = %s\\n", result[i].key, result[i].value);
                }
                free(result);
            }
            printf("\\n");
        }
    }
    
    /* Test case 2: Colon delimiter */
    {
        FILE* testFile = fopen("test2.txt", "w");
        if (testFile != NULL) {
            fprintf(testFile, "zebra:animal\\n");
            fprintf(testFile, "apple:fruit\\n");
            fprintf(testFile, "carrot:vegetable\\n");
            fclose(testFile);
            
            printf("Test 2: Colon delimiter\\n");
            size_t count = 0;
            KeyValue* result = readAndSortFile("test2.txt", &count);
            if (result != NULL) {
                for (size_t i = 0; i < count; i++) {
                    printf("%s : %s\\n", result[i].key, result[i].value);
                }
                free(result);
            }
            printf("\\n");
        }
    }
    
    /* Test case 3: Mixed with comments and empty lines */
    {
        FILE* testFile = fopen("test3.txt", "w");
        if (testFile != NULL) {
            fprintf(testFile, "# Configuration file\\n");
            fprintf(testFile, "server=localhost\\n");
            fprintf(testFile, "\\n");
            fprintf(testFile, "port=8080\\n");
            fprintf(testFile, "# End\\n");
            fclose(testFile);
            
            printf("Test 3: With comments and empty lines\\n");
            size_t count = 0;
            KeyValue* result = readAndSortFile("test3.txt", &count);
            if (result != NULL) {
                for (size_t i = 0; i < count; i++) {
                    printf("%s = %s\\n", result[i].key, result[i].value);
                }
                free(result);
            }
            printf("\\n");
        }
    }
    
    /* Test case 4: Whitespace handling */
    {
        FILE* testFile = fopen("test4.txt", "w");
        if (testFile != NULL) {
            fprintf(testFile, "  key1  =  value1  \\n");
            fprintf(testFile, "key2=value2\\n");
            fprintf(testFile, "  key3  :  value3  \\n");
            fclose(testFile);
            
            printf("Test 4: Whitespace handling\\n");
            size_t count = 0;
            KeyValue* result = readAndSortFile("test4.txt", &count);
            if (result != NULL) {
                for (size_t i = 0; i < count; i++) {
                    printf("[%s] = [%s]\\n", result[i].key, result[i].value);
                }
                free(result);
            }
            printf("\\n");
        }
    }
    
    /* Test case 5: Invalid file path (security test) */
    {
        printf("Test 5: Invalid file path\\n");
        size_t count = 0;
        KeyValue* result = readAndSortFile("../etc/passwd", &count);
        printf("Records read: %zu\\n", count);
        if (result != NULL) {
            free(result);
        }
        printf("\\n");
    }
    
    return 0;
}
