#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024

// Forward declaration for helper function
char* strdup_custom(const char* s);

/**
 * Transposes the content of a text file.
 *
 * @param filePath The path to the input text file.
 */
void transposeFile(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // --- First pass: get dimensions ---
    int numRows = 0;
    int numCols = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strspn(line, " \t\n\r") == strlen(line)) continue; // Skip empty/whitespace lines
        numRows++;
        if (numCols == 0) {
            char* line_copy = strdup_custom(line);
            if (!line_copy) { fclose(file); return; }
            char* token = strtok(line_copy, " \t\n\r");
            while (token != NULL) {
                numCols++;
                token = strtok(NULL, " \t\n\r");
            }
            free(line_copy);
        }
    }

    if (numRows == 0 || numCols == 0) {
        fclose(file);
        return;
    }

    // --- Allocate memory for the matrix ---
    char*** matrix = (char***)malloc(numRows * sizeof(char**));
    if (!matrix) { fclose(file); return; }
    for (int i = 0; i < numRows; i++) {
        matrix[i] = (char**)malloc(numCols * sizeof(char*));
        if (!matrix[i]) { /* handle allocation error, free previous */ fclose(file); return; }
    }

    // --- Second pass: read data into matrix ---
    rewind(file);
    int currentRow = 0;
    while (fgets(line, sizeof(line), file) != NULL && currentRow < numRows) {
        if (strspn(line, " \t\n\r") == strlen(line)) continue;
        
        char* token = strtok(line, " \t\n\r");
        int currentCol = 0;
        while (token != NULL && currentCol < numCols) {
            matrix[currentRow][currentCol] = strdup_custom(token);
            token = strtok(NULL, " \t\n\r");
            currentCol++;
        }
        currentRow++;
    }
    fclose(file);

    // --- Transpose and print ---
    for (int j = 0; j < numCols; j++) {
        for (int i = 0; i < numRows; i++) {
            printf("%s%s", matrix[i][j], (i == numRows - 1) ? "" : " ");
        }
        printf("\n");
    }

    // --- Free memory ---
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            free(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);
}

/**
 * Helper function to create a test file with given content.
 */
void createTestFile(const char* fileName, const char* content) {
    FILE* fp = fopen(fileName, "w");
    if (fp != NULL) {
        fputs(content, fp);
        fclose(fp);
    }
}

/**
 * Custom implementation of strdup for portability.
 */
char* strdup_custom(const char* s) {
    size_t len = strlen(s) + 1;
    char* new_s = (char*)malloc(len);
    if (new_s == NULL) {
        return NULL;
    }
    return (char*)memcpy(new_s, s, len);
}

int main() {
    const char* testContents[] = {
        // Test Case 1: Example from prompt
        "name age\nalice 21\nryan 30",
        // Test Case 2: Single row
        "a b c d",
        // Test Case 3: Single column
        "a\nb\nc",
        // Test Case 4: 4x4 matrix
        "1 2 3 4\n5 6 7 8\n9 10 11 12\n13 14 15 16",
        // Test Case 5: 3x3 with strings
        "first middle last\njohn f kennedy\nmartin luther king"
    };
    int numTests = sizeof(testContents) / sizeof(testContents[0]);
    const char* fileName = "file.txt";

    for (int i = 0; i < numTests; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input:\n");
        printf("%s\n", testContents[i]);
        printf("\nOutput:\n");
        
        createTestFile(fileName, testContents[i]);
        transposeFile(fileName);
        printf("\n");
    }

    // Clean up the created file
    remove(fileName);

    return 0;
}