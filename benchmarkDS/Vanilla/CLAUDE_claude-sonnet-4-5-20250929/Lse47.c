
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define IMAGES_FOLDER "images"
#define MAX_PATH 256

int checkFileExists(const char* filename) {
    char filePath[MAX_PATH];
    snprintf(filePath, MAX_PATH, "%s/%s", IMAGES_FOLDER, filename);
    
    struct stat buffer;
    return (stat(filePath, &buffer) == 0);
}

unsigned char* serveImage(const char* filename, size_t* fileSize) {
    char filePath[MAX_PATH];
    snprintf(filePath, MAX_PATH, "%s/%s", IMAGES_FOLDER, filename);
    
    *fileSize = 0;
    
    if (!checkFileExists(filename)) {
        return NULL;
    }
    
    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* content = (unsigned char*)malloc(size);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }
    
    size_t bytesRead = fread(content, 1, size, file);
    fclose(file);
    
    if (bytesRead != size) {
        free(content);
        return NULL;
    }
    
    *fileSize = size;
    return content;
}

int main() {
    const char* testFiles[] = {
        "image1.jpg",
        "image2.png",
        "nonexistent.jpg",
        "test.gif",
        "photo.bmp"
    };
    int numTests = 5;
    
    printf("Testing file existence and serving:\\n");
    for (int i = 0; i < numTests; i++) {
        int exists = checkFileExists(testFiles[i]);
        printf("File: %s - Exists: %s\\n", testFiles[i], exists ? "true" : "false");
        
        size_t fileSize;
        unsigned char* content = serveImage(testFiles[i], &fileSize);
        if (content != NULL) {
            printf("  -> File served successfully, size: %zu bytes\\n", fileSize);
            free(content);
        } else {
            printf("  -> File not found or error occurred\\n");
        }
    }
    
    return 0;
}
