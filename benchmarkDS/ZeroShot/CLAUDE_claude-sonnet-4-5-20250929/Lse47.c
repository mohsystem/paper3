
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define IMAGES_FOLDER "images"
#define MAX_PATH_LENGTH 512

int file_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0 && S_ISREG(buffer.st_mode));
}

int is_valid_filename(const char* filename) {
    /* Prevent directory traversal attacks */
    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\\\') != NULL) {
        return 0;
    }
    return 1;
}

int serve_image(const char* filename, FILE* output) {
    char file_path[MAX_PATH_LENGTH];
    FILE* file;
    unsigned char buffer[4096];
    size_t bytes_read;
    
    /* Validate filename */
    if (!is_valid_filename(filename)) {
        fprintf(stderr, "Error: Invalid filename\\n");
        return 0;
    }
    
    /* Build file path */
    snprintf(file_path, MAX_PATH_LENGTH, "%s/%s", IMAGES_FOLDER, filename);
    
    /* Check if file exists */
    if (!file_exists(file_path)) {
        fprintf(stderr, "Error: File not found - %s\\n", file_path);
        return 0;
    }
    
    /* Open file */
    file = fopen(file_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return 0;
    }
    
    /* Send file to output stream */
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytes_read, output);
    }
    
    fclose(file);
    printf("File served successfully: %s\\n", filename);
    return 1;
}

int main() {
    printf("Image Server Test Cases:\\n");
    printf("=========================\\n");
    
    /* Test case 1 */
    printf("\\nTest 1: Serve test.jpg\\n");
    serve_image("test.jpg", stdout);
    
    /* Test case 2 */
    printf("\\nTest 2: Serve logo.png\\n");
    serve_image("logo.png", stdout);
    
    /* Test case 3 */
    printf("\\nTest 3: Serve banner.gif\\n");
    serve_image("banner.gif", stdout);
    
    /* Test case 4 - Invalid path (directory traversal attempt) */
    printf("\\nTest 4: Attempt directory traversal\\n");
    serve_image("../secret.txt", stdout);
    
    /* Test case 5 - Non-existent file */
    printf("\\nTest 5: Request non-existent file\\n");
    serve_image("nonexistent.jpg", stdout);
    
    return 0;
}
