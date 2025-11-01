#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Platform-specific headers and macros for creating a directory
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEPARATOR '\\'
#else
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777) // 0777 is the permission mode
#define PATH_SEPARATOR '/'
#endif

/**
 * Handles the upload of a file by saving it to the 'uploads' folder.
 * The 'uploads' folder is created in the same directory as the source file.
 * @param filename The name of the file to save.
 * @param content The string content to write into the file.
 * @return 0 on success, -1 on failure.
 */
int upload_file(const char* filename, const char* content) {
    const char* upload_folder = "uploads";
    
    // Check if directory exists, create if not
    struct stat st = {0};
    if (stat(upload_folder, &st) == -1) {
        if (MKDIR(upload_folder) != 0 && errno != EEXIST) {
            perror("C Error: Failed to create directory");
            return -1;
        }
    }
    
    // Construct full path: "uploads" + "/" + "filename" + "\0"
    size_t path_len = strlen(upload_folder) + 1 + strlen(filename) + 1;
    char* file_path = (char*)malloc(path_len);
    if (file_path == NULL) {
        perror("C Error: Failed to allocate memory for path");
        return -1;
    }
    snprintf(file_path, path_len, "%s%c%s", upload_folder, PATH_SEPARATOR, filename);
    
    // Save the file
    FILE* fp = fopen(file_path, "w");
    if (fp == NULL) {
        perror("C Error: Failed to open file");
        free(file_path);
        return -1;
    }
    
    if (fputs(content, fp) == EOF) {
        perror("C Error: Failed to write to file");
        fclose(fp);
        free(file_path);
        return -1;
    }
    
    printf("C: Successfully saved file to %s\n", file_path);
    
    fclose(fp);
    free(file_path);
    
    return 0;
}

int main() {
    printf("--- C Test Cases ---\n");
    upload_file("c_test1.txt", "This is a test from C.");
    upload_file("header.h", "#ifndef HEADER_H\n#define HEADER_H\n#endif");
    upload_file("Makefile", "all: \n\t@echo \"Hello\"");
    upload_file("todo.txt", "- Learn C\n- Build a project");
    upload_file("version.info", "1.0.0");
    printf("--------------------\n");
    return 0;
}