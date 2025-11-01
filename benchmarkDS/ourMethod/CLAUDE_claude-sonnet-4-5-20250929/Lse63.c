
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define BASE_DIR "."

/* Explicit zero function that compiler cannot optimize away */
void explicit_zero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Normalize and validate path to prevent traversal */
int validate_path(const char* base, const char* input, char* resolved, size_t resolved_size) {
    char base_real[PATH_MAX];
    char input_full[PATH_MAX];
    
    if (!base || !input || !resolved) return 0;
    
    memset(base_real, 0, sizeof(base_real));
    memset(input_full, 0, sizeof(input_full));
    
    /* Resolve base directory */
    if (!realpath(base, base_real)) {
        return 0;
    }
    
    /* Build full input path */
    int ret = snprintf(input_full, sizeof(input_full), "%s/%s", base_real, input);
    if (ret < 0 || ret >= (int)sizeof(input_full)) {
        return 0;
    }
    
    /* Resolve input path */
    if (!realpath(input_full, resolved)) {
        /* If file doesn't exist, resolve parent and append filename */\n        char parent[PATH_MAX];\n        char filename[PATH_MAX];\n        char parent_real[PATH_MAX];\n        const char* last_slash;\n        size_t parent_len;\n        \n        memset(parent, 0, sizeof(parent));\n        memset(filename, 0, sizeof(filename));\n        memset(parent_real, 0, sizeof(parent_real));\n        \n        last_slash = strrchr(input_full, '/');
        if (!last_slash) return 0;
        
        parent_len = last_slash - input_full;
        if (parent_len >= sizeof(parent)) return 0;
        
        memcpy(parent, input_full, parent_len);
        strncpy(filename, last_slash + 1, sizeof(filename) - 1);
        
        if (!realpath(parent, parent_real)) {
            return 0;
        }
        
        ret = snprintf(resolved, resolved_size, "%s/%s", parent_real, filename);
        if (ret < 0 || ret >= (int)resolved_size) {
            return 0;
        }
    }
    
    /* Verify resolved path is within base directory */
    if (strncmp(resolved, base_real, strlen(base_real)) != 0) {
        return 0;
    }
    
    return 1;
}

int process_file(const char* input_path, const char* output_path) {
    char resolved_input[PATH_MAX];
    char resolved_output[PATH_MAX];
    unsigned char* buffer = NULL;
    size_t bytes_read = 0;
    unsigned int checksum = 0;
    int fd = -1;
    int out_fd = -1;
    struct stat st;
    char temp_output[PATH_MAX];
    char result[256];
    int len;
    ssize_t written;
    
    if (!input_path || !output_path) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return 0;
    }
    
    memset(resolved_input, 0, sizeof(resolved_input));
    memset(resolved_output, 0, sizeof(resolved_output));
    
    /* Validate and normalize paths */
    if (!validate_path(BASE_DIR, input_path, resolved_input, sizeof(resolved_input)) ||
        !validate_path(BASE_DIR, output_path, resolved_output, sizeof(resolved_output))) {
        fprintf(stderr, "Error: Path validation failed\\n");
        return 0;
    }
    
    /* Open file with O_RDONLY, O_CLOEXEC, and O_NOFOLLOW */
    fd = open(resolved_input, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to open input file\\n");
        return 0;
    }
    
    /* Use fstat on opened descriptor to validate file */
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file\\n");
        close(fd);
        return 0;
    }
    
    /* Check if regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return 0;
    }
    
    /* Check file size limit */
    if (st.st_size > (off_t)MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File exceeds maximum size\\n");
        close(fd);
        return 0;
    }
    
    /* Safe allocation with checked size */
    buffer = (unsigned char*)malloc((size_t)st.st_size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return 0;
    }
    
    /* Initialize buffer */
    memset(buffer, 0, (size_t)st.st_size);
    
    /* Read file content with bounds checking */
    {
        ssize_t total_read = 0;
        while (total_read < st.st_size) {
            ssize_t n = read(fd, buffer + total_read, (size_t)(st.st_size - total_read));
            if (n < 0) {
                if (errno == EINTR) continue;
                fprintf(stderr, "Error: Read failed\\n");
                explicit_zero(buffer, (size_t)st.st_size);
                free(buffer);
                close(fd);
                return 0;
            }
            if (n == 0) break;
            total_read += n;
        }
        bytes_read = (size_t)total_read;
    }
    close(fd);
    fd = -1;
    
    /* Calculate simple checksum */
    {
        size_t i;
        for (i = 0; i < bytes_read; i++) {
            checksum = (checksum + buffer[i]) & 0xFF;
        }
    }
    
    /* Write result to temp file then atomic rename */
    memset(temp_output, 0, sizeof(temp_output));
    snprintf(temp_output, sizeof(temp_output), "%s.tmp%d", resolved_output, getpid());
    
    out_fd = open(temp_output, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (out_fd < 0) {
        fprintf(stderr, "Error: Failed to create output file\\n");
        explicit_zero(buffer, (size_t)st.st_size);
        free(buffer);
        return 0;
    }
    
    memset(result, 0, sizeof(result));
    len = snprintf(result, sizeof(result), 
        "Bytes read: %zu\\nChecksum: %u\\nBuffer empty: %s\\n",
        bytes_read, checksum, (checksum == 0 ? "true" : "false"));
    
    if (len > 0 && len < (int)sizeof(result)) {
        written = write(out_fd, result, (size_t)len);
        if (written != len) {
            fprintf(stderr, "Error: Write failed\\n");
            close(out_fd);
            unlink(temp_output);
            explicit_zero(buffer, (size_t)st.st_size);
            free(buffer);
            return 0;
        }
    }
    
    fsync(out_fd);
    close(out_fd);
    out_fd = -1;
    
    /* Atomic rename */
    if (rename(temp_output, resolved_output) != 0) {
        fprintf(stderr, "Error: Rename failed\\n");
        unlink(temp_output);
        explicit_zero(buffer, (size_t)st.st_size);
        free(buffer);
        return 0;
    }
    
    /* Clear sensitive data */
    explicit_zero(buffer, (size_t)st.st_size);
    free(buffer);
    
    return 1;
}

int main(void) {
    FILE* f;
    int i;
    
    /* Test case 1: Normal file */
    f = fopen("test1.txt", "w");
    if (f) {
        fprintf(f, "Hello World");
        fclose(f);
    }
    printf("Test 1: %s\\n", process_file("test1.txt", "output1.txt") ? "PASS" : "FAIL");
    
    /* Test case 2: Empty file */
    f = fopen("test2.txt", "w");
    if (f) fclose(f);
    printf("Test 2: %s\\n", process_file("test2.txt", "output2.txt") ? "PASS" : "FAIL");
    
    /* Test case 3: File with zero checksum */
    f = fopen("test3.txt", "wb");
    if (f) {
        char zeros[3] = {0, 0, 0};
        fwrite(zeros, 1, 3, f);
        fclose(f);
    }
    printf("Test 3: %s\\n", process_file("test3.txt", "output3.txt") ? "PASS" : "FAIL");
    
    /* Test case 4: Large content */
    f = fopen("test4.txt", "wb");
    if (f) {
        for (i = 0; i < 1000; i++) {
            char c = i % 256;
            fwrite(&c, 1, 1, f);
        }
        fclose(f);
    }
    printf("Test 4: %s\\n", process_file("test4.txt", "output4.txt") ? "PASS" : "FAIL");
    
    /* Test case 5: Path traversal attempt (should fail) */
    printf("Test 5: %s\\n", !process_file("../test5.txt", "output5.txt") ? "PASS" : "FAIL");
    
    return 0;
}
