
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_COLS 100
#define MAX_ROWS 100

void transpose(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;
    
    char rows[MAX_ROWS][MAX_COLS][MAX_LINE];
    int rowCount = 0;
    int colCounts[MAX_ROWS] = {0};
    int maxCols = 0;
    char line[MAX_LINE];
    
    while (fgets(line, MAX_LINE, file)) {
        int colCount = 0;
        char* token = strtok(line, " \\n");
        while (token != NULL && colCount < MAX_COLS) {
            strcpy(rows[rowCount][colCount], token);
            colCount++;
            token = strtok(NULL, " \\n");
        }
        colCounts[rowCount] = colCount;
        if (colCount > maxCols) maxCols = colCount;
        rowCount++;
    }
    fclose(file);
    
    for (int col = 0; col < maxCols; col++) {
        int first = 1;
        for (int row = 0; row < rowCount; row++) {
            if (col < colCounts[row]) {
                if (!first) printf(" ");
                printf("%s", rows[row][col]);
                first = 0;
            }
        }
        printf("\\n");
    }
}

int main() {
    // Test case 1
    FILE* f1 = fopen("test1.txt", "w");
    fprintf(f1, "name age\\n");
    fprintf(f1, "alice 21\\n");
    fprintf(f1, "ryan 30\\n");
    fclose(f1);
    printf("Test 1:\\n");
    transpose("test1.txt");
    printf("\\n");
    
    // Test case 2
    FILE* f2 = fopen("test2.txt", "w");
    fprintf(f2, "a b c\\n");
    fprintf(f2, "d e f\\n");
    fprintf(f2, "g h i\\n");
    fclose(f2);
    printf("Test 2:\\n");
    transpose("test2.txt");
    printf("\\n");
    
    // Test case 3
    FILE* f3 = fopen("test3.txt", "w");
    fprintf(f3, "1 2\\n");
    fprintf(f3, "3 4\\n");
    fclose(f3);
    printf("Test 3:\\n");
    transpose("test3.txt");
    printf("\\n");
    
    // Test case 4
    FILE* f4 = fopen("test4.txt", "w");
    fprintf(f4, "x\\n");
    fprintf(f4, "y\\n");
    fprintf(f4, "z\\n");
    fclose(f4);
    printf("Test 4:\\n");
    transpose("test4.txt");
    printf("\\n");
    
    // Test case 5
    FILE* f5 = fopen("test5.txt", "w");
    fprintf(f5, "first second third fourth\\n");
    fprintf(f5, "1 2 3 4\\n");
    fclose(f5);
    printf("Test 5:\\n");
    transpose("test5.txt");
    printf("\\n");
    
    return 0;
}
