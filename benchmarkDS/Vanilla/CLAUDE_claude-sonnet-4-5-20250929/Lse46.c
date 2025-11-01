
// C implementation of web server requires complex socket programming
// Here's a simplified function concept

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data;
    size_t size;
} FileContent;

FileContent img(const char* name) {
    FileContent content = {NULL, 0};
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "images/%s", name);
    
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        return content;
    }
    
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    content.data = (char*)malloc(filesize);
    if (content.data != NULL) {
        content.size = fread(content.data, 1, filesize, file);
    }
    
    fclose(file);
    return content;
}

int main() {
    // Test cases
    const char* testCases[] = {"test1.jpg", "test2.png", "test3.gif", "test4.bmp", "test5.jpg"};
    
    printf("Testing img function with test cases:\\n");
    
    for (int i = 0; i < 5; i++) {
        FileContent result = img(testCases[i]);
        if (result.data != NULL) {
            printf("Successfully read: %s (%zu bytes)\\n", testCases[i], result.size);
            free(result.data);
        } else {
            printf("Failed to read: %s\\n", testCases[i]);
        }
    }
    
    return 0;
}
