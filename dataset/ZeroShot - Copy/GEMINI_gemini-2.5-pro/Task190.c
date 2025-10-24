#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Secure: Define _GNU_SOURCE for getline on some compilers if not already defined.
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

// Helper to free the 2D grid of strings to prevent memory leaks.
void free_grid(char*** grid, int num_rows) {
    if (!grid) return;
    for (int i = 0; i < num_rows; i++) {
        if (grid[i]) {
            // This assumes grid[i] has a consistent number of columns which is unknown here.
            // The freeing logic must be tied to the allocation logic.
            // For this implementation, we know each grid[i][j] is a separate allocation.
            // A more robust free would take num_cols as a parameter.
            // However, the caller will handle freeing the tokens.
            free(grid[i]);
        }
    }
    free(grid);
}

char* transposeFileContent(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // --- 1. Read all lines into a dynamic array of strings ---
    char** lines = NULL;
    int num_rows = 0;
    char* line_buf = NULL;
    size_t line_buf_size = 0;

    // Secure: getline handles memory allocation for line_buf, preventing buffer overflows.
    while (getline(&line_buf, &line_buf_size, file) != -1) {
        char** new_lines = realloc(lines, (num_rows + 1) * sizeof(char*));
        if (!new_lines) {
            fprintf(stderr, "Memory allocation failed\n");
            // Cleanup on failure
            for(int i = 0; i < num_rows; ++i) free(lines[i]);
            free(lines); free(line_buf); fclose(file);
            return NULL;
        }
        lines = new_lines;
        line_buf[strcspn(line_buf, "\r\n")] = 0; // Remove trailing newline
        lines[num_rows++] = strdup(line_buf); // Secure: strdup allocates new memory for each line
    }
    free(line_buf);
    fclose(file);

    if (num_rows == 0) {
        free(lines);
        char* empty_result = malloc(1);
        if (empty_result) empty_result[0] = '\0';
        return empty_result;
    }

    // --- 2. Tokenize lines into a 2D grid ---
    char* temp_line = strdup(lines[0]);
    if (!temp_line) { /* Error handling */ return NULL; }
    int num_cols = 0;
    for (char *p = strtok(temp_line, " "); p; p = strtok(NULL, " ")) {
        num_cols++;
    }
    free(temp_line);

    if (num_cols == 0 && num_rows > 0) { // Handles file with only newlines
        num_cols = 1; // Treat as one column of empty strings
    }
    
    char*** grid = (char***)malloc(num_rows * sizeof(char**));
    if(!grid) { /* Error handling */ return NULL; }

    for (int i = 0; i < num_rows; i++) {
        grid[i] = (char**)malloc(num_cols * sizeof(char*));
        if (!grid[i]) { /* Error handling */ return NULL; }
        
        char* line_copy = strdup(lines[i]); // Work on a copy
        if (!line_copy) { /* Error handling */ return NULL; }

        char* token = strtok(line_copy, " ");
        for (int j = 0; j < num_cols; j++) {
            grid[i][j] = (token) ? strdup(token) : strdup("");
            if (token) token = strtok(NULL, " ");
        }
        free(line_copy);
    }

    // --- 3. Build the transposed output string ---
    size_t total_len = 1; // For null terminator
    for (int j = 0; j < num_cols; j++) {
        for (int i = 0; i < num_rows; i++) total_len += strlen(grid[i][j]);
        total_len += (num_rows > 0) ? (num_rows - 1) : 0; // Spaces
    }
    total_len += (num_cols > 0) ? (num_cols - 1) : 0; // Newlines

    char* result = (char*)malloc(total_len);
    if (!result) { /* Error handling */ return NULL; }
    result[0] = '\0';

    for (int j = 0; j < num_cols; j++) {
        for (int i = 0; i < num_rows; i++) {
            // Secure: Use strcat_s or snprintf in production, but here we've pre-sized the buffer.
            strcat(result, grid[i][j]);
            if (i < num_rows - 1) strcat(result, " ");
        }
        if (j < num_cols - 1) strcat(result, "\n");
    }

    // --- 4. Cleanup ---
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) free(grid[i][j]);
        free(grid[i]);
    }
    free(grid);
    for (int i = 0; i < num_rows; ++i) free(lines[i]);
    free(lines);

    return result;
}

void run_c_test_case(int test_num, const char* content, const char* expected) {
    const char* filename = "file.txt";
    printf("--- Test Case %d ---\n", test_num);
    FILE* f = fopen(filename, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
    
    char* result = transposeFileContent(filename);
    
    printf("Input:\n%s\n", strlen(content) > 0 ? content : "(empty)");
    printf("\nOutput:\n%s\n", result && strlen(result) > 0 ? result : "(empty)");
    printf("\nExpected:\n%s\n", strlen(expected) > 0 ? expected : "(empty)");

    if (result && expected) {
        printf("\nResult matches expected: %s\n", strcmp(result, expected) == 0 ? "true" : "false");
    } else {
        printf("\nResult matches expected: false\n");
    }
    
    free(result); // Secure: Free the memory returned by the function.
    remove(filename);
    printf("---------------------\n\n");
}

int main() {
    run_c_test_case(1, "name age\nalice 21\nryan 30", "name alice ryan\nage 21 30");
    run_c_test_case(2, "a b c d", "a\nb\nc\nd");
    run_c_test_case(3, "x\ny\nz", "x y z");
    run_c_test_case(4, "1 2 3\n4 5 6\n7 8 9", "1 4 7\n2 5 8\n3 6 9");
    run_c_test_case(5, "", "");
    return 0;
}