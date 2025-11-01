
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define BASE_DIR "images"
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define BUFFER_SIZE 8192

/* Structure to hold file content and size */
typedef struct {
    unsigned char* data;
    size_t size;
} FileContent;

/* URL decode helper function */
static char* url_decode(const char* encoded) {
    if (encoded == NULL) return NULL;
    
    size_t len = strlen(encoded);
    char* decoded = (char*)malloc(len + 1);
    if (decoded == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (encoded[i] == '%' && i + 2 < len) {
            int value;
            if (sscanf(&encoded[i + 1], "%2x", &value) == 1) {
                decoded[j++] = (char)value;
                i += 2;
            } else {
                decoded[j++] = encoded[i];
            }
        } else if (encoded[i] == '+') {
            decoded[j++] = ' ';
        } else {
            decoded[j++] = encoded[i];
        }
    }
    decoded[j] = '\\0';
    return decoded;
}

/* Check if path is within base directory */
static int is_path_within_base(const char* path, const char* base) {
    size_t base_len = strlen(base);
    if (strncmp(path, base, base_len) != 0) {
        return 0;
    }
    if (path[base_len] == '\\0' || path[base_len] == '/') {
        return 1;
    }
    return 0;
}

/* Main img() function: opens and returns file content */
FileContent* img(const char* name) {
    FileContent* result = NULL;
    char* decoded_name = NULL;
    char requested_path[PATH_MAX];
    char canonical_base[PATH_MAX];
    char resolved_path[PATH_MAX];
    int fd = -1;
    struct stat st;
    unsigned char* buffer = NULL;
    
    /* Input validation: reject NULL or empty names */
    if (name == NULL || name[0] == '\\0') {
        fprintf(stderr, "Error: File name cannot be NULL or empty\\n");
        goto cleanup;
    }
    
    /* Decode URL-encoded input */
    decoded_name = url_decode(name);
    if (decoded_name == NULL) {
        fprintf(stderr, "Error: Failed to decode file name\\n");
        goto cleanup;
    }
    
    /* Sanitize: reject path traversal attempts */
    if (strstr(decoded_name, "..") != NULL ||
        strchr(decoded_name, '/') != NULL ||
        strchr(decoded_name, '\\\\') != NULL ||\n        decoded_name[0] == '.') {\n        fprintf(stderr, "Error: Invalid file name - path traversal not allowed\\n");\n        goto cleanup;\n    }\n    \n    /* Get canonical base directory path */\n    if (realpath(BASE_DIR, canonical_base) == NULL) {\n        /* Try to create directory if it doesn't exist */
        mkdir(BASE_DIR, 0755);
        if (realpath(BASE_DIR, canonical_base) == NULL) {
            fprintf(stderr, "Error: Failed to resolve base directory\\n");
            goto cleanup;
        }
    }
    
    /* Construct requested path - check for buffer overflow */
    if (snprintf(requested_path, sizeof(requested_path), "%s/%s", 
                 BASE_DIR, decoded_name) >= (int)sizeof(requested_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        goto cleanup;
    }
    
    /* Open file with O_RDONLY | O_NOFOLLOW | O_CLOEXEC */
    fd = open(requested_path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "Error: File not found\\n");
        } else if (errno == ELOOP) {
            fprintf(stderr, "Error: Symlink not allowed\\n");
        } else {
            fprintf(stderr, "Error: Failed to open file\\n");
        }
        goto cleanup;
    }
    
    /* Validate file attributes after opening (TOCTOU prevention) */
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file\\n");
        goto cleanup;
    }
    
    /* Reject if not a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        goto cleanup;
    }
    
    /* Check file size limit */
    if (st.st_size > MAX_FILE_SIZE || st.st_size < 0) {
        fprintf(stderr, "Error: File size exceeds limit or is invalid\\n");
        goto cleanup;
    }
    
    /* Verify resolved path is within base directory using fd */
    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);
    ssize_t len = readlink(fd_path, resolved_path, sizeof(resolved_path) - 1);
    if (len > 0) {
        resolved_path[len] = '\\0';
        if (!is_path_within_base(resolved_path, canonical_base)) {
            fprintf(stderr, "Error: Access denied - path outside base directory\\n");
            goto cleanup;
        }
    }
    
    /* Allocate memory for file content - check for overflow */
    buffer = (unsigned char*)malloc(st.st_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory\\n");
        goto cleanup;
    }
    
    /* Read file content with bounded buffer */
    ssize_t total_read = 0;
    ssize_t bytes_read;
    
    while (total_read < st.st_size) {
        bytes_read = read(fd, buffer + total_read, st.st_size - total_read);
        if (bytes_read < 0) {
            fprintf(stderr, "Error: Failed to read file\\n");
            goto cleanup;
        }
        if (bytes_read == 0) {
            break;
        }
        total_read += bytes_read;
    }
    
    /* Allocate result structure */
    result = (FileContent*)malloc(sizeof(FileContent));
    if (result == NULL) {
        fprintf(stderr, "Error: Failed to allocate result structure\\n");
        goto cleanup;
    }
    
    result->data = buffer;
    result->size = total_read;
    buffer = NULL; /* Transfer ownership */
    
cleanup:
    if (decoded_name != NULL) {
        free(decoded_name);
    }
    if (fd >= 0) {
        close(fd);
    }
    if (buffer != NULL) {
        free(buffer);
    }
    
    return result;
}

/* Free FileContent structure */
void free_file_content(FileContent* content) {
    if (content != NULL) {
        if (content->data != NULL) {
            free(content->data);
        }
        free(content);
    }
}

int main(void) {
    /* Create test directory */
    mkdir(BASE_DIR, 0755);
    
    /* Create test files */
    FILE* f1 = fopen(BASE_DIR "/test1.txt", "w");
    if (f1) {
        fprintf(f1, "Test file 1");
        fclose(f1);
    }
    
    FILE* f2 = fopen(BASE_DIR "/test2.jpg", "w");
    if (f2) {
        fprintf(f2, "Image data");
        fclose(f2);
    }
    
    FILE* f3 = fopen(BASE_DIR "/safe.png", "w");
    if (f3) {
        fprintf(f3, "Safe image");
        fclose(f3);
    }
    
    /* Test case 1: Valid file */
    printf("Running Test 1...\\n");
    FileContent* content1 = img("test1.txt");
    if (content1 != NULL) {
        printf("Test 1 passed: %.*s\\n", (int)content1->size, content1->data);
        free_file_content(content1);
    } else {
        printf("Test 1 failed\\n");
    }
    
    /* Test case 2: Another valid file */
    printf("Running Test 2...\\n");
    FileContent* content2 = img("test2.jpg");
    if (content2 != NULL) {
        printf("Test 2 passed: %.*s\\n", (int)content2->size, content2->data);
        free_file_content(content2);
    } else {
        printf("Test 2 failed\\n");
    }
    
    /* Test case 3: Path traversal attempt (should fail) */
    printf("Running Test 3...\\n");
    FileContent* content3 = img("../etc/passwd");
    if (content3 != NULL) {
        printf("Test 3 FAILED: Path traversal succeeded\\n");
        free_file_content(content3);
    } else {
        printf("Test 3 passed: Path traversal blocked\\n");
    }
    
    /* Test case 4: NULL input (should fail) */
    printf("Running Test 4...\\n");
    FileContent* content4 = img(NULL);
    if (content4 != NULL) {
        printf("Test 4 FAILED: NULL accepted\\n");
        free_file_content(content4);
    } else {
        printf("Test 4 passed: NULL rejected\\n");
    }
    
    /* Test case 5: Non-existent file (should fail) */
    printf("Running Test 5...\\n");
    FileContent* content5 = img("nonexistent.txt");
    if (content5 != NULL) {
        printf("Test 5 FAILED: Non-existent file returned data\\n");
        free_file_content(content5);
    } else {
        printf("Test 5 passed: Non-existent file handled\\n");
    }
    
    return 0;
}
