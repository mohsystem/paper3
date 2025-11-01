#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>

// Platform-specific directory creation and path separator
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEPARATOR "\\"
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEPARATOR "/"
#endif

// Helper function to copy a file
bool copy_file(const char* src, const char* dest) {
    FILE *source_file, *dest_file;
    char buffer[4096];
    size_t bytes_read;
    bool success = true;

    source_file = fopen(src, "rb");
    if (source_file == NULL) {
        perror("Error opening source file");
        return false;
    }

    dest_file = fopen(dest, "wb");
    if (dest_file == NULL) {
        perror("Error opening destination file");
        fclose(source_file);
        return false;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            perror("Error writing to destination file");
            success = false;
            break;
        }
    }

    if (ferror(source_file)) {
        perror("Error reading from source file");
        success = false;
    }

    fclose(source_file);
    fclose(dest_file);
    return success;
}

// Helper function to get the base name of a file path
const char* get_basename(const char* path) {
    const char* basename = strrchr(path, '/');
    #ifdef _WIN32
    const char* basename_win = strrchr(path, '\\');
    if (basename_win > basename) {
        basename = basename_win;
    }
    #endif
    return basename ? basename + 1 : path;
}

/**
 * @brief Simulates saving an uploaded PDF file to an 'uploads' directory.
 * @param sourceFilePath The path to the source PDF file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool savePdf(const char* sourceFilePath) {
    if (sourceFilePath == NULL || sourceFilePath[0] == '\0') {
        fprintf(stderr, "Error: Source file path is empty.\n");
        return false;
    }

    // 1. Check if source file exists and is a regular file
    struct stat st_source;
    if (stat(sourceFilePath, &st_source) != 0 || !S_ISREG(st_source.st_mode)) {
        fprintf(stderr, "Error: Source file does not exist or is not a regular file: %s\n", sourceFilePath);
        return false;
    }
    
    // 2. Check if the file is a PDF (case-insensitive)
    const char* extension = strrchr(sourceFilePath, '.');
    if (extension == NULL || strlen(extension) != 4) {
        fprintf(stderr, "Error: File is not a PDF: %s\n", sourceFilePath);
        return false;
    }
    char lower_ext[5];
    for(int i = 0; i < 5; ++i) {
        lower_ext[i] = tolower((unsigned char)extension[i]);
    }
    if (strcmp(lower_ext, ".pdf") != 0) {
        fprintf(stderr, "Error: File is not a PDF: %s\n", sourceFilePath);
        return false;
    }

    // 3. Create the 'uploads' directory if it doesn't exist
    const char* uploadDirName = "uploads";
    struct stat st_dir = {0};
    if (stat(uploadDirName, &st_dir) == -1) {
        if (MKDIR(uploadDirName) != 0) {
            perror("Error: Could not create directory 'uploads'");
            return false;
        }
    }

    // 4. Construct destination path and copy the file
    const char* baseName = get_basename(sourceFilePath);
    char destinationPath[512];
    snprintf(destinationPath, sizeof(destinationPath), "%s%s%s", uploadDirName, PATH_SEPARATOR, baseName);

    if (!copy_file(sourceFilePath, destinationPath)) {
        fprintf(stderr, "Error: Could not save file '%s'.\n", sourceFilePath);
        return false;
    }
    
    printf("Successfully saved '%s' to '%s'\n", sourceFilePath, destinationPath);
    return true;
}

// Helper function to create dummy files for testing
void createDummyFile(const char* fileName, const char* content) {
    FILE* fp = fopen(fileName, "w");
    if (fp != NULL) {
        fprintf(fp, "%s", content);
        fclose(fp);
    } else {
        fprintf(stderr, "Failed to create dummy file: %s\n", fileName);
    }
}

int main() {
    // Setup test environment by creating some dummy files
    printf("--- Setting up test environment ---\n");
    createDummyFile("test1.pdf", "This is the content of PDF 1.");
    createDummyFile("test2.pdf", "This is the content of PDF 2.");
    createDummyFile("test3.txt", "This is a text file, not a PDF.");
    createDummyFile("test4.pdf", "This is PDF 4, used to test when the uploads dir already exists.");

    const char* testCases[] = {
        "test1.pdf",          // Case 1: Valid PDF file
        "test2.pdf",          // Case 2: Another valid PDF file
        "test3.txt",          // Case 3: Invalid file type (not a PDF)
        "non_existent.pdf",   // Case 4: File that does not exist
        "test4.pdf"           // Case 5: Valid PDF, where the 'uploads' directory already exists
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("\n--- Running Test Cases ---\n");
    for (int i = 0; i < numTestCases; ++i) {
        printf("\nTest Case %d: Processing '%s'\n", i + 1, testCases[i]);
        bool success = savePdf(testCases[i]);
        printf("Result: %s\n", success ? "SUCCESS" : "FAILURE");
    }

    return 0;
}