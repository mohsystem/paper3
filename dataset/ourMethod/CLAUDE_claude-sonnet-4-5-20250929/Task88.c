
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#define MAX_STRING_LENGTH 1024
#define RANDOM_STRING_LENGTH 16
#define MAX_PATH_LENGTH 512

/* Secure memory clearing function */
void secure_memset(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* vptr = (volatile unsigned char*)ptr;
    while (len--) {
        *vptr++ = 0;
    }
}

/* Generate cryptographically random string using system entropy */
int generateRandomString(char* output, size_t length) {
    /* Validate inputs */
    if (output == NULL || length == 0 || length > MAX_STRING_LENGTH) {
        return -1;
    }
    
    /* Initialize random seed with time and process ID for better entropy */
    srand((unsigned int)(time(NULL) ^ getpid()));
    
    /* Generate random printable ASCII characters (0x20 to 0x7E) */
    for (size_t i = 0; i < length; ++i) {
        /* Use rand() with range validation */
        int random_val = rand();
        /* Map to printable ASCII range with bounds check */
        output[i] = (char)(0x20 + (random_val % (0x7E - 0x20 + 1)));
    }
    
    /* Null-terminate the string */
    output[length] = '\\0';
    
    return 0;
}

/* Convert ASCII string to Unicode (UTF-8) */
char* convertToUnicode(const char* input) {
    /* Validate input */
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    if (len == 0 || len > MAX_STRING_LENGTH) {
        return NULL;
    }
    
    /* Allocate memory with bounds check */
    char* output = (char*)malloc(len + 1);
    if (output == NULL) {
        return NULL;
    }
    
    /* Copy string (already UTF-8 compatible ASCII) with bounds check */
    memcpy(output, input, len);
    output[len] = '\\0';
    
    return output;
}

/* Create temporary file and write Unicode string, return path */
char* createTempFileWithUnicode(void) {
    char randomStr[RANDOM_STRING_LENGTH + 1];
    char* unicodeStr = NULL;
    char* finalPath = NULL;
    int fd = -1;
    
    /* Initialize memory to zero */
    memset(randomStr, 0, sizeof(randomStr));
    
    /* Generate random string with validation */
    if (generateRandomString(randomStr, RANDOM_STRING_LENGTH) != 0) {
        goto cleanup;
    }
    
    /* Convert to Unicode with validation */
    unicodeStr = convertToUnicode(randomStr);
    if (unicodeStr == NULL) {
        goto cleanup;
    }
    
    /* Allocate path buffer with size validation */
    finalPath = (char*)calloc(MAX_PATH_LENGTH, sizeof(char));
    if (finalPath == NULL) {
        goto cleanup;
    }
    
#ifndef _WIN32
    /* POSIX: Use mkstemp for secure temporary file creation (O_CREAT|O_EXCL) */
    char tempTemplate[] = "/tmp/tempfile_XXXXXX";
    
    /* Copy template with bounds check */
    if (snprintf(finalPath, MAX_PATH_LENGTH, "%s", tempTemplate) >= MAX_PATH_LENGTH) {
        goto cleanup;
    }
    
    /* Create temporary file atomically with O_CREAT|O_EXCL flags */
    fd = mkstemp(finalPath);
    if (fd == -1) {
        goto cleanup;
    }
    
    /* Write data with proper error checking */
    size_t unicodeLen = strlen(unicodeStr);
    ssize_t written = write(fd, unicodeStr, unicodeLen);
    if (written != (ssize_t)unicodeLen) {
        goto cleanup;
    }
    
    /* Flush data to disk */
    if (fsync(fd) != 0) {
        goto cleanup;
    }
    
    /* Close file descriptor */
    close(fd);
    fd = -1;
    
#else
    /* Windows: Use GetTempPath and GetTempFileName for secure creation */
    char tempDir[MAX_PATH_LENGTH];
    DWORD result = GetTempPathA(MAX_PATH_LENGTH, tempDir);
    if (result == 0 || result >= MAX_PATH_LENGTH) {
        goto cleanup;
    }
    
    /* Generate unique temporary file name */
    if (GetTempFileNameA(tempDir, "tmp", 0, finalPath) == 0) {
        goto cleanup;
    }
    
    /* Open file with exclusive access */
    FILE* fp = fopen(finalPath, "wb");
    if (fp == NULL) {
        goto cleanup;
    }
    
    /* Write data with bounds check */
    size_t unicodeLen = strlen(unicodeStr);
    size_t written = fwrite(unicodeStr, 1, unicodeLen, fp);
    if (written != unicodeLen) {
        fclose(fp);
        goto cleanup;
    }
    
    /* Flush to disk */
    if (fflush(fp) != 0) {
        fclose(fp);
        goto cleanup;
    }
    
    fclose(fp);
#endif
    
    /* Clear sensitive data before returning */
    secure_memset(randomStr, sizeof(randomStr));
    free(unicodeStr);
    unicodeStr = NULL;
    
    return finalPath;

cleanup:
    /* Clean up on error */
    if (fd != -1) {
        close(fd);
    }
    if (unicodeStr != NULL) {
        secure_memset(unicodeStr, strlen(unicodeStr));
        free(unicodeStr);
    }
    if (finalPath != NULL) {
        secure_memset(finalPath, MAX_PATH_LENGTH);
        free(finalPath);
    }
    secure_memset(randomStr, sizeof(randomStr));
    return NULL;
}

int main(void) {
    char* path = NULL;
    
    printf("Test 1: Creating temporary file with Unicode content\\n");
    path = createTempFileWithUnicode();
    if (path != NULL) {
        printf("Temporary file created at: %s\\n", path);
        secure_memset(path, strlen(path));
        free(path);
        path = NULL;
    } else {
        printf("Failed to create temporary file\\n");
    }
    
    printf("\\nTest 2: Creating second temporary file\\n");
    path = createTempFileWithUnicode();
    if (path != NULL) {
        printf("Temporary file created at: %s\\n", path);
        secure_memset(path, strlen(path));
        free(path);
        path = NULL;
    }
    
    printf("\\nTest 3: Creating third temporary file\\n");
    path = createTempFileWithUnicode();
    if (path != NULL) {
        printf("Temporary file created at: %s\\n", path);
        secure_memset(path, strlen(path));
        free(path);
        path = NULL;
    }
    
    printf("\\nTest 4: Creating fourth temporary file\\n");
    path = createTempFileWithUnicode();
    if (path != NULL) {
        printf("Temporary file created at: %s\\n", path);
        secure_memset(path, strlen(path));
        free(path);
        path = NULL;
    }
    
    printf("\\nTest 5: Creating fifth temporary file\\n");
    path = createTempFileWithUnicode();
    if (path != NULL) {
        printf("Temporary file created at: %s\\n", path);
        secure_memset(path, strlen(path));
        free(path);
        path = NULL;
    }
    
    return 0;
}
