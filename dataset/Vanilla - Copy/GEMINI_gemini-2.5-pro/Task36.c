#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Reads a file specified by the file path and prints its contents to the console.
 * @param filePath The path to the file to be read.
 */
void readFileAndPrint(const char* filePath) {
    printf("Reading file: %s\n", filePath);
    FILE *file = fopen(filePath, "r");

    if (file == NULL) {
        perror("Error");
        return;
    }

    char buffer[256];
    int empty = 1;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
        empty = 0;
    }

    if(empty){
        printf("(File is empty)\n");
    }

    fclose(file);
}

/**
 * @brief Helper function to create a test file with specified content.
 */
void createTestFile(const char* fileName, const char* content) {
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Failed to create test file");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
}

int main() {
    // --- Test Case 1: Read a simple, single-line file ---
    printf("--- Test Case 1: Simple File ---\n");
    const char* file1 = "c_test1.txt";
    createTestFile(file1, "Hello, World from C!");
    readFileAndPrint(file1);
    printf("-------------------------------------\n\n");

    // --- Test Case 2: Read a multi-line file ---
    printf("--- Test Case 2: Multi-line File ---\n");
    const char* file2 = "c_test2.txt";
    createTestFile(file2, "This is line one.\nThis is line two.\nAnd this is the third line.");
    readFileAndPrint(file2);
    printf("-------------------------------------\n\n");

    // --- Test Case 3: Read an empty file ---
    printf("--- Test Case 3: Empty File ---\n");
    const char* file3 = "c_test3_empty.txt";
    createTestFile(file3, "");
    readFileAndPrint(file3);
    printf("-------------------------------------\n\n");

    // --- Test Case 4: Attempt to read a non-existent file ---
    printf("--- Test Case 4: Non-existent File ---\n");
    readFileAndPrint("nonexistentfile.txt");
    printf("-------------------------------------\n\n");

    // --- Test Case 5: Read a file with special characters ---
    printf("--- Test Case 5: Special Characters File ---\n");
    const char* file4 = "c_test4_special.txt";
    // Note: Console support for UTF-8 can be platform-dependent.
    createTestFile(file4, "Special chars: ñ, é, ö, ü, ç");
    readFileAndPrint(file4);
    printf("-------------------------------------\n\n");

    // Cleanup test files
    remove(file1);
    remove(file2);
    remove(file3);
    remove(file4);

    return 0;
}