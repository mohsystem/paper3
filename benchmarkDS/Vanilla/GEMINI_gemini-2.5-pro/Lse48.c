#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the entire content of a file in binary mode.
 *
 * @param fileName The path to the file.
 * @param fileSize A pointer to a long to store the size of the file.
 * @return A dynamically allocated buffer with the file content, or NULL on failure.
 *         The caller is responsible for freeing the returned buffer.
 */
unsigned char* img(const char* fileName, long* fileSize) {
    FILE* file = fopen(fileName, "rb"); // "rb" for read binary
    if (file == NULL) {
        *fileSize = 0;
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file); // Go back to the beginning

    if (*fileSize <= 0) { // Handle empty file or error
        fclose(file);
        *fileSize = 0;
        // For empty file, we can return a non-null, zero-sized pointer or NULL.
        // Returning NULL is simpler to handle for the caller.
        return NULL;
    }

    // Allocate memory for the buffer
    unsigned char* buffer = (unsigned char*)malloc(*fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        *fileSize = 0;
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, *fileSize, file);
    if (bytesRead != (size_t)*fileSize) {
        fprintf(stderr, "Error: Could not read the entire file.\n");
        free(buffer);
        fclose(file);
        *fileSize = 0;
        return NULL;
    }

    fclose(file);
    return buffer;
}

// Helper function to print buffer content
void print_content(const unsigned char* content, long size) {
    printf("Content (as hex bytes): [");
    for (long i = 0; i < size; ++i) {
        printf("0x%02x", content[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Helper function to create files for testing
void setupTestFiles() {
    FILE* f;

    // Test case 1: A simple text file
    f = fopen("test1.txt", "w");
    if (f) {
        fprintf(f, "Hello");
        fclose(f);
    }

    // Test case 2: A binary file with specific byte values
    f = fopen("test2.bin", "wb");
    if (f) {
        unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
        fwrite(data, 1, sizeof(data), f);
        fclose(f);
    }
    
    // Test case 3: An empty file
    f = fopen("empty.txt", "w");
    if (f) {
        fclose(f);
    }

    // Test case 5: A binary file with a null character in the middle
    f = fopen("test3.bin", "wb");
    if (f) {
        unsigned char data[] = {'A', 0, 'B'};
        fwrite(data, 1, sizeof(data), f);
        fclose(f);
    }
}

// Helper function to clean up test files
void cleanupTestFiles() {
    remove("test1.txt");
    remove("test2.bin");
    remove("empty.txt");
    remove("test3.bin");
}

int main() {
    setupTestFiles();

    const char* testFiles[] = {
        "test1.txt",        // Case 1: Simple text file
        "test2.bin",        // Case 2: Simple binary file
        "empty.txt",        // Case 3: Empty file
        "nonexistent.file", // Case 4: File that does not exist
        "test3.bin"         // Case 5: File with a null byte
    };

    for (int i = 0; i < 5; ++i) {
        const char* file = testFiles[i];
        long size = 0;
        
        printf("--- Test Case %d: %s ---\n", i + 1, file);
        unsigned char* content = img(file, &size);

        if (content != NULL) {
            printf("Success: Read %ld bytes.\n", size);
            print_content(content, size);
            free(content); // IMPORTANT: Free the allocated memory
        } else {
            // Check if file is just empty vs not existing
            FILE* test_existence = fopen(file, "r");
            if (test_existence) {
                 printf("Success: Read 0 bytes (file is empty).\n");
                 fclose(test_existence);
            } else {
                 printf("Failure: File could not be read (or does not exist).\n");
            }
        }
        printf("\n");
    }

    cleanupTestFiles();
    return 0;
}