
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_RECORDS 1000

typedef struct {
    char key[256];
    char value[256];
} Record;

int compare_records(const void* a, const void* b) {
    Record* rec1 = (Record*)a;
    Record* rec2 = (Record*)b;
    return strcmp(rec1->key, rec2->key);
}

void trim(char* str) {
    char* start = str;
    char* end;
    
    while (*start == ' ' || *start == '\\t' || *start == '\\n' || *start == '\\r') {
        start++;
    }
    
    if (*start == 0) {
        str[0] = 0;
        return;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) {
        end--;
    }
    
    *(end + 1) = 0;
    memmove(str, start, strlen(start) + 1);
}

int read_and_sort_file(const char* filename, Record* records) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int count = 0;
    
    while (fgets(line, sizeof(line), file) && count < MAX_RECORDS) {
        trim(line);
        
        if (strlen(line) == 0) continue;
        
        char* equal_sign = strchr(line, '=');
        if (!equal_sign) continue;
        
        size_t key_len = equal_sign - line;
        strncpy(records[count].key, line, key_len);
        records[count].key[key_len] = '\\0';
        trim(records[count].key);
        
        strcpy(records[count].value, equal_sign + 1);
        trim(records[count].value);
        
        count++;
    }
    
    fclose(file);
    
    qsort(records, count, sizeof(Record), compare_records);
    
    return count;
}

int main() {
    Record records[MAX_RECORDS];
    
    // Test case 1
    printf("Test Case 1:\\n");
    FILE* f1 = fopen("test1.txt", "w");
    fprintf(f1, "name=John\\n");
    fprintf(f1, "age=30\\n");
    fprintf(f1, "city=NewYork\\n");
    fclose(f1);
    
    int count1 = read_and_sort_file("test1.txt", records);
    for (int i = 0; i < count1; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    
    // Test case 2
    printf("\\nTest Case 2:\\n");
    FILE* f2 = fopen("test2.txt", "w");
    fprintf(f2, "zebra=animal\\n");
    fprintf(f2, "apple=fruit\\n");
    fprintf(f2, "car=vehicle\\n");
    fclose(f2);
    
    int count2 = read_and_sort_file("test2.txt", records);
    for (int i = 0; i < count2; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    
    // Test case 3
    printf("\\nTest Case 3:\\n");
    FILE* f3 = fopen("test3.txt", "w");
    fprintf(f3, "key3=value3\\n");
    fprintf(f3, "key1=value1\\n");
    fprintf(f3, "key2=value2\\n");
    fclose(f3);
    
    int count3 = read_and_sort_file("test3.txt", records);
    for (int i = 0; i < count3; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    
    // Test case 4
    printf("\\nTest Case 4:\\n");
    FILE* f4 = fopen("test4.txt", "w");
    fclose(f4);
    
    int count4 = read_and_sort_file("test4.txt", records);
    printf("Records found: %d\\n", count4);
    
    // Test case 5
    printf("\\nTest Case 5:\\n");
    FILE* f5 = fopen("test5.txt", "w");
    fprintf(f5, "user.name=admin\\n");
    fprintf(f5, "server.port=8080\\n");
    fprintf(f5, "app.version=1.0\\n");
    fclose(f5);
    
    int count5 = read_and_sort_file("test5.txt", records);
    for (int i = 0; i < count5; i++) {
        printf("%s=%s\\n", records[i].key, records[i].value);
    }
    
    return 0;
}
