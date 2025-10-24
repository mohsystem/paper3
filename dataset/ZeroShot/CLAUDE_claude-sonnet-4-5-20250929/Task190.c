
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_COLS 100
#define MAX_ROWS 100

typedef struct {
    char** lines;
    int count;
} TransposeResult;

TransposeResult transposeFile(const char* filename) {
    TransposeResult result = {NULL, 0};
    char rows[MAX_ROWS][MAX_COLS][256];
    int numRows = 0;
    int numCols = 0;
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\\n");
        return result;
    }
    
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, file) != NULL && numRows < MAX_ROWS) {
        int col = 0;
        char* token = strtok(line, " \\n");
        while (token != NULL && col < MAX_COLS) {
            strcpy(rows[numRows][col], token);
            token = strtok(NULL, " \\n");
            col++;
        }
        if (numRows == 0) {
            numCols = col;
        }
        numRows++;
    }
    fclose(file);
    
    if (numRows == 0) {
        return result;
    }
    
    result.lines = (char**)malloc(numCols * sizeof(char*));
    result.count = numCols;
    
    for (int col = 0; col < numCols; col++) {
        result.lines[col] = (char*)malloc(MAX_LINE * sizeof(char));
        result.lines[col][0] = '\\0';
        
        for (int row = 0; row < numRows; row++) {
            if (row > 0) {
                strcat(result.lines[col], " ");
            }
            strcat(result.lines[col], rows[row][col]);
        }
    }
    
    return result;
}

void freeTransposeResult(TransposeResult result) {
    if (result.lines != NULL) {
        for (int i = 0; i < result.count; i++) {
            free(result.lines[i]);
        }
        free(result.lines);
    }
}

int main() {
    // Test case 1: Basic example
    FILE* file1 = fopen("test1.txt", "w");
    fprintf(file1, "name age\\n");
    fprintf(file1, "alice 21\\n");
    fprintf(file1, "ryan 30\\n");
    fclose(file1);
    
    printf("Test 1:\\n");
    TransposeResult result1 = transposeFile("test1.txt");
    for (int i = 0; i < result1.count; i++) {
        printf("%s\\n", result1.lines[i]);
    }
    printf("\\n");
    freeTransposeResult(result1);
    
    // Test case 2: Single row
    FILE* file2 = fopen("test2.txt", "w");
    fprintf(file2, "a b c d\\n");
    fclose(file2);
    
    printf("Test 2:\\n");
    TransposeResult result2 = transposeFile("test2.txt");
    for (int i = 0; i < result2.count; i++) {
        printf("%s\\n", result2.lines[i]);
    }
    printf("\\n");
    freeTransposeResult(result2);
    
    // Test case 3: Single column
    FILE* file3 = fopen("test3.txt", "w");
    fprintf(file3, "a\\n");
    fprintf(file3, "b\\n");
    fprintf(file3, "c\\n");
    fclose(file3);
    
    printf("Test 3:\\n");
    TransposeResult result3 = transposeFile("test3.txt");
    for (int i = 0; i < result3.count; i++) {
        printf("%s\\n", result3.lines[i]);
    }
    printf("\\n");
    freeTransposeResult(result3);
    
    // Test case 4: 3x3 matrix
    FILE* file4 = fopen("test4.txt", "w");
    fprintf(file4, "1 2 3\\n");
    fprintf(file4, "4 5 6\\n");
    fprintf(file4, "7 8 9\\n");
    fclose(file4);
    
    printf("Test 4:\\n");
    TransposeResult result4 = transposeFile("test4.txt");
    for (int i = 0; i < result4.count; i++) {
        printf("%s\\n", result4.lines[i]);
    }
    printf("\\n");
    freeTransposeResult(result4);
    
    // Test case 5: Multiple columns
    FILE* file5 = fopen("test5.txt", "w");
    fprintf(file5, "first last age city\\n");
    fprintf(file5, "john doe 25 NYC\\n");
    fprintf(file5, "jane smith 30 LA\\n");
    fclose(file5);
    
    printf("Test 5:\\n");
    TransposeResult result5 = transposeFile("test5.txt");
    for (int i = 0; i < result5.count; i++) {
        printf("%s\\n", result5.lines[i]);
    }
    printf("\\n");
    freeTransposeResult(result5);
    
    return 0;
}
