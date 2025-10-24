
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_RECORDS 1000

typedef struct {
    char** records;
    int count;
} RecordList;

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

RecordList readAndSortFile(const char* filename) {
    RecordList result;
    result.records = (char**)malloc(MAX_RECORDS * sizeof(char*));
    result.count = 0;
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\\n", filename);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL && result.count < MAX_RECORDS) {
        // Remove newline
        line[strcspn(line, "\\r\\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) > 0) {
            result.records[result.count] = (char*)malloc((strlen(line) + 1) * sizeof(char));
            strcpy(result.records[result.count], line);
            result.count++;
        }
    }
    
    fclose(file);
    
    // Sort the records
    qsort(result.records, result.count, sizeof(char*), compare_strings);
    
    return result;
}

void freeRecordList(RecordList* list) {
    for (int i = 0; i < list->count; i++) {
        free(list->records[i]);
    }
    free(list->records);
}

int main() {
    // Test case 1: File with simple key-value pairs
    {
        FILE* file = fopen("test1.txt", "w");
        fprintf(file, "name=John\\n");
        fprintf(file, "age=30\\n");
        fprintf(file, "city=New York\\n");
        fclose(file);
        
        printf("Test 1:\\n");
        RecordList result1 = readAndSortFile("test1.txt");
        for (int i = 0; i < result1.count; i++) {
            printf("%s\\n", result1.records[i]);
        }
        printf("\\n");
        freeRecordList(&result1);
    }
    
    // Test case 2: File with numeric keys
    {
        FILE* file = fopen("test2.txt", "w");
        fprintf(file, "3=three\\n");
        fprintf(file, "1=one\\n");
        fprintf(file, "2=two\\n");
        fclose(file);
        
        printf("Test 2:\\n");
        RecordList result2 = readAndSortFile("test2.txt");
        for (int i = 0; i < result2.count; i++) {
            printf("%s\\n", result2.records[i]);
        }
        printf("\\n");
        freeRecordList(&result2);
    }
    
    // Test case 3: File with mixed keys
    {
        FILE* file = fopen("test3.txt", "w");
        fprintf(file, "zebra=animal\\n");
        fprintf(file, "apple=fruit\\n");
        fprintf(file, "car=vehicle\\n");
        fprintf(file, "banana=fruit\\n");
        fclose(file);
        
        printf("Test 3:\\n");
        RecordList result3 = readAndSortFile("test3.txt");
        for (int i = 0; i < result3.count; i++) {
            printf("%s\\n", result3.records[i]);
        }
        printf("\\n");
        freeRecordList(&result3);
    }
    
    // Test case 4: Empty file
    {
        FILE* file = fopen("test4.txt", "w");
        fclose(file);
        
        printf("Test 4 (empty file):\\n");
        RecordList result4 = readAndSortFile("test4.txt");
        printf("Records count: %d\\n\\n", result4.count);
        freeRecordList(&result4);
    }
    
    // Test case 5: File with duplicate keys
    {
        FILE* file = fopen("test5.txt", "w");
        fprintf(file, "key1=value1\\n");
        fprintf(file, "key3=value3\\n");
        fprintf(file, "key1=value2\\n");
        fprintf(file, "key2=value4\\n");
        fclose(file);
        
        printf("Test 5:\\n");
        RecordList result5 = readAndSortFile("test5.txt");
        for (int i = 0; i < result5.count; i++) {
            printf("%s\\n", result5.records[i]);
        }
        printf("\\n");
        freeRecordList(&result5);
    }
    
    return 0;
}
