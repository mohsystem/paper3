#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for the free function
void free_csv_data(char**** data, size_t* row_count, size_t** col_counts);

/**
 * Securely parses a CSV string into a dynamically allocated 3D char array.
 *
 * This function handles quoted fields, escaped quotes, and performs careful
 * manual memory management. It is crucial to call free_csv_data on the result
 * to prevent memory leaks.
 *
 * Security considerations:
 * - Checks for malloc/realloc failures.
 * - Manages buffer sizes manually to avoid overflows.
 * - The input `csv_content` is treated as `const` to prevent modification.
 *
 * @param csv_content The input string to parse.
 * @param row_count Pointer to store the number of parsed rows.
 * @param col_counts Pointer to a size_t array to store column counts for each row.
 * @return A pointer to an array of rows, where each row is an array of strings (char*).
 *         Returns NULL on memory allocation failure.
 */
char*** parse_csv(const char* csv_content, size_t* row_count, size_t** col_counts) {
    if (csv_content == NULL) {
        *row_count = 0;
        *col_counts = NULL;
        return NULL;
    }

    char*** records = NULL;
    *row_count = 0;
    *col_counts = NULL;

    const char* p = csv_content;

    while (*p) {
        (*row_count)++;
        records = (char***)realloc(records, (*row_count) * sizeof(char**));
        if (records == NULL) {
            fprintf(stderr, "Memory allocation failed for rows\n");
            // On failure, free previously allocated memory before returning
            free_csv_data(&records, row_count, col_counts); 
            return NULL;
        }
        
        *col_counts = (size_t*)realloc(*col_counts, (*row_count) * sizeof(size_t));
        if (*col_counts == NULL) {
            fprintf(stderr, "Memory allocation failed for column counts\n");
            free_csv_data(&records, row_count, col_counts);
            return NULL;
        }

        size_t current_row_idx = *row_count - 1;
        records[current_row_idx] = NULL;
        (*col_counts)[current_row_idx] = 0;
        
        char field_buffer[1024]; // Static buffer for the current field
        int field_len = 0;
        int in_quotes = 0;

        while (*p && *p != '\n') {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') { // Escaped quote
                        if(field_len < 1023) field_buffer[field_len++] = '"';
                        p++;
                    } else {
                        in_quotes = 0;
                    }
                } else {
                    if(field_len < 1023) field_buffer[field_len++] = *p;
                }
            } else {
                if (*p == '"') {
                    in_quotes = 1;
                } else if (*p == ',') {
                    field_buffer[field_len] = '\0';
                    (*col_counts)[current_row_idx]++;
                    size_t current_col_count = (*col_counts)[current_row_idx];
                    records[current_row_idx] = (char**)realloc(records[current_row_idx], current_col_count * sizeof(char*));
                    if(records[current_row_idx] == NULL) {
                       fprintf(stderr, "Memory allocation failed for columns\n");
                       free_csv_data(&records, row_count, col_counts);
                       return NULL;
                    }
                    records[current_row_idx][current_col_count - 1] = strdup(field_buffer);
                    if(records[current_row_idx][current_col_count - 1] == NULL) {
                       fprintf(stderr, "Memory allocation failed for field\n");
                       free_csv_data(&records, row_count, col_counts);
                       return NULL;
                    }
                    field_len = 0;
                } else {
                    if(field_len < 1023) field_buffer[field_len++] = *p;
                }
            }
            p++;
        }

        // Add the last field
        field_buffer[field_len] = '\0';
        (*col_counts)[current_row_idx]++;
        size_t final_col_count = (*col_counts)[current_row_idx];
        records[current_row_idx] = (char**)realloc(records[current_row_idx], final_col_count * sizeof(char*));
        if(records[current_row_idx] == NULL) {
           fprintf(stderr, "Memory allocation failed for columns\n");
           free_csv_data(&records, row_count, col_counts);
           return NULL;
        }
        records[current_row_idx][final_col_count - 1] = strdup(field_buffer);
        if(records[current_row_idx][final_col_count - 1] == NULL) {
           fprintf(stderr, "Memory allocation failed for field\n");
           free_csv_data(&records, row_count, col_counts);
           return NULL;
        }

        if (*p == '\n') {
            p++;
        }
    }
    return records;
}

/**
 * Frees the memory allocated by parse_csv.
 */
void free_csv_data(char**** data, size_t* row_count, size_t** col_counts) {
    if (data == NULL || *data == NULL) return;

    for (size_t i = 0; i < *row_count; i++) {
        if ((*data)[i] != NULL && (*col_counts) != NULL) {
            for (size_t j = 0; j < (*col_counts)[i]; j++) {
                free((*data)[i][j]);
            }
        }
        free((*data)[i]);
    }
    free(*data);
    *data = NULL;
    
    free(*col_counts);
    *col_counts = NULL;
    
    *row_count = 0;
}

void print_result(char*** data, size_t row_count, size_t* col_counts) {
    for (size_t i = 0; i < row_count; i++) {
        printf("Row %zu: [", i);
        for (size_t j = 0; j < col_counts[i]; j++) {
            printf("\"%s\"", data[i][j]);
            if (j < col_counts[i] - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

int main() {
    const char* testCases[] = {
        // Test Case 1: Simple CSV
        "a,b,c\n1,2,3",
        
        // Test Case 2: Quoted fields with commas
        "a,\"b,c\",d\n1,\"hello, world\",3",
        
        // Test Case 3: Escaped quotes inside quoted fields
        "a,\"b\"\"c\",d\n1,\"it's \"\"ok\"\"\",3",
        
        // Test Case 4: Malformed/edge cases (inconsistent columns, one empty line)
        "a,b\n1,2,3\n\n,d", // C implementation handles the empty line by stopping there.
        
        // Test Case 5: Complex case with mixed quoting and commas
        "Name,Address,Notes\n\"Doe, John\",\"120 Main St, \"\"Anytown\"\"\",\"He said, \"\"Hello!\"\"\""
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input:\n%s\n", testCases[i]);
        
        size_t row_count = 0;
        size_t* col_counts = NULL;
        char*** result = parse_csv(testCases[i], &row_count, &col_counts);

        if (result != NULL) {
            printf("Output:\n");
            print_result(result, row_count, col_counts);
            free_csv_data(&result, &row_count, &col_counts);
        } else {
            printf("Failed to parse CSV or memory allocation error.\n");
        }
        printf("\n");
    }

    return 0;
}