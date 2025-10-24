
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_LINE_LENGTH 1024
#define MAX_RECORDS 10000

typedef struct {
    char key[256];
    char value[512];
} KeyValuePair;

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && (*start == ' ' || *start == '\\t' || *start == '\\n' || *start == '\\r')) {
        start++;
    }
    
    if (*start == 0) {
        str[0] = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) {
        end--;
    }
    *(end + 1) = 0;
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

long getFileSize(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

int compareKeyValuePairs(const void* a, const void* b) {
    KeyValuePair* pairA = (KeyValuePair*)a;
    KeyValuePair* pairB = (KeyValuePair*)b;
    return strcmp(pairA->key, pairB->key);
}

int readAndSortKeyValueFile(const char* filePath, KeyValuePair* records, int maxRecords) {
    int recordCount = 0;
    
    if (filePath == NULL || strlen(filePath) == 0) {
        fprintf(stderr, "Invalid file path\\n");
        return 0;
    }
    
    long fileSize = getFileSize(filePath);
    if (fileSize < 0) {
        fprintf(stderr, "Error: Cannot access file - %s\\n", filePath);
        return 0;
    }
    
    if (fileSize > MAX_FILE_SIZE) {
        fprintf(stderr, "File size exceeds maximum allowed size\\n");
        return 0;
    }
    
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file - %s\\n", filePath);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;
    
    while (fgets(line, sizeof(line), file) != NULL && recordCount < maxRecords) {
        lineNumber++;
        trim(line);
        
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        char* equalSign = strchr(line, '=');
        if (equalSign != NULL) {
            *equalSign = '\\0';
            char* key = line;
            char* value = equalSign + 1;
            
            trim(key);
            trim(value);
            
            strncpy(records[recordCount].key, key, sizeof(records[recordCount].key) - 1);
            records[recordCount].key[sizeof(records[recordCount].key) - 1] = '\\0';
            
            strncpy(records[recordCount].value, value, sizeof(records[recordCount].value) - 1);
            records[recordCount].value[sizeof(records[recordCount].value) - 1] = '\\0';
            
            recordCount++;
        } else {
            fprintf(stderr, "Malformed line %d: %s\\n", lineNumber, line);
        }
    }
    
    fclose(file);
    
    qsort(records, recordCount, sizeof(KeyValuePair), compareKeyValuePairs);
    
    return recordCount;
}

void writeTestFile(const char* filename, const char* lines[], int lineCount) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        for (int i = 0; i < lineCount; i++) {
            fprintf(file, "%s\\n", lines[i]);
        }
        fclose(file);
    }
}

int main() {
    KeyValuePair records[MAX_RECORDS];
    
    // Test case 1: Create and read a valid file
    printf("Test 1 - Valid file:\\n");
    const char* test1Lines[] = {
        "name=John",
        "age=30",
        "city=NewYork",
        "country=USA"
    };
    writeTestFile("test1.txt", test1Lines, 4);
    
    int count1 = readAndSortKeyValueFile("test1.txt", records, MAX_RECORDS);
    for (int i = 0; i < count1; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    remove("test1.txt");
    printf("\\n");
    
    // Test case 2: File with comments and empty lines
    printf("Test 2 - File with comments:\\n");
    const char* test2Lines[] = {
        "# This is a comment",
        "zebra=animal",
        "",
        "apple=fruit",
        "banana=fruit"
    };
    writeTestFile("test2.txt", test2Lines, 5);
    
    int count2 = readAndSortKeyValueFile("test2.txt", records, MAX_RECORDS);
    for (int i = 0; i < count2; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    remove("test2.txt");
    printf("\\n");
    
    // Test case 3: File with malformed lines
    printf("Test 3 - Malformed lines:\\n");
    const char* test3Lines[] = {
        "valid=line",
        "invalid line without equals",
        "another=valid"
    };
    writeTestFile("test3.txt", test3Lines, 3);
    
    int count3 = readAndSortKeyValueFile("test3.txt", records, MAX_RECORDS);
    for (int i = 0; i < count3; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    remove("test3.txt");
    printf("\\n");
    
    // Test case 4: Non-existent file
    printf("Test 4 - Non-existent file:\\n");
    int count4 = readAndSortKeyValueFile("nonexistent.txt", records, MAX_RECORDS);
    printf("Records found: %d\\n", count4);
    printf("\\n");
    
    // Test case 5: Empty file path
    printf("Test 5 - Empty file path:\\n");
    int count5 = readAndSortKeyValueFile("", records, MAX_RECORDS);
    printf("Records found: %d\\n", count5);
    
    return 0;
}
