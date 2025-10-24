
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORDS 100
#define MAX_ROWS 100

typedef struct {
    char** data;
    int rows;
    int cols;
} Matrix;

void freeMatrix(Matrix* m) {
    if (m && m->data) {
        for (int i = 0; i < m->rows; i++) {
            if (m->data[i]) {
                free(m->data[i]);
            }
        }
        free(m->data);
        m->data = NULL;
    }
}

Matrix transpose(const char* filename) {
    Matrix result = {NULL, 0, 0};
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\\n", filename);
        return result;
    }
    
    char*** matrix = (char***)calloc(MAX_ROWS, sizeof(char**));
    if (!matrix) {
        fclose(file);
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    int rowCount = 0;
    int maxCols = 0;
    
    while (fgets(line, sizeof(line), file) && rowCount < MAX_ROWS) {
        line[strcspn(line, "\\n")] = 0;
        if (strlen(line) == 0) continue;
        
        matrix[rowCount] = (char**)calloc(MAX_WORDS, sizeof(char*));
        if (!matrix[rowCount]) {
            fclose(file);
            for (int i = 0; i < rowCount; i++) {
                if (matrix[i]) {
                    for (int j = 0; matrix[i][j]; j++) {
                        free(matrix[i][j]);
                    }
                    free(matrix[i]);
                }
            }
            free(matrix);
            return result;
        }
        
        int colCount = 0;
        char* token = strtok(line, " ");
        while (token && colCount < MAX_WORDS) {
            matrix[rowCount][colCount] = (char*)malloc(strlen(token) + 1);
            if (matrix[rowCount][colCount]) {
                strcpy(matrix[rowCount][colCount], token);
                colCount++;
            }
            token = strtok(NULL, " ");
        }
        
        if (colCount > maxCols) maxCols = colCount;
        rowCount++;
    }
    fclose(file);
    
    if (rowCount == 0) {
        free(matrix);
        return result;
    }
    
    result.data = (char**)calloc(maxCols, sizeof(char*));
    if (!result.data) {
        for (int i = 0; i < rowCount; i++) {
            if (matrix[i]) {
                for (int j = 0; matrix[i][j]; j++) {
                    free(matrix[i][j]);
                }
                free(matrix[i]);
            }
        }
        free(matrix);
        return result;
    }
    
    result.rows = maxCols;
    result.cols = rowCount;
    
    for (int j = 0; j < maxCols; j++) {
        result.data[j] = (char*)calloc(MAX_LINE_LENGTH, sizeof(char));
        if (!result.data[j]) continue;
        
        for (int i = 0; i < rowCount; i++) {
            if (matrix[i] && matrix[i][j]) {
                if (strlen(result.data[j]) > 0) {
                    strcat(result.data[j], " ");
                }
                strcat(result.data[j], matrix[i][j]);
            }
        }
    }
    
    for (int i = 0; i < rowCount; i++) {
        if (matrix[i]) {
            for (int j = 0; matrix[i][j]; j++) {
                free(matrix[i][j]);
            }
            free(matrix[i]);
        }
    }
    free(matrix);
    
    return result;
}

int main() {
    // Test case 1: Basic example
    {
        FILE* f = fopen("test1.txt", "w");
        fprintf(f, "name age\\n");
        fprintf(f, "alice 21\\n");
        fprintf(f, "ryan 30\\n");
        fclose(f);
        
        printf("Test 1:\\n");
        Matrix result = transpose("test1.txt");
        for (int i = 0; i < result.rows; i++) {
            printf("%s\\n", result.data[i]);
        }
        freeMatrix(&result);
        printf("\\n");
    }
    
    // Test case 2: Single row
    {
        FILE* f = fopen("test2.txt", "w");
        fprintf(f, "a b c d\\n");
        fclose(f);
        
        printf("Test 2:\\n");
        Matrix result = transpose("test2.txt");
        for (int i = 0; i < result.rows; i++) {
            printf("%s\\n", result.data[i]);
        }
        freeMatrix(&result);
        printf("\\n");
    }
    
    // Test case 3: Single column
    {
        FILE* f = fopen("test3.txt", "w");
        fprintf(f, "a\\n");
        fprintf(f, "b\\n");
        fprintf(f, "c\\n");
        fclose(f);
        
        printf("Test 3:\\n");
        Matrix result = transpose("test3.txt");
        for (int i = 0; i < result.rows; i++) {
            printf("%s\\n", result.data[i]);
        }
        freeMatrix(&result);
        printf("\\n");
    }
    
    // Test case 4: 3x3 matrix
    {
        FILE* f = fopen("test4.txt", "w");
        fprintf(f, "1 2 3\\n");
        fprintf(f, "4 5 6\\n");
        fprintf(f, "7 8 9\\n");
        fclose(f);
        
        printf("Test 4:\\n");
        Matrix result = transpose("test4.txt");
        for (int i = 0; i < result.rows; i++) {
            printf("%s\\n", result.data[i]);
        }
        freeMatrix(&result);
        printf("\\n");
    }
    
    // Test case 5: Empty file
    {
        FILE* f = fopen("test5.txt", "w");
        fclose(f);
        
        printf("Test 5:\\n");
        Matrix result = transpose("test5.txt");
        if (result.rows == 0) {
            printf("Empty result\\n");
        }
        freeMatrix(&result);
        printf("\\n");
    }
    
    return 0;
}
