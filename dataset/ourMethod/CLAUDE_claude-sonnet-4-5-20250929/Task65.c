
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

/* Maximum file size allowed (10 MB) */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
/* Initial buffer size for reading */
#define INITIAL_BUFFER_SIZE 4096

/* Structure to hold file data */
typedef struct {
    char *data;
    size_t size;
} FileData;

/* Portable explicit memory zeroization to prevent compiler optimization */
static void explicit_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *vptr = (volatile unsigned char *)ptr;
    while (len--) {
        *vptr++ = 0;
    }
}

/* Free FileData and zero sensitive content */
void free_file_data(FileData *fd) {
    if (fd == NULL) return;
    if (fd->data != NULL) {
        /* Zero the buffer before freeing to clear any sensitive data */
        explicit_zero(fd->data, fd->size);
        free(fd->data);
        fd->data = NULL;
    }
    fd->size = 0;
}

/* Validate filename format - reject path traversal attempts and null bytes */
int validate_filename_format(const char *filename) {
    if (filename == NULL || filename[0] == '\\0') {
        return 0;
    }
    
    size_t len = strlen(filename);
    
    /* Check maximum filename length */
    if (len > 255) {
        return 0;
    }
    
    /* Check for null bytes within the string */
    if (memchr(filename, '\\0', len) != (filename + len)) {
        return 0;
    }
    
    /* Reject path traversal patterns */
    if (strstr(filename, "..") != NULL) {
        return 0;
    }
    
    /* Reject absolute paths */
    if (filename[0] == '/') {
        return 0;
    }
    
    /* Reject Windows-style paths */
    if (strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    /* Reject directory separators (enforce simple filenames only) */\n    if (strchr(filename, '/') != NULL) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Read file data with TOCTOU protection: open first, validate handle, then read */\nFileData* read_file_data(const char *filename) {\n    int fd = -1;\n    struct stat st;\n    FileData *result = NULL;\n    char *buffer = NULL;\n    size_t buffer_capacity = 0;\n    size_t total_read = 0;\n    ssize_t bytes_read = 0;\n    \n    /* Validate input parameter */\n    if (filename == NULL) {\n        fprintf(stderr, "Error: Invalid filename parameter\\n");\n        return NULL;\n    }\n    \n    /* Validate filename format before opening */\n    if (!validate_filename_format(filename)) {\n        fprintf(stderr, "Error: Invalid filename format\\n");\n        return NULL;\n    }\n    \n    /* Open file with secure flags: O_RDONLY, O_NOFOLLOW to prevent symlink following, O_CLOEXEC */\n    fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);\n    if (fd < 0) {\n        fprintf(stderr, "Error: Cannot open file\\n");\n        return NULL;\n    }\n    \n    /* Validate the opened file descriptor with fstat (TOCTOU protection) */\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Error: Cannot stat file\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    /* Ensure it's a regular file, not a directory or special file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return NULL;
    }
    
    /* Check file size limit to prevent excessive memory allocation */
    if (st.st_size > MAX_FILE_SIZE || st.st_size < 0) {
        fprintf(stderr, "Error: File size exceeds limit or is invalid\\n");
        close(fd);
        return NULL;
    }
    
    /* Allocate result structure */
    result = (FileData *)calloc(1, sizeof(FileData));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return NULL;
    }
    result->data = NULL;
    result->size = 0;
    
    /* Allocate initial buffer */
    buffer_capacity = (st.st_size > 0) ? (size_t)st.st_size + 1 : INITIAL_BUFFER_SIZE;
    buffer = (char *)malloc(buffer_capacity);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        free(result);
        return NULL;
    }
    
    /* Read file content with bounds checking */
    while (1) {
        /* Prevent integer overflow in size calculations */
        if (total_read > MAX_FILE_SIZE) {
            fprintf(stderr, "Error: File size limit exceeded during read\\n");
            goto cleanup_error;
        }
        
        /* Ensure buffer has space; grow if needed */
        if (total_read + INITIAL_BUFFER_SIZE > buffer_capacity) {
            size_t new_capacity = buffer_capacity * 2;
            
            /* Check for overflow and limit */
            if (new_capacity > MAX_FILE_SIZE || new_capacity < buffer_capacity) {
                fprintf(stderr, "Error: Buffer size limit reached\\n");
                goto cleanup_error;
            }
            
            char *new_buffer = (char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                fprintf(stderr, "Error: Memory reallocation failed\\n");
                goto cleanup_error;
            }
            buffer = new_buffer;
            buffer_capacity = new_capacity;
        }
        
        /* Read chunk with bounds check */
        size_t space_available = buffer_capacity - total_read - 1;
        if (space_available == 0) {
            fprintf(stderr, "Error: No space available in buffer\\n");
            goto cleanup_error;
        }
        
        bytes_read = read(fd, buffer + total_read, space_available);
        
        if (bytes_read < 0) {
            fprintf(stderr, "Error: Read failed\\n");
            goto cleanup_error;
        }
        
        if (bytes_read == 0) {
            /* End of file reached */
            break;
        }
        
        total_read += (size_t)bytes_read;
    }
    
    /* Null-terminate the buffer */
    buffer[total_read] = '\\0';
    
    /* Assign to result */
    result->data = buffer;
    result->size = total_read;
    
    close(fd);
    return result;

cleanup_error:
    /* Clean up on error path */
    if (buffer != NULL) {
        explicit_zero(buffer, buffer_capacity);
        free(buffer);
    }
    close(fd);
    free(result);
    return NULL;
}

/* Main function with test cases */
int main(int argc, char *argv[]) {
    FileData *file_data = NULL;
    
    /* Test case execution if no arguments provided */
    if (argc < 2) {
        printf("Creating test files and running test cases...\\n\\n");
        
        /* Test case 1: Valid file */
        FILE *test1 = fopen("test1.txt", "w");
        if (test1 != NULL) {
            fprintf(test1, "Hello, World!");
            fclose(test1);
            
            printf("Test 1 - Valid file:\\n");
            file_data = read_file_data("test1.txt");
            if (file_data != NULL) {
                printf("Success: Read %zu bytes: %s\\n", file_data->size, file_data->data);
                free_file_data(file_data);
                free(file_data);
            }
            remove("test1.txt");
            printf("\\n");
        }
        
        /* Test case 2: Non-existent file */
        printf("Test 2 - Non-existent file:\\n");
        file_data = read_file_data("nonexistent.txt");
        if (file_data == NULL) {
            printf("Expected failure: File does not exist\\n");
        }
        printf("\\n");
        
        /* Test case 3: Path traversal attempt */
        printf("Test 3 - Path traversal attempt:\\n");
        file_data = read_file_data("../etc/passwd");
        if (file_data == NULL) {
            printf("Expected failure: Path traversal rejected\\n");
        }
        printf("\\n");
        
        /* Test case 4: Empty filename */
        printf("Test 4 - Empty filename:\\n");
        file_data = read_file_data("");
        if (file_data == NULL) {
            printf("Expected failure: Empty filename rejected\\n");
        }
        printf("\\n");
        
        /* Test case 5: Valid file with multiple lines */
        FILE *test5 = fopen("test5.txt", "w");
        if (test5 != NULL) {
            fprintf(test5, "Line 1\\nLine 2\\nLine 3\\n");
            fclose(test5);
            
            printf("Test 5 - Multi-line file:\\n");
            file_data = read_file_data("test5.txt");
            if (file_data != NULL) {
                printf("Success: Read %zu bytes:\\n%s", file_data->size, file_data->data);
                free_file_data(file_data);
                free(file_data);
            }
            remove("test5.txt");
            printf("\\n");
        }
        
        return 0;
    }
    
    /* Command line argument mode */
    const char *filename = argv[1];
    
    printf("Attempting to read file: %s\\n", filename);
    
    file_data = read_file_data(filename);
    
    if (file_data == NULL) {
        fprintf(stderr, "Failed to read file\\n");
        return 1;
    }
    
    printf("Successfully read %zu bytes from file:\\n", file_data->size);
    printf("%s\\n", file_data->data);
    
    /* Clean up */
    free_file_data(file_data);
    free(file_data);
    
    return 0;
}
