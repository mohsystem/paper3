#include <stdio.h>
#include <stdbool.h> // For bool type

/**
 * @brief Opens a file with the given name and writes the provided content to it.
 * The file is created if it does not exist, and overwritten if it does.
 *
 * @param fileName The name of the file to write to.
 * @param content The string content to write into the file.
 * @return true if the write was successful, false otherwise.
 */
bool writeToFile(const char* fileName, const char* content) {
    FILE *file = NULL;

    // Open the file in write mode ("w").
    // This creates the file if it doesn't exist, or truncates it if it does.
    file = fopen(fileName, "w");

    // Security critical: Always check if fopen succeeded.
    // If file is NULL, an error occurred (e.g., permission denied).
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }

    // Write the content to the file.
    if (fprintf(file, "%s", content) < 0) {
        perror("Error writing to file");
        fclose(file); // Attempt to close the file even on write error.
        return false;
    }

    // Security critical: Always close the file handle to prevent resource leaks.
    if (fclose(file) != 0) {
        perror("Error closing file");
        // The write was likely successful, but closing failed.
        // Depending on requirements, you might still want to return true here.
        // For simplicity, we'll indicate failure.
        return false;
    }

    return true;
}

int main() {
    // --- Test Cases ---

    // Test Case 1: The original prompt
    printf("Test Case 1: Writing to 'important_config'\n");
    if (writeToFile("important_config", "important_config")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    printf("--------------------\n");

    // Test Case 2: Writing to a different file with different content
    printf("Test Case 2: Writing to 'test_config_1.txt'\n");
    if (writeToFile("test_config_1.txt", "test data 1")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    printf("--------------------\n");

    // Test Case 3: Another file type
    printf("Test Case 3: Writing to 'another.cfg'\n");
    if (writeToFile("another.cfg", "some other config")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    printf("--------------------\n");

    // Test Case 4: Writing an empty string to a file
    printf("Test Case 4: Writing to 'empty_file.txt'\n");
    if (writeToFile("empty_file.txt", "")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    printf("--------------------\n");

    // Test Case 5: Writing content with special characters
    printf("Test Case 5: Writing to 'special_chars_config.log'\n");
    if (writeToFile("special_chars_config.log", "log data with !@#$%^&*()")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    printf("--------------------\n");

    return 0;
}