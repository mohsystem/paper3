
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <dirent.h>
#endif

#define UPLOAD_DIRECTORY "uploads"
#define MAX_PATH_LENGTH 256
#define MAX_CONTENT_LENGTH 10000

int createDirectory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0777) == 0;
    }
    return 1;
}

int uploadFileWithContent(const char* fileName, const char* content) {
    if (!createDirectory(UPLOAD_DIRECTORY)) {
        printf("Error creating upload directory\\n");
        return 0;
    }
    
    char filePath[MAX_PATH_LENGTH];
    snprintf(filePath, MAX_PATH_LENGTH, "%s/%s", UPLOAD_DIRECTORY, fileName);
    
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        printf("Error opening file for writing: %s\\n", fileName);
        return 0;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
    
    printf("File uploaded successfully: %s\\n", fileName);
    return 1;
}

int uploadFile(const char* sourceFilePath, const char* destinationFileName) {
    if (!createDirectory(UPLOAD_DIRECTORY)) {
        printf("Error creating upload directory\\n");
        return 0;
    }
    
    FILE* source = fopen(sourceFilePath, "rb");
    if (source == NULL) {
        printf("Source file does not exist: %s\\n", sourceFilePath);
        return 0;
    }
    
    char destPath[MAX_PATH_LENGTH];
    snprintf(destPath, MAX_PATH_LENGTH, "%s/%s", UPLOAD_DIRECTORY, destinationFileName);
    
    FILE* dest = fopen(destPath, "wb");
    if (dest == NULL) {
        printf("Error creating destination file\\n");
        fclose(source);
        return 0;
    }
    
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytesRead, dest);
    }
    
    fclose(source);
    fclose(dest);
    
    printf("File uploaded successfully: %s\\n", destinationFileName);
    return 1;
}

void listUploadedFiles() {
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    char searchPath[MAX_PATH_LENGTH];
    snprintf(searchPath, MAX_PATH_LENGTH, "%s/*", UPLOAD_DIRECTORY);
    
    HANDLE hFind = FindFirstFile(searchPath, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                printf("  - %s\\n", findData.cFileName);
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(UPLOAD_DIRECTORY);
    if (dir != NULL) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                printf("  - %s\\n", entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
}

int main() {
    printf("=== File Upload Program ===\\n\\n");
    
    // Test Case 1: Upload a file with content
    printf("Test Case 1: Upload text file\\n");
    int result1 = uploadFileWithContent("test1.txt", "Hello, this is test file 1!");
    printf("Result: %s\\n\\n", result1 ? "true" : "false");
    
    // Test Case 2: Upload another file with content
    printf("Test Case 2: Upload document\\n");
    int result2 = uploadFileWithContent("document.txt", "This is a sample document with multiple lines.\\nLine 2\\nLine 3");
    printf("Result: %s\\n\\n", result2 ? "true" : "false");
    
    // Test Case 3: Upload a file with JSON content
    printf("Test Case 3: Upload JSON file\\n");
    int result3 = uploadFileWithContent("data.json", "{\\"name\\":\\"John\\",\\"age\\":30,\\"city\\":\\"New York\\"}");
    printf("Result: %s\\n\\n", result3 ? "true" : "false");
    
    // Test Case 4: Upload a file with special characters
    printf("Test Case 4: Upload file with special characters\\n");
    int result4 = uploadFileWithContent("special.txt", "Special chars: !@#$%^&*()");
    printf("Result: %s\\n\\n", result4 ? "true" : "false");
    
    // Test Case 5: List all uploaded files
    printf("Test Case 5: List all uploaded files\\n");
    printf("Uploaded files:\\n");
    listUploadedFiles();
    
    return 0;
}
