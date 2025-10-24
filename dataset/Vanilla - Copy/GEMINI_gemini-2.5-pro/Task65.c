#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0777)
#define RMDIR(path) rmdir(path)
#endif

/**
 * @brief Retrieves a filename, validates it, and returns the file's content.
 * 
 * @param filename The path to the file.
 * @return A dynamically allocated string containing the file content or an error message.
 *         The caller is responsible for freeing this memory using free(). Returns NULL on
 *         memory allocation failure during error message creation.
 */
char* readFileContent(const char* filename) {
    if (filename == NULL || *filename == '\0' || strspn(filename, " \t\n\v\f\r") == strlen(filename)) {
        const char* msg = "Error: Filename cannot be null or empty.";
        char* error_msg = (char*)malloc(strlen(msg) + 1);
        if (error_msg) strcpy(error_msg, msg);
        return error_msg;
    }

    struct stat path_stat;
    if (stat(filename, &path_stat) != 0) {
        const char* prefix = "Error: File does not exist: ";
        size_t len = strlen(prefix) + strlen(filename) + 1;
        char* error_msg = (char*)malloc(len);
        if (error_msg) snprintf(error_msg, len, "%s%s", prefix, filename);
        return error_msg;
    }
    
    if (S_ISDIR(path_stat.st_mode)) {
        const char* prefix = "Error: Path points to a directory: ";
        size_t len = strlen(prefix) + strlen(filename) + 1;
        char* error_msg = (char*)malloc(len);
        if (error_msg) snprintf(error_msg, len, "%s%s", prefix, filename);
        return error_msg;
    }
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        const char* prefix = "Error: Cannot open file (permission denied): ";
        size_t len = strlen(prefix) + strlen(filename) + 1;
        char* error_msg = (char*)malloc(len);
        if (error_msg) snprintf(error_msg, len, "%s%s", prefix, filename);
        return error_msg;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        const char* msg = "Error: Memory allocation failed.";
        char* error_msg = (char*)malloc(strlen(msg) + 1);
        if (error_msg) strcpy(error_msg, msg);
        return error_msg;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fclose(file);
        free(buffer);
        const char* prefix = "Error: Failed to read the entire file: ";
        size_t len = strlen(prefix) + strlen(filename) + 1;
        char* error_msg = (char*)malloc(len);
        if(error_msg) snprintf(error_msg, len, "%s%s", prefix, filename);
        return error_msg;
    }
    
    buffer[file_size] = '\0';
    fclose(file);
    
    return buffer;
}

void createTestFile(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp != NULL) {
        fprintf(fp, "%s", content);
        fclose(fp);
    }
}

void runTests() {
    printf("--- Running 5 Test Cases ---\n");
    char* result;

    // Test Case 1: Valid and existing file
    printf("\n--- Test Case 1: Valid File ---\n");
    const char* validFile = "test_valid.txt";
    createTestFile(validFile, "Hello C!\nThis is a test file.");
    result = readFileContent(validFile);
    printf("Result: \n%s\n", result ? result : "NULL (allocation failed)");
    free(result);
    remove(validFile);

    // Test Case 2: Non-existent file
    printf("\n--- Test Case 2: Non-existent File ---\n");
    result = readFileContent("nonexistent.txt");
    printf("Result: %s\n", result ? result : "NULL (allocation failed)");
    free(result);

    // Test Case 3: Filename is a directory
    printf("\n--- Test Case 3: Path is a Directory ---\n");
    const char* dirName = "test_dir";
    if (MKDIR(dirName) == 0) {
        result = readFileContent(dirName);
        printf("Result: %s\n", result ? result : "NULL (allocation failed)");
        free(result);
        RMDIR(dirName);
    } else {
        printf("Test setup failed: Could not create directory.\n");
    }

    // Test Case 4: Null filename
    printf("\n--- Test Case 4: Null Filename ---\n");
    result = readFileContent(NULL);
    printf("Result: %s\n", result ? result : "NULL (allocation failed)");
    free(result);
    
    // Test Case 5: Empty/Whitespace filename
    printf("\n--- Test Case 5: Empty/Whitespace Filename ---\n");
    result = readFileContent("   ");
    printf("Result: %s\n", result ? result : "NULL (allocation failed)");
    free(result);
    
    printf("\n--- All tests completed ---\n");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("--- Reading from command line argument ---\n");
        const char* filename = argv[1];
        printf("Reading file: %s\n", filename);
        char* content = readFileContent(filename);
        printf("Content:\n%s\n", content ? content : "Failed to read.");
        free(content);
        printf("----------------------------------------\n\n");
    } else {
        printf("Usage: ./your_program_name <filename>\n\n");
    }

    runTests();
    
    return 0;
}