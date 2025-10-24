
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 1000
#define MAX_LINE_LENGTH 256

typedef struct {
    char** records;
    int count;
} RecordList;

char* trim(char* str) {
    char* end;
    while (*str == ' ' || *str == '\\t' || *str == '\\n' || *str == '\\r') str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) end--;
    *(end + 1) = 0;
    return str;
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

RecordList readAndSortFile(const char* filename) {
    RecordList result;
    result.records = (char**)malloc(MAX_RECORDS * sizeof(char*));
    result.count = 0;
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL && result.count < MAX_RECORDS) {
        char* trimmed = trim(line);
        if (strlen(trimmed) > 0) {
            result.records[result.count] = (char*)malloc((strlen(trimmed) + 1) * sizeof(char));
            strcpy(result.records[result.count], trimmed);
            result.count++;
        }
    }
    
    fclose(file);
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
    // Test case 1: Basic key-value pairs
    {
        FILE* file = fopen("test1.txt", "w");
        fprintf(file, "name=John\\n");
        fprintf(file, "age=30\\n");
        fprintf(file, "city=Boston\\n");
        fclose(file);
        
        printf("Test 1:\\n");
        RecordList result = readAndSortFile("test1.txt");
        for (int i = 0; i < result.count; i++) {
            printf("%s\\n", result.records[i]);
        }
        freeRecordList(&result);
    }
    
    // Test case 2: Numeric keys
    {
        FILE* file = fopen("test2.txt", "w");
        fprintf(file, "3=apple\\n");
        fprintf(file, "1=banana\\n");
        fprintf(file, "2=cherry\\n");
        fclose(file);
        
        printf("\\nTest 2:\\n");
        RecordList result = readAndSortFile("test2.txt");
        for (int i = 0; i < result.count; i++) {
            printf("%s\\n", result.records[i]);
        }
        freeRecordList(&result);
    }
    
    // Test case 3: Mixed case
    {
        FILE* file = fopen("test3.txt", "w");
        fprintf(file, "Zebra=animal\\n");
        fprintf(file, "apple=fruit\\n");
        fprintf(file, "Banana=fruit\\n");
        fclose(file);
        
        printf("\\nTest 3:\\n");
        RecordList result = readAndSortFile("test3.txt");
        for (int i = 0; i < result.count; i++) {
            printf("%s\\n", result.records[i]);
        }
        freeRecordList(&result);
    }
    
    // Test case 4: Empty lines
    {
        FILE* file = fopen("test4.txt", "w");
        fprintf(file, "key1=value1\\n");
        fprintf(file, "\\n");
        fprintf(file, "key2=value2\\n");
        fprintf(file, "   \\n");
        fprintf(file, "key3=value3\\n");
        fclose(file);
        
        printf("\\nTest 4:\\n");
        RecordList result = readAndSortFile("test4.txt");
        for (int i = 0; i < result.count; i++) {
            printf("%s\\n", result.records[i]);
        }
        freeRecordList(&result);
    }
    
    // Test case 5: Single record
    {
        FILE* file = fopen("test5.txt", "w");
        fprintf(file, "onlykey=onlyvalue\\n");
        fclose(file);
        
        printf("\\nTest 5:\\n");
        RecordList result = readAndSortFile("test5.txt");
        for (int i = 0; i < result.count; i++) {
            printf("%s\\n", result.records[i]);
        }
        freeRecordList(&result);
    }
    
    return 0;
}
