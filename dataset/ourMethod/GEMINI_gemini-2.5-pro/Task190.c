#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to free memory. Assumes each row in grid has 'cols' words.
void free_grid(char*** grid, int rows, int cols) {
    if (!grid) return;
    for (int i = 0; i < rows; i++) {
        if (grid[i]) {
            for (int j = 0; j < cols; j++) {
                free(grid[i][j]);
            }
            free(grid[i]);
        }
    }
    free(grid);
}

// POSIX strdup is not in standard C. Provide a compatible implementation.
char* my_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = (char*)malloc(len);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    return new_s;
}

char* transposeFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        const char* err_msg = "Error: File not found.";
        return my_strdup(err_msg);
    }

    char*** grid = NULL;
    int rows = 0;
    int cols = 0;
    char line_buffer[4096];

    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        line_buffer[strcspn(line_buffer, "\n")] = 0;
        char* p_start = line_buffer;
        while (*p_start && (*p_start == ' ' || *p_start == '\t')) p_start++;
        if (*p_start == '\0') continue;

        rows++;
        grid = (char***)realloc(grid, rows * sizeof(char**));
        if (!grid) { fclose(file); return NULL; }

        if (rows == 1) { // First non-empty line
            char* line_copy = my_strdup(p_start);
            if (!line_copy) { free(grid); fclose(file); return NULL; }
            char* token = strtok(line_copy, " ");
            while (token) {
                cols++;
                token = strtok(NULL, " ");
            }
            free(line_copy);
        }

        grid[rows - 1] = (char**)malloc(cols * sizeof(char*));
        if (!grid[rows - 1]) { free_grid(grid, rows - 1, cols); fclose(file); return NULL; }

        char* token = strtok(p_start, " ");
        for (int i = 0; i < cols; i++) {
            grid[rows - 1][i] = my_strdup(token ? token : "");
            if (!grid[rows - 1][i]) { free_grid(grid, rows, i); fclose(file); return NULL; }
            token = strtok(NULL, " ");
        }
    }
    fclose(file);

    if (rows == 0) { return my_strdup(""); }

    size_t buffer_size = 1;
    for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) buffer_size += strlen(grid[i][j]);
    }
    buffer_size += (size_t)(rows > 1 ? rows - 1 : 0) * cols + (cols > 1 ? cols - 1 : 0);

    char* result = (char*)malloc(buffer_size);
    if (!result) { free_grid(grid, rows, cols); return NULL; }
    
    char* p = result;
    for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
            p += sprintf(p, "%s", grid[i][j]);
            if (i < rows - 1) *p++ = ' ';
        }
        if (j < cols - 1) *p++ = '\n';
    }
    *p = '\0';

    free_grid(grid, rows, cols);
    return result;
}

void createTestFile(const char* fileName, const char* content) {
    FILE* f = fopen(fileName, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

void runTestCase(int testNum, const char* content, const char* expected) {
    const char* fileName = "file.txt";
    printf("--- Test Case %d ---\n", testNum);
    
    createTestFile(fileName, content);
    char* result = transposeFile(fileName);
    
    printf("Input:\n%s\n", strlen(content) > 0 ? content : "<empty file>");
    printf("\nOutput:\n%s\n", result ? result : "<NULL>");
    printf("\nExpected:\n%s\n", expected);
    
    if (result && strcmp(result, expected) == 0) {
        printf("Test Passed: true\n");
    } else {
        printf("Test Passed: false\n");
    }
    
    free(result);
    remove(fileName);
    
    printf("--------------------\n");
}

int main() {
    runTestCase(1, "name age\nalice 21\nryan 30", "name alice ryan\nage 21 30");
    runTestCase(2, "a b c\nd e f", "a d\nb e\nc f");
    runTestCase(3, "single_word", "single_word");
    runTestCase(4, "1 2 3 4 5", "1\n2\n3\n4\n5");
    runTestCase(5, "", "");
    
    printf("--- Test Case 6 (File not found) ---\n");
    const char* expected_error = "Error: File not found.";
    char* result = transposeFile("non_existent_file.txt");
    printf("Output:\n%s\n", result ? result : "<NULL>");
    printf("\nExpected:\n%s\n", expected_error);
    if (result && strcmp(result, expected_error) == 0) {
        printf("Test Passed: true\n");
    } else {
        printf("Test Passed: false\n");
    }
    free(result);
    printf("--------------------\n");
    return 0;
}