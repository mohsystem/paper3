
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FIELDS 100
#define MAX_FIELD_LEN 1000
#define MAX_LINES 1000

typedef struct {
    char** fields;
    int field_count;
} CSVRecord;

typedef struct {
    CSVRecord* records;
    int record_count;
} CSVData;

CSVData* parse_csv(const char* csv_content) {
    CSVData* data = (CSVData*)malloc(sizeof(CSVData));
    data->records = (CSVRecord*)malloc(MAX_LINES * sizeof(CSVRecord));
    data->record_count = 0;
    
    if (csv_content == NULL || strlen(csv_content) == 0) {
        return data;
    }
    
    char* content = strdup(csv_content);
    char* line = strtok(content, "\\n");
    
    while (line != NULL) {
        CSVRecord record;
        record.fields = (char**)malloc(MAX_FIELDS * sizeof(char*));
        record.field_count = 0;
        
        char field[MAX_FIELD_LEN];
        int field_pos = 0;
        bool in_quotes = false;
        int i = 0;
        
        while (i <= strlen(line)) {
            char c = line[i];
            
            if (c == '"' && i < strlen(line)) {
                if (in_quotes && i + 1 <= strlen(line) && line[i + 1] == '"') {
                    field[field_pos++] = '"';
                    i++;
                } else {
                    in_quotes = !in_quotes;
                }
            } else if ((c == ',' || c == '\\0') && !in_quotes) {
                field[field_pos] = '\\0';
                record.fields[record.field_count] = strdup(field);
                record.field_count++;
                field_pos = 0;
            } else {
                if (c != '\\0') {
                    field[field_pos++] = c;
                }
            }
            i++;
        }
        
        data->records[data->record_count++] = record;
        line = strtok(NULL, "\\n");
    }
    
    free(content);
    return data;
}

void print_csv(CSVData* data) {
    for (int i = 0; i < data->record_count; i++) {
        printf("[");
        for (int j = 0; j < data->records[i].field_count; j++) {
            printf("\\"%s\\"", data->records[i].fields[j]);
            if (j < data->records[i].field_count - 1) {
                printf(", ");
            }
        }
        printf("]\\n");
    }
}

void free_csv(CSVData* data) {
    for (int i = 0; i < data->record_count; i++) {
        for (int j = 0; j < data->records[i].field_count; j++) {
            free(data->records[i].fields[j]);
        }
        free(data->records[i].fields);
    }
    free(data->records);
    free(data);
}

int main() {
    // Test case 1: Simple CSV
    printf("Test Case 1: Simple CSV\\n");
    const char* csv1 = "Name,Age,City\\nJohn,30,New York\\nAlice,25,London";
    CSVData* result1 = parse_csv(csv1);
    print_csv(result1);
    free_csv(result1);
    printf("\\n");
    
    // Test case 2: CSV with quoted fields
    printf("Test Case 2: CSV with quoted fields\\n");
    const char* csv2 = "Name,Description\\nProduct A,\\"This is a product\\"\\nProduct B,\\"Another product\\"";
    CSVData* result2 = parse_csv(csv2);
    print_csv(result2);
    free_csv(result2);
    printf("\\n");
    
    // Test case 3: CSV with commas in quoted fields
    printf("Test Case 3: CSV with commas in quoted fields\\n");
    const char* csv3 = "Name,Address\\nJohn,\\"123 Main St, Apt 4\\"\\nAlice,\\"456 Oak Ave, Suite 10\\"";
    CSVData* result3 = parse_csv(csv3);
    print_csv(result3);
    free_csv(result3);
    printf("\\n");
    
    // Test case 4: CSV with escaped quotes
    printf("Test Case 4: CSV with escaped quotes\\n");
    const char* csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She replied \\"\\"Hi\\"\\"\\"";
    CSVData* result4 = parse_csv(csv4);
    print_csv(result4);
    free_csv(result4);
    printf("\\n");
    
    // Test case 5: Empty and mixed content
    printf("Test Case 5: Empty and mixed content\\n");
    const char* csv5 = "A,B,C\\n1,,3\\n,5,\\n7,8,9";
    CSVData* result5 = parse_csv(csv5);
    print_csv(result5);
    free_csv(result5);
    
    return 0;
}
