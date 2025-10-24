
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_LINE_LENGTH 1024
#define MAX_RECORDS 10000

typedef struct {
    char key[256];
    char value[768];
} KeyValuePair;

int compareKeyValue(const void* a, const void* b) {
    return strcmp(((KeyValuePair*)a)->key, ((KeyValuePair*)b)->key);
}

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && (*start == ' ' || *start == '\\t' || *start == '\\r' || *start == '\\n')) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\\t' || *end == '\\r' || *end == '\\n')) {
        *end = '\\0';
        end--;
    }
}

int readAndSortKeyValueFile(const char* filename, KeyValuePair* records, int maxRecords) {
    int recordCount = 0;
    
    if (filename == NULL || records == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return 0;
    }
    
    // Check file size
    struct stat fileStat;
    if (stat(filename, &fileStat) != 0) {
        fprintf(stderr, "File not found: %s\\n", filename);
        return 0;
    }
    
    if (fileStat.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File size exceeds maximum allowed size\\n");
        return 0;
    }
    
    // Open file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;
    
    while (fgets(line, sizeof(line), file) != NULL && recordCount < maxRecords) {
        lineNumber++;
        trim(line);
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Find separator
        char* separator = strchr(line, '=');
        if (separator == NULL) {
            separator = strchr(line, ':');
        }
        
        if (separator != NULL && separator != line) {
            *separator = '\\0';
            char* key = line;
            char* value = separator + 1;
            
            trim(key);
            trim(value);
            
            // Bounds checking
            if (strlen(key) < sizeof(records[recordCount].key) &&
                strlen(value) < sizeof(records[recordCount].value)) {
                strncpy(records[recordCount].key, key, sizeof(records[recordCount].key) - 1);
                strncpy(records[recordCount].value, value, sizeof(records[recordCount].value) - 1);
                records[recordCount].key[sizeof(records[recordCount].key) - 1] = '\\0';
                records[recordCount].value[sizeof(records[recordCount].value) - 1] = '\\0';
                recordCount++;
            }
        } else {
            fprintf(stderr, "Invalid format at line %d: %s\\n", lineNumber, line);
        }
    }
    
    fclose(file);
    
    // Sort records
    qsort(records, recordCount, sizeof(KeyValuePair), compareKeyValue);
    
    return recordCount;
}

int main() {
    const char* testFile = "test_keyvalue.txt";
    KeyValuePair records[MAX_RECORDS];
    
    // Test case 1: Basic key-value pairs
    {
        FILE* file = fopen(testFile, "w");
        fprintf(file, "name=John\\n");
        fprintf(file, "age=30\\n");
        fprintf(file, "city=NewYork\\n");
        fprintf(file, "country=USA\\n");
        fprintf(file, "email=john@example.com\\n");
        fclose(file);
        
        printf("Test Case 1: Basic key-value pairs\\n");
        int count = readAndSortKeyValueFile(testFile, records, MAX_RECORDS);
        for (int i = 0; i < count; i++) {
            printf("%s = %s\\n", records[i].key, records[i].value);
        }
    }
    
    // Test case 2: Colon separator
    {
        FILE* file = fopen(testFile, "w");
        fprintf(file, "zebra:animal\\n");
        fprintf(file, "apple:fruit\\n");
        fprintf(file, "carrot:vegetable\\n");
        fclose(file);
        
        printf("\\nTest Case 2: Colon separator\\n");
        int count = readAndSortKeyValueFile(testFile, records, MAX_RECORDS);
        for (int i = 0; i < count; i++) {
            printf("%s : %s\\n", records[i].key, records[i].value);
        }
    }
    
    // Test case 3: Mixed with empty lines
    {
        FILE* file = fopen(testFile, "w");
        fprintf(file, "dog=animal\\n");
        fprintf(file, "\\n");
        fprintf(file, "banana=fruit\\n");
        fprintf(file, "apple=fruit\\n");
        fclose(file);
        
        printf("\\nTest Case 3: With empty lines\\n");
        int count = readAndSortKeyValueFile(testFile, records, MAX_RECORDS);
        for (int i = 0; i < count; i++) {
            printf("%s = %s\\n", records[i].key, records[i].value);
        }
    }
    
    // Test case 4: Non-existent file
    {
        printf("\\nTest Case 4: Non-existent file\\n");
        int count = readAndSortKeyValueFile("nonexistent.txt", records, MAX_RECORDS);
        printf("Records found: %d\\n", count);
    }
    
    // Test case 5: Invalid format
    {
        FILE* file = fopen(testFile, "w");
        fprintf(file, "validkey=validvalue\\n");
        fprintf(file, "invalidline\\n");
        fprintf(file, "anotherkey=anothervalue\\n");
        fclose(file);
        
        printf("\\nTest Case 5: Invalid format handling\\n");
        int count = readAndSortKeyValueFile(testFile, records, MAX_RECORDS);
        for (int i = 0; i < count; i++) {
            printf("%s = %s\\n", records[i].key, records[i].value);
        }
    }
    
    // Cleanup
    remove(testFile);
    
    return 0;
}
