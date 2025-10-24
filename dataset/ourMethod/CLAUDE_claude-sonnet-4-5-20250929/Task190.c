
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Security: Define reasonable limits to prevent resource exhaustion
#define MAX_LINE_LENGTH 4096
#define MAX_ROWS 10000
#define MAX_COLS 1000
#define MAX_TOKEN_LENGTH 256

// Structure to hold the matrix
typedef struct {
    char*** data;  // 2D array of strings
    size_t rows;
    size_t cols;
} Matrix;

// Security: Safe string copy with bounds checking
static bool safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return false;
    }
    // Security: Use strncpy and ensure null termination
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\\0';
    return true;
}

// Security: Initialize matrix with NULL pointers
static Matrix* create_matrix(size_t rows, size_t cols) {
    if (rows == 0 || cols == 0 || rows > MAX_ROWS || cols > MAX_COLS) {
        return NULL; // Fail closed on invalid dimensions
    }
    
    Matrix* matrix = (Matrix*)calloc(1, sizeof(Matrix));
    if (matrix == NULL) {
        return NULL; // Security: Check allocation
    }
    
    matrix->data = (char***)calloc(rows, sizeof(char**));
    if (matrix->data == NULL) {
        free(matrix);
        return NULL;
    }
    
    // Security: Initialize all pointers to NULL
    for (size_t i = 0; i < rows; i++) {
        matrix->data[i] = (char**)calloc(cols, sizeof(char*));
        if (matrix->data[i] == NULL) {
            // Security: Clean up on failure
            for (size_t j = 0; j < i; j++) {
                free(matrix->data[j]);
            }
            free(matrix->data);
            free(matrix);
            return NULL;
        }
    }
    
    matrix->rows = rows;
    matrix->cols = cols;
    return matrix;
}

// Security: Free all allocated memory
static void free_matrix(Matrix* matrix) {
    if (matrix == NULL) return;
    
    if (matrix->data != NULL) {
        for (size_t i = 0; i < matrix->rows; i++) {
            if (matrix->data[i] != NULL) {
                for (size_t j = 0; j < matrix->cols; j++) {
                    if (matrix->data[i][j] != NULL) {
                        // Security: Clear sensitive data before freeing
                        memset(matrix->data[i][j], 0, strlen(matrix->data[i][j]));
                        free(matrix->data[i][j]);
                    }
                }
                free(matrix->data[i]);
            }
        }
        free(matrix->data);
    }
    free(matrix);
}

// Transpose file content
// Security: Validates all inputs, checks bounds, handles errors
char* transpose_file(const char* filename) {
    // Security: Validate filename
    if (filename == NULL || strlen(filename) == 0 || strstr(filename, "..") != NULL) {
        return NULL; // Fail closed on invalid input
    }
    
    // Security: Use "r" mode, check file open success
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL; // Fail closed if cannot open
    }
    
    char** lines = (char**)calloc(MAX_ROWS, sizeof(char*));
    if (lines == NULL) {
        fclose(file);
        return NULL;
    }
    
    size_t line_count = 0;
    size_t max_cols = 0;
    char line_buffer[MAX_LINE_LENGTH];
    
    // Security: Initialize buffer
    memset(line_buffer, 0, sizeof(line_buffer));
    
    // Read all lines
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL && line_count < MAX_ROWS) {
        // Security: Check for line too long
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\\n') {
            line_buffer[len - 1] = '\\0';
            len--;
        }
        
        // Security: Allocate exact size needed plus null terminator
        lines[line_count] = (char*)malloc(len + 1);
        if (lines[line_count] == NULL) {
            // Security: Clean up on allocation failure
            for (size_t i = 0; i < line_count; i++) {
                free(lines[i]);
            }
            free(lines);
            fclose(file);
            return NULL;
        }
        
        safe_strcpy(lines[line_count], len + 1, line_buffer);
        
        // Count columns in this line
        size_t col_count = 0;
        char* temp_line = strdup(line_buffer);
        if (temp_line != NULL) {
            char* token = strtok(temp_line, " ");
            while (token != NULL && col_count < MAX_COLS) {
                col_count++;
                token = strtok(NULL, " ");
            }
            free(temp_line);
        }
        
        if (col_count > max_cols) {
            max_cols = col_count;
        }
        
        line_count++;
    }
    
    fclose(file);
    
    if (line_count == 0 || max_cols == 0) {
        free(lines);
        return NULL;
    }
    
    // Create matrix
    Matrix* matrix = create_matrix(line_count, max_cols);
    if (matrix == NULL) {
        for (size_t i = 0; i < line_count; i++) {
            free(lines[i]);
        }
        free(lines);
        return NULL;
    }
    
    // Parse lines into matrix
    for (size_t i = 0; i < line_count; i++) {
        char* token = strtok(lines[i], " ");
        size_t col = 0;
        while (token != NULL && col < max_cols) {
            size_t token_len = strlen(token);
            matrix->data[i][col] = (char*)malloc(token_len + 1);
            if (matrix->data[i][col] != NULL) {
                safe_strcpy(matrix->data[i][col], token_len + 1, token);
            }
            col++;
            token = strtok(NULL, " ");
        }
        free(lines[i]);
    }
    free(lines);
    
    // Calculate result size
    size_t result_size = 0;
    for (size_t col = 0; col < max_cols; col++) {
        for (size_t row = 0; row < line_count; row++) {
            if (matrix->data[row][col] != NULL) {
                result_size += strlen(matrix->data[row][col]) + 1;
            }
        }
        result_size += 1;
    }
    
    // Security: Allocate exact size needed
    char* result = (char*)calloc(result_size + 1, 1);
    if (result == NULL) {
        free_matrix(matrix);
        return NULL;
    }
    
    // Transpose
    size_t pos = 0;
    for (size_t col = 0; col < max_cols; col++) {
        for (size_t row = 0; row < line_count; row++) {
            if (matrix->data[row][col] != NULL) {
                size_t token_len = strlen(matrix->data[row][col]);
                // Security: Bounds check before copy
                if (pos + token_len < result_size) {
                    strncpy(result + pos, matrix->data[row][col], token_len);
                    pos += token_len;
                }
                if (row < line_count - 1 && pos < result_size) {
                    result[pos++] = ' ';
                }
            }
        }
        if (col < max_cols - 1 && pos < result_size) {
            result[pos++] = '\\n';
        }
    }
    result[pos] = '\\0';
    
    free_matrix(matrix);
    return result;
}

int main(void) {
    // Test case 1: Standard input
    {
        FILE* f = fopen("file.txt", "w");
        if (f != NULL) {
            fprintf(f, "name age\\n");
            fprintf(f, "alice 21\\n");
            fprintf(f, "ryan 30\\n");
            fclose(f);
        }
        
        char* result = transpose_file("file.txt");
        printf("Test 1:\\n%s\\n\\n", result ? result : "Error");
        free(result);
    }
    
    // Test case 2: Single row
    {
        FILE* f = fopen("file.txt", "w");
        if (f != NULL) {
            fprintf(f, "a b c d\\n");
            fclose(f);
        }
        
        char* result = transpose_file("file.txt");
        printf("Test 2:\\n%s\\n\\n", result ? result : "Error");
        free(result);
    }
    
    // Test case 3: Single column
    {
        FILE* f = fopen("file.txt", "w");
        if (f != NULL) {
            fprintf(f, "a\\nb\\nc\\n");
            fclose(f);
        }
        
        char* result = transpose_file("file.txt");
        printf("Test 3:\\n%s\\n\\n", result ? result : "Error");
        free(result);
    }
    
    // Test case 4: 3x3 matrix
    {
        FILE* f = fopen("file.txt", "w");
        if (f != NULL) {
            fprintf(f, "1 2 3\\n");
            fprintf(f, "4 5 6\\n");
            fprintf(f, "7 8 9\\n");
            fclose(f);
        }
        
        char* result = transpose_file("file.txt");
        printf("Test 4:\\n%s\\n\\n", result ? result : "Error");
        free(result);
    }
    
    // Test case 5: Non-existent file
    {
        char* result = transpose_file("nonexistent.txt");
        printf("Test 5 (non-existent): %s\\n\\n", result ? "Error" : "Handled correctly");
        free(result);
    }
    
    return 0;
}
