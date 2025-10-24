
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_RECORDS 1000

typedef struct {
    char key[256];
    char value[256];
} Record;

void trim(char* str) {
    char* start = str;
    char* end;
    
    while (*start == ' ' || *start == '\\t' || *start == '\\r' || *start == '\\n') {
        start++;
    }
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\\t' || *end == '\\r' || *end == '\\n')) {
        end--;
    }
    
    *(end + 1) = '\\0';
    memmove(str, start, strlen(start) + 1);
}

int compareRecords(const void* a, const void* b) {
    return strcmp(((Record*)a)->key, ((Record*)b)->key);
}

int readAndSortFile(const char* filename, Record* records) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int count = 0;
    
    while (fgets(line, sizeof(line), file) && count < MAX_RECORDS) {
        trim(line);
        
        if (strlen(line) > 0) {
            char* delimiter = strchr(line, '=');
            if (delimiter != NULL) {
                *delimiter = '\\0';
                char* key = line;
                char* value = delimiter + 1;
                
                trim(key);
                trim(value);
                
                strncpy(records[count].key, key, sizeof(records[count].key) - 1);
                records[count].key[sizeof(records[count].key) - 1] = '\\0';
                
                strncpy(records[count].value, value, sizeof(records[count].value) - 1);
                records[count].value[sizeof(records[count].value) - 1] = '\\0';
                
                count++;
            }
        }
    }
    
    fclose(file);
    
    // Sort by key
    qsort(records, count, sizeof(Record), compareRecords);
    
    return count;
}

void createTestFile(const char* filename, const char* lines[], int numLines) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating file: %s\\n", filename);
        return;
    }
    
    for (int i = 0; i < numLines; i++) {
        fprintf(file, "%s\\n", lines[i]);
    }
    
    fclose(file);
}

void printRecords(Record* records, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s = %s\\n", records[i].key, records[i].value);
    }
}

int main() {
    Record records[MAX_RECORDS];
    
    // Test case 1: Basic key-value pairs
    const char* test1[] = {"name=John", "age=30", "city=NewYork"};
    createTestFile("test1.txt", test1, 3);
    printf("Test 1:\\n");
    printRecords(records, readAndSortFile("test1.txt", records));
    
    // Test case 2: Unsorted keys
    const char* test2[] = {"zebra=animal", "apple=fruit", "car=vehicle"};
    createTestFile("test2.txt", test2, 3);
    printf("\\nTest 2:\\n");
    printRecords(records, readAndSortFile("test2.txt", records));
    
    // Test case 3: Keys with spaces
    const char* test3[] = {"first name=Alice", "last name=Smith", "age=25"};
    createTestFile("test3.txt", test3, 3);
    printf("\\nTest 3:\\n");
    printRecords(records, readAndSortFile("test3.txt", records));
    
    // Test case 4: Duplicate keys
    const char* test4[] = {"color=red", "color=blue", "shape=circle"};
    createTestFile("test4.txt", test4, 3);
    printf("\\nTest 4:\\n");
    printRecords(records, readAndSortFile("test4.txt", records));
    
    // Test case 5: Empty and invalid lines
    const char* test5[] = {"valid=data", "", "invalid_line", "another=valid"};
    createTestFile("test5.txt", test5, 4);
    printf("\\nTest 5:\\n");
    printRecords(records, readAndSortFile("test5.txt", records));
    
    return 0;
}
