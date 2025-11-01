
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

#define MAX_FILE_SIZE (1024 * 1024)
#define MAX_PATH_LEN 4096
#define BUFFER_SIZE 256

/* Secure random float generation using /dev/urandom */
static int get_random_float(float* result) {
    if (result == NULL) return -1;
    
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        perror("Failed to open /dev/urandom");
        return -1;
    }
    
    unsigned int rand_val = 0;
    ssize_t bytes_read = read(fd, &rand_val, sizeof(rand_val));
    close(fd);
    
    if (bytes_read != sizeof(rand_val)) {
        fprintf(stderr, "Failed to read random bytes\\n");
        return -1;
    }
    
    /* Convert to float in range [0, 1) */
    *result = (float)rand_val / (float)UINT_MAX;
    return 0;
}

/* Validate filename - reject path traversal */
static int validate_filename(const char* filename) {
    if (filename == NULL || filename[0] == '\\0') {
        fprintf(stderr, "Invalid filename\\n");
        return -1;
    }
    
    /* Check for path traversal patterns */
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL || 
        strchr(filename, '\\\\') != NULL) {
        fprintf(stderr, "Invalid filename: path traversal detected\\n");
        return -1;
    }
    
    /* Check length */
    if (strlen(filename) > 255) {
        fprintf(stderr, "Filename too long\\n");
        return -1;
    }
    
    return 0;
}

/* Explicit zeroing function that cannot be optimized away */
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

int generate_and_write_random_floats(const char* filename) {
    int result = -1;
    char* buffer = NULL;
    char temp_filename[MAX_PATH_LEN];
    int fd = -1;
    
    /* Validate filename */
    if (validate_filename(filename) != 0) {
        return -1;
    }
    
    /* Generate 3 random floats */
    float float1 = 0.0f, float2 = 0.0f, float3 = 0.0f;
    if (get_random_float(&float1) != 0 ||
        get_random_float(&float2) != 0 ||
        get_random_float(&float3) != 0) {
        fprintf(stderr, "Failed to generate random floats\\n");
        goto cleanup;
    }
    
    /* Allocate buffer for concatenated string */
    buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        goto cleanup;
    }
    
    /* Convert floats to strings and concatenate */
    int written = snprintf(buffer, BUFFER_SIZE, "%f%f%f", float1, float2, float3);
    if (written < 0 || written >= BUFFER_SIZE) {
        fprintf(stderr, "String formatting failed\\n");
        goto cleanup;
    }
    
    /* Validate size */
    if (written > MAX_FILE_SIZE) {
        fprintf(stderr, "Data exceeds maximum file size\\n");
        goto cleanup;
    }
    
    /* Create temp file in current directory with restrictive permissions */
    written = snprintf(temp_filename, sizeof(temp_filename), ".temp_XXXXXX");
    if (written < 0 || written >= sizeof(temp_filename)) {
        fprintf(stderr, "Temp filename creation failed\\n");
        goto cleanup;
    }
    
    /* Create temp file with O_EXCL to prevent race conditions */
    fd = mkstemp(temp_filename);
    if (fd < 0) {
        perror("Failed to create temp file");
        goto cleanup;
    }
    
    /* Set restrictive permissions (0600) */
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
        perror("Failed to set file permissions");
        goto cleanup;
    }
    
    /* Validate fd with fstat */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat failed");
        goto cleanup;
    }
    
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file\\n");
        goto cleanup;
    }
    
    /* Write to temp file */
    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written < 0 || bytes_written != (ssize_t)strlen(buffer)) {
        perror("Write failed");
        goto cleanup;
    }
    
    /* Flush and sync */
    if (fsync(fd) != 0) {
        perror("fsync failed");
        goto cleanup;
    }
    
    close(fd);
    fd = -1;
    
    /* Atomic rename to target filename */
    if (rename(temp_filename, filename) != 0) {
        perror("Rename failed");
        goto cleanup;
    }
    
    result = 0;
    
cleanup:
    if (fd >= 0) {
        close(fd);
        unlink(temp_filename); /* Remove temp file on error */
    }
    
    if (buffer != NULL) {
        secure_zero(buffer, BUFFER_SIZE);
        free(buffer);
    }
    
    return result;
}

int main(void) {
    printf("Test 1: %d\\n", generate_and_write_random_floats("output1.txt"));
    printf("Test 2: %d\\n", generate_and_write_random_floats("output2.txt"));
    printf("Test 3: %d\\n", generate_and_write_random_floats("result.txt"));
    printf("Test 4: %d\\n", generate_and_write_random_floats("../output.txt"));
    printf("Test 5: %d\\n", generate_and_write_random_floats(NULL));
    
    return 0;
}
