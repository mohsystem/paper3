#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A structure to hold the parsed CSV data and its dimensions
typedef struct {
    char*** data;
    int num_rows;
    int* cols_in_row;
} CsvData;

/**
 * Parses a CSV string into a CsvData structure.
 * This implementation handles quoted fields containing commas and escaped double quotes.
 * The caller is responsible for freeing the memory using the free_csv_data function.
 *
 * @param csv_content The string content of the CSV file.
 * @return A CsvData struct containing the parsed data.
 */
CsvData parse_csv(const char* csv_content) {
    CsvData result = {NULL, 0, NULL};
    if (csv_content == NULL || *csv_content == '\0') {
        return result;
    }

    const char* p = csv_content;
    char line_buffer[4096]; // Assume max line length
    int line_idx = 0;
    
    while (*p) {
        // Read one line
        char* l_p = line_buffer;
        while (*p && *p != '\n') {
            *l_p++ = *p++;
        }
        *l_p = '\0';
        if (*p == '\n') p++;

        if (strlen(line_buffer) == 0) continue;

        result.num_rows++;
        result.data = (char***)realloc(result.data, result.num_rows * sizeof(char**));
        result.cols_in_row = (int*)realloc(result.cols_in_row, result.num_rows * sizeof(int));
        
        int current_row_idx = result.num_rows - 1;
        result.data[current_row_idx] = NULL;
        result.cols_in_row[current_row_idx] = 0;

        char field_buffer[1024]; // Assume max field length
        int field_idx = 0;
        int col_count = 0;
        int in_quotes = 0;
        
        for (int i = 0; line_buffer[i] != '\0'; i++) {
            char c = line_buffer[i];
            
            if (in_quotes) {
                if (c == '"') {
                    if (line_buffer[i+1] == '"') {
                        field_buffer[field_idx++] = '"';
                        i++; // Skip next quote
                    } else {
                        in_quotes = false;
                    }
                } else {
                    field_buffer[field_idx++] = c;
                }
            } else {
                if (c == '"') {
                    in_quotes = true;
                } else if (c == ',') {
                    field_buffer[field_idx] = '\0';
                    col_count++;
                    result.data[current_row_idx] = (char**)realloc(result.data[current_row_idx], col_count * sizeof(char*));
                    result.data[current_row_idx][col_count - 1] = strdup(field_buffer);
                    field_idx = 0;
                } else {
                    field_buffer[field_idx++] = c;
                }
            }
        }
        
        field_buffer[field_idx] = '\0';
        col_count++;
        result.data[current_row_idx] = (char**)realloc(result.data[current_row_idx], col_count * sizeof(char*));
        result.data[current_row_idx][col_count - 1] = strdup(field_buffer);
        result.cols_in_row[current_row_idx] = col_count;
    }
    
    return result;
}

/**
 * Frees all memory allocated for a CsvData structure.
 *
 * @param csv A CsvData struct to free.
 */
void free_csv_data(CsvData csv) {
    if (csv.data == NULL) return;
    for (int i = 0; i < csv.num_rows; i++) {
        for (int j = 0; j < csv.cols_in_row[i]; j++) {
            free(csv.data[i][j]);
        }
        free(csv.data[i]);
    }
    free(csv.data);
    free(csv.cols_in_row);
}

void print_csv_data(CsvData csv) {
    for (int i = 0; i < csv.num_rows; i++) {
        printf("[");
        for (int j = 0; j < csv.cols_in_row[i]; j++) {
            printf("\"%s\"", csv.data[i][j]);
            if (j < csv.cols_in_row[i] - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

int main() {
    const char* test_cases[] = {
        // Test Case 1: Simple CSV
        "a,b,c\n1,2,3",
        // Test Case 2: Quoted fields with commas
        "\"a,b\",c\n1,\"2,3\"",
        // Test Case 3: Empty fields and empty quoted fields
        "a,,c\n1,\"\",3",
        // Test Case 4: Escaped quotes within a quoted field
        "a,\"b\"\"c\",d\n\"e \"\"f\"\"\",g",
        // Test Case 5: Mixed and complex cases
        "Name,Age,\"Address, City\"\nJohn Doe,30,\"123 Main St, \"\"Anytown\"\"\"\n\"Jane, Smith\",25,\"456 Oak Ave, Somewhere\""
    };

    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_tests; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input:\n%s\n", test_cases[i]);
        CsvData result = parse_csv(test_cases[i]);
        printf("Output:\n");
        print_csv_data(result);
        free_csv_data(result);
        printf("\n");
    }

    return 0;
}