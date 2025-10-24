
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FIELDS 100
#define MAX_FIELD_LENGTH 1000
#define MAX_LINES 1000

typedef struct {
    char** fields;
    int field_count;
} CSVRecord;

typedef struct {
    CSVRecord* records;
    int record_count;
} CSVData;

char** parseLine(const char* line, int* field_count) {
    char** fields = (char**)malloc(MAX_FIELDS * sizeof(char*));
    char* field = (char*)malloc(MAX_FIELD_LENGTH * sizeof(char));
    bool inQuotes = false;
    int fieldIndex = 0;
    int charIndex = 0;
    
    for (int i = 0; i < strlen(line); i++) {
        char c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < strlen(line) && line[i + 1] == '"') {
                field[charIndex++] = '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            field[charIndex] = '\\0';
            fields[fieldIndex] = (char*)malloc((strlen(field) + 1) * sizeof(char));
            strcpy(fields[fieldIndex], field);
            fieldIndex++;
            charIndex = 0;
        } else {
            field[charIndex++] = c;
        }
    }
    
    field[charIndex] = '\\0';
    fields[fieldIndex] = (char*)malloc((strlen(field) + 1) * sizeof(char));
    strcpy(fields[fieldIndex], field);
    fieldIndex++;
    
    free(field);
    *field_count = fieldIndex;
    return fields;
}

CSVData parseCSV(const char* csvContent) {
    CSVData data;
    data.records = (CSVRecord*)malloc(MAX_LINES * sizeof(CSVRecord));
    data.record_count = 0;
    
    if (csvContent == NULL || strlen(csvContent) == 0) {
        return data;
    }
    
    char* content = (char*)malloc((strlen(csvContent) + 1) * sizeof(char));
    strcpy(content, csvContent);
    
    char* line = strtok(content, "\\n");
    while (line != NULL) {
        if (strlen(line) > 0) {
            int field_count;
            char** fields = parseLine(line, &field_count);
            data.records[data.record_count].fields = fields;
            data.records[data.record_count].field_count = field_count;
            data.record_count++;
        }
        line = strtok(NULL, "\\n");
    }
    
    free(content);
    return data;
}

void printCSV(CSVData data) {
    for (int i = 0; i < data.record_count; i++) {
        printf("[");
        for (int j = 0; j < data.records[i].field_count; j++) {
            printf("\\"%s\\"", data.records[i].fields[j]);
            if (j < data.records[i].field_count - 1) {
                printf(", ");
            }
        }
        printf("]\\n");
    }
}

void freeCSVData(CSVData data) {
    for (int i = 0; i < data.record_count; i++) {
        for (int j = 0; j < data.records[i].field_count; j++) {
            free(data.records[i].fields[j]);
        }
        free(data.records[i].fields);
    }
    free(data.records);
}

int main() {
    // Test Case 1: Simple CSV
    const char* csv1 = "Name,Age,City\\nJohn,30,New York\\nJane,25,Los Angeles";
    printf("Test Case 1 - Simple CSV:\\n");
    CSVData result1 = parseCSV(csv1);
    printCSV(result1);
    freeCSVData(result1);
    printf("\\n");
    
    // Test Case 2: CSV with quoted fields
    const char* csv2 = "Name,Description\\n\\"John Doe\\",\\"Software Engineer\\"\\n\\"Jane Smith\\",\\"Data Scientist\\"";
    printf("Test Case 2 - CSV with quoted fields:\\n");
    CSVData result2 = parseCSV(csv2);
    printCSV(result2);
    freeCSVData(result2);
    printf("\\n");
    
    // Test Case 3: CSV with commas in quoted fields
    const char* csv3 = "Product,Price,Description\\nLaptop,1200,\\"High performance, 16GB RAM\\"\\nPhone,800,\\"5G enabled, dual camera\\"";
    printf("Test Case 3 - CSV with commas in quoted fields:\\n");
    CSVData result3 = parseCSV(csv3);
    printCSV(result3);
    freeCSVData(result3);
    printf("\\n");
    
    // Test Case 4: CSV with escaped quotes
    const char* csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She said \\"\\"Goodbye\\"\\"\\"";
    printf("Test Case 4 - CSV with escaped quotes:\\n");
    CSVData result4 = parseCSV(csv4);
    printCSV(result4);
    freeCSVData(result4);
    printf("\\n");
    
    // Test Case 5: Empty and single line CSV
    const char* csv5 = "Header1,Header2,Header3";
    printf("Test Case 5 - Single line CSV:\\n");
    CSVData result5 = parseCSV(csv5);
    printCSV(result5);
    freeCSVData(result5);
    
    return 0;
}
