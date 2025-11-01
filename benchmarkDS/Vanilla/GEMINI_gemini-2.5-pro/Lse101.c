#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Opens a file and writes the given content to it, overwriting existing content.
 * @param file_name The name of the file to write to.
 * @param content The string content to write to the file.
 * @return 0 on success, -1 on failure.
 */
int write_to_file(const char* file_name, const char* content) {
    FILE *fp;

    // "w" mode: create file for writing; discard previous contents if any.
    fp = fopen(file_name, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fputs(content, fp) == EOF) {
        perror("Error writing to file");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/**
 * Helper function to read a file and verify its content.
 * @param file_name The name of the file to read.
 * @param expected_content The content expected to be in the file.
 */
void verify_file_content(const char* file_name, const char* expected_content) {
    FILE *fp;
    char *buffer = NULL;
    long file_size;

    fp = fopen(file_name, "rb"); // Use binary mode 'rb' for accurate size
    if (fp == NULL) {
        perror("Error opening file for verification");
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    // Allocate memory for the content + null terminator
    buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return;
    }

    // Read file content into buffer
    if (fread(buffer, 1, file_size, fp) != file_size) {
        fprintf(stderr, "Error reading file for verification\n");
        free(buffer);
        fclose(fp);
        return;
    }
    buffer[file_size] = '\0'; // Null-terminate the string

    fclose(fp);

    // Compare
    if (strcmp(buffer, expected_content) == 0) {
        printf("Verification SUCCESS for %s\n", file_name);
    } else {
        printf("Verification FAILED for %s\n", file_name);
        printf("Expected: '%s'\n", expected_content);
        printf("Got: '%s'\n", buffer);
    }

    free(buffer);
}

int main() {
    // Test Case 1
    printf("--- Test Case 1 ---\n");
    const char* file1 = "important_config";
    const char* content1 = "important_config";
    if (write_to_file(file1, content1) == 0) {
        printf("Successfully wrote to %s\n", file1);
        verify_file_content(file1, content1);
    }
    printf("\n");

    // Test Case 2
    printf("--- Test Case 2 ---\n");
    const char* file2 = "test1.txt";
    const char* content2 = "Hello World";
    if (write_to_file(file2, content2) == 0) {
        printf("Successfully wrote to %s\n", file2);
        verify_file_content(file2, content2);
    }
    printf("\n");

    // Test Case 3
    printf("--- Test Case 3 ---\n");
    const char* file3 = "data.log";
    const char* content3 = "Log entry: 2023-10-27";
    if (write_to_file(file3, content3) == 0) {
        printf("Successfully wrote to %s\n", file3);
        verify_file_content(file3, content3);
    }
    printf("\n");

    // Test Case 4
    printf("--- Test Case 4 ---\n");
    const char* file4 = "empty_file.txt";
    const char* content4 = "";
    if (write_to_file(file4, content4) == 0) {
        printf("Successfully wrote to %s\n", file4);
        verify_file_content(file4, content4);
    }
    printf("\n");

    // Test Case 5
    printf("--- Test Case 5 ---\n");
    const char* file5 = "another_config.ini";
    const char* content5 = "[settings]\nuser=admin";
    if (write_to_file(file5, content5) == 0) {
        printf("Successfully wrote to %s\n", file5);
        // On Windows, 'w' translates '\n' to '\r\n'. For verification,
        // we either need a more complex read or expect the translated content.
        // However, standard text mode fopen() for reading often translates back,
        // so direct comparison usually works. Using binary read ('rb') avoids
        // translation issues and is more robust for verification.
        verify_file_content(file5, content5);
    }
    printf("\n");

    return 0;
}