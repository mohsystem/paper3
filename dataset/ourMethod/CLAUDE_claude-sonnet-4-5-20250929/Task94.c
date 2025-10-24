
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Maximum file size to prevent excessive memory usage (10MB) */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
/* Maximum number of records to prevent DoS */
#define MAX_RECORDS 100000
/* Maximum key/value length */
#define MAX_KV_LENGTH 1024
/* Maximum line length */
#define MAX_LINE_LENGTH (MAX_KV_LENGTH * 2 + 2)
/* Maximum path length */
#define MAX_PATH_LENGTH 4096

typedef struct {
    char key[MAX_KV_LENGTH + 1];   /* +1 for null terminator */
    char value[MAX_KV_LENGTH + 1]; /* +1 for null terminator */
} KeyValue;

/* Securely clear sensitive data from memory */
void secureClear(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Validates that path doesn't contain directory traversal attempts */\nbool validatePath(const char* path) {\n    if (path == NULL) return false;\n    \n    size_t len = strnlen(path, MAX_PATH_LENGTH + 1);\n    if (len == 0 || len > MAX_PATH_LENGTH) {\n        return false;\n    }\n    \n    /* Reject paths with directory traversal patterns */\n    if (strstr(path, "..") != NULL) {\n        return false;\n    }\n    \n    /* Reject absolute paths */\n    if (path[0] == '/' || path[0] == '\\\\') {\n        return false;\n    }\n    \n    return true;\n}\n\n/* Trim whitespace from string in place */\nvoid trimWhitespace(char* str) {\n    if (str == NULL) return;\n    \n    /* Trim leading whitespace */\n    char* start = str;\n    while (*start && isspace((unsigned char)*start)) {\n        start++;\n    }\n    \n    /* If string is all whitespace */\n    if (*start == '\\0') {\n        str[0] = '\\0';\n        return;\n    }\n    \n    /* Trim trailing whitespace */\n    char* end = start + strlen(start) - 1;\n    while (end > start && isspace((unsigned char)*end)) {\n        end--;\n    }\n    *(end + 1) = '\\0';\n    \n    /* Move trimmed string to beginning */\n    if (start != str) {\n        size_t len = strlen(start);\n        memmove(str, start, len + 1); /* +1 for null terminator */\n    }\n}\n\n/* Safely parse a line into key-value pair */\nbool parseLine(const char* line, KeyValue* kv) {\n    if (line == NULL || kv == NULL) return false;\n    \n    /* Initialize output structure */\n    memset(kv, 0, sizeof(KeyValue));\n    \n    size_t lineLen = strnlen(line, MAX_LINE_LENGTH);\n    if (lineLen == 0 || lineLen >= MAX_LINE_LENGTH) {\n        return false;\n    }\n    \n    /* Find delimiter */\n    const char* delim = strchr(line, '=');\n    if (delim == NULL) {\n        delim = strchr(line, ':');\n        if (delim == NULL) {\n            return false;\n        }\n    }\n    \n    /* Calculate key and value lengths with bounds checking */\n    size_t keyLen = delim - line;\n    size_t valueLen = lineLen - keyLen - 1;\n    \n    if (keyLen == 0 || keyLen > MAX_KV_LENGTH || \n        valueLen == 0 || valueLen > MAX_KV_LENGTH) {\n        return false;\n    }\n    \n    /* Copy key with bounds checking */\n    if (keyLen >= sizeof(kv->key)) {\n        return false;\n    }\n    memcpy(kv->key, line, keyLen);\n    kv->key[keyLen] = '\\0'; /* Ensure null termination */\n    \n    /* Copy value with bounds checking */\n    if (valueLen >= sizeof(kv->value)) {\n        return false;\n    }\n    memcpy(kv->value, delim + 1, valueLen);\n    kv->value[valueLen] = '\\0'; /* Ensure null termination */\n    \n    /* Trim whitespace */\n    trimWhitespace(kv->key);\n    trimWhitespace(kv->value);\n    \n    /* Validate after trimming */\n    if (kv->key[0] == '\\0' || kv->value[0] == '\\0') {\n        return false;\n    }\n    \n    return true;\n}\n\n/* Comparison function for qsort */\nint compareKeyValue(const void* a, const void* b) {\n    const KeyValue* kv1 = (const KeyValue*)a;\n    const KeyValue* kv2 = (const KeyValue*)b;\n    return strcmp(kv1->key, kv2->key);\n}\n\n/* Read and sort key-value records from file */\nKeyValue* readAndSortRecords(const char* filename, size_t* outCount) {\n    if (filename == NULL || outCount == NULL) {\n        return NULL;\n    }\n    \n    *outCount = 0;\n    \n    /* Validate file path */\n    if (!validatePath(filename)) {\n        fprintf(stderr, "Error: Invalid file path\\n");\n        return NULL;\n    }\n    \n    /* Open file for reading - use "r" mode with careful validation */\n    FILE* file = fopen(filename, "r");\n    if (file == NULL) {\n        fprintf(stderr, "Error: Cannot open file: %s\\n", filename);\n        return NULL;\n    }\n    \n    /* Check file size to prevent excessive memory usage */\n    if (fseek(file, 0, SEEK_END) != 0) {\n        fprintf(stderr, "Error: Cannot seek file\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    long fileSize = ftell(file);\n    if (fileSize < 0 || (unsigned long)fileSize > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: File too large or invalid\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    if (fseek(file, 0, SEEK_SET) != 0) {\n        fprintf(stderr, "Error: Cannot seek to beginning\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    /* Allocate initial buffer for records */\n    size_t capacity = 100;\n    KeyValue* records = (KeyValue*)calloc(capacity, sizeof(KeyValue));\n    if (records == NULL) {\n        fprintf(stderr, "Error: Memory allocation failed\\n");\n        fclose(file);\n        return NULL;\n    }\n    \n    char line[MAX_LINE_LENGTH];\n    size_t count = 0;\n    \n    /* Read file line by line with bounds checking */\n    while (fgets(line, sizeof(line), file) != NULL && count < MAX_RECORDS) {\n        /* Remove newline if present */\n        size_t len = strnlen(line, sizeof(line));\n        if (len > 0 && line[len - 1] == '\
') {\n            line[len - 1] = '\\0';\n            len--;\n        }\n        if (len > 0 && line[len - 1] == '\\r') {\n            line[len - 1] = '\\0';
        }
        
        /* Grow buffer if needed with overflow check */
        if (count >= capacity) {
            if (capacity > SIZE_MAX / 2 / sizeof(KeyValue)) {
                fprintf(stderr, "Error: Capacity overflow\\n");
                break;
            }
            size_t newCapacity = capacity * 2;
            if (newCapacity > MAX_RECORDS) {
                newCapacity = MAX_RECORDS;
            }
            
            KeyValue* newRecords = (KeyValue*)realloc(records, newCapacity * sizeof(KeyValue));
            if (newRecords == NULL) {
                fprintf(stderr, "Error: Memory reallocation failed\\n");
                break;
            }
            records = newRecords;
            /* Initialize new memory */
            memset(records + capacity, 0, (newCapacity - capacity) * sizeof(KeyValue));
            capacity = newCapacity;
        }
        
        /* Parse and store record */
        if (parseLine(line, &records[count])) {
            count++;
        }
    }
    
    fclose(file);
    
    if (count >= MAX_RECORDS) {
        fprintf(stderr, "Warning: Maximum record count reached\\n");
    }
    
    /* Sort records by key */
    if (count > 0) {
        qsort(records, count, sizeof(KeyValue), compareKeyValue);
    }
    
    *outCount = count;
    return records;
}

int main(void) {
    /* Test case 1: Basic key-value pairs */
    {
        FILE* out = fopen("test1.txt", "w");
        if (out != NULL) {
            fprintf(out, "name=John\\n");
            fprintf(out, "age=30\\n");
            fprintf(out, "city=NYC\\n");
            fclose(out);
        }
        
        size_t count = 0;
        KeyValue* result = readAndSortRecords("test1.txt", &count);
        printf("Test 1 - Basic sorting:\\n");
        if (result != NULL) {
            for (size_t i = 0; i < count; i++) {
                printf("%s=%s\\n", result[i].key, result[i].value);
            }
            secureClear(result, count * sizeof(KeyValue));
            free(result);
        }
        printf("\\n");
    }
    
    /* Test case 2: Colon delimiter */
    {
        FILE* out = fopen("test2.txt", "w");
        if (out != NULL) {
            fprintf(out, "zebra:animal\\n");
            fprintf(out, "apple:fruit\\n");
            fprintf(out, "carrot:vegetable\\n");
            fclose(out);
        }
        
        size_t count = 0;
        KeyValue* result = readAndSortRecords("test2.txt", &count);
        printf("Test 2 - Colon delimiter:\\n");
        if (result != NULL) {
            for (size_t i = 0; i < count; i++) {
                printf("%s:%s\\n", result[i].key, result[i].value);
            }
            secureClear(result, count * sizeof(KeyValue));
            free(result);
        }
        printf("\\n");
    }
    
    /* Test case 3: Whitespace handling */
    {
        FILE* out = fopen("test3.txt", "w");
        if (out != NULL) {
            fprintf(out, "  key1  =  value1  \\n");
            fprintf(out, "key2=value2\\n");
            fprintf(out, "\\n");
            fprintf(out, "key0=value0\\n");
            fclose(out);
        }
        
        size_t count = 0;
        KeyValue* result = readAndSortRecords("test3.txt", &count);
        printf("Test 3 - Whitespace handling:\\n");
        if (result != NULL) {
            for (size_t i = 0; i < count; i++) {
                printf("%s=%s\\n", result[i].key, result[i].value);
            }
            secureClear(result, count * sizeof(KeyValue));
            free(result);
        }
        printf("\\n");
    }
    
    /* Test case 4: Invalid file path */
    {
        printf("Test 4 - Invalid path (../etc/passwd):\\n");
        size_t count = 0;
        KeyValue* result = readAndSortRecords("../etc/passwd", &count);
        printf("Records read: %zu\\n\\n", count);
        if (result != NULL) {
            secureClear(result, count * sizeof(KeyValue));
            free(result);
        }
    }
    
    /* Test case 5: Empty file */
    {
        FILE* out = fopen("test5.txt", "w");
        if (out != NULL) {
            fclose(out);
        }
        
        size_t count = 0;
        KeyValue* result = readAndSortRecords("test5.txt", &count);
        printf("Test 5 - Empty file:\\n");
        printf("Records read: %zu\\n", count);
        if (result != NULL) {
            secureClear(result, count * sizeof(KeyValue));
            free(result);
        }
    }
    
    return 0;
}
