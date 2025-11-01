#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/**
 * Reads the content of a file from the /safe/ directory.
 *
 * @param fileName The name of the file to read (not the full path).
 * @return A dynamically allocated string with the file's content.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL on failure (e.g., file not found, is a directory, etc.).
 */
char* readFileFromSafeDir(const char* fileName) {
    // Basic check for path traversal characters.
    if (fileName == NULL || strstr(fileName, "/") != NULL || strstr(fileName, "..") != NULL) {
        fprintf(stderr, "[ERROR] Invalid filename provided.\n");
        return NULL;
    }

    char filePath[256];
    int written = snprintf(filePath, sizeof(filePath), "/safe/%s", fileName);
    if (written >= sizeof(filePath)) {
        fprintf(stderr, "[ERROR] File path is too long.\n");
        return NULL;
    }

    // Check if path is a directory before trying to open as a file
    struct stat path_stat;
    stat(filePath, &path_stat);
    if (S_ISDIR(path_stat.st_mode)) {
        fprintf(stderr, "[ERROR] Path is a directory, not a file: %s\n", filePath);
        return NULL;
    }

    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("[ERROR] fopen");
        return NULL;
    }

    // Seek to the end to find the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize < 0) {
        perror("[ERROR] ftell");
        fclose(file);
        return NULL;
    }
    rewind(file);

    // Allocate memory for the content + null terminator
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        fprintf(stderr, "[ERROR] Could not read the entire file.\n");
        fclose(file);
        free(buffer);
        return NULL;
    }

    buffer[fileSize] = '\0'; // Null-terminate the string
    fclose(file);

    return buffer;
}

void run_test(const char* test_name, const char* filename) {
    printf("Test Case: %s\n", test_name);
    char* content = readFileFromSafeDir(filename);
    if (content != NULL) {
        printf("Content: \"%s\"\n\n", content);
        free(content); // Free the memory allocated by the function
    } else {
        printf("Function returned NULL (as expected for an error).\n\n");
    }
}

int main() {
    // To run these tests, you must first set up the directory and files.
    // On a Unix-like system, you can use these commands:
    // sudo mkdir /safe
    // sudo chown $(whoami) /safe
    // echo "This is a test file." > /safe/existing.txt
    // touch /safe/empty.txt
    // mkdir /safe/test_dir
    //
    // Compile with: gcc -o your_program_name your_source_file.c

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Read an existing file with content.
    run_test("Reading 'existing.txt'", "existing.txt");

    // Test Case 2: Read a non-existent file.
    run_test("Reading 'non_existent.txt'", "non_existent.txt");

    // Test Case 3: Read an empty file.
    run_test("Reading 'empty.txt'", "empty.txt");

    // Test Case 4: Attempt to read a directory.
    run_test("Reading directory 'test_dir'", "test_dir");

    // Test Case 5: Attempt to use an invalid filename (path traversal).
    run_test("Reading invalid filename '../etc/passwd'", "../etc/passwd");
    
    return 0;
}