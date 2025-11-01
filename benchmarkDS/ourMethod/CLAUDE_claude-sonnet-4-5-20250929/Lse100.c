
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

/* Security: Create file with restrictive permissions to prevent unauthorized access */
bool createSecureConfigFile(const char* filename) {
    if (filename == NULL) {
        fprintf(stderr, "Error: NULL filename\\n");
        return false;
    }
    
    /* Security: Use O_CREAT|O_EXCL to fail if file exists (prevents TOCTOU race) */
    /* O_NOFOLLOW prevents following symbolic links */
    /* O_CLOEXEC prevents file descriptor leaks across exec */
    int flags = O_CREAT | O_EXCL | O_WRONLY;
    
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    
    /* Security: Set mode to 0600 (owner read/write only) at creation time */
    /* This is more secure than creating with default permissions and changing later */
    mode_t mode = S_IRUSR | S_IWUSR;  /* 0600 permissions */
    
    /* Security: Open file with restrictive permissions atomically */
    int fd = open(filename, flags, mode);
    
    if (fd == -1) {
        /* Security: Log error without exposing sensitive details */
        fprintf(stderr, "Error creating file: %s\\n", strerror(errno));
        return false;
    }
    
    /* Security: Verify the opened file is a regular file using fstat */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: fstat failed\\n");
        close(fd);
        return false;
    }
    
    /* Security: Ensure it's a regular file, not a symlink or directory */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return false;
    }
    
    /* Security: Close file descriptor immediately after creation */
    close(fd);
    return true;
}

int main(void) {
    /* Test case 1: Create important_config file */
    printf("Test 1 - Create important_config: %s\\n", 
           createSecureConfigFile("important_config") ? "Success" : "Failed");
    
    /* Test case 2: Attempt to create same file again (should fail) */
    printf("Test 2 - Create duplicate (should fail): %s\\n", 
           createSecureConfigFile("important_config") ? "Success" : "Failed");
    
    /* Test case 3: Create different config file */
    printf("Test 3 - Create test_config1: %s\\n", 
           createSecureConfigFile("test_config1") ? "Success" : "Failed");
    
    /* Test case 4: Create another config file */
    printf("Test 4 - Create test_config2: %s\\n", 
           createSecureConfigFile("test_config2") ? "Success" : "Failed");
    
    /* Test case 5: Create secure settings file */
    printf("Test 5 - Create secure_settings: %s\\n", 
           createSecureConfigFile("secure_settings") ? "Success" : "Failed");
    
    /* Cleanup test files */
    unlink("important_config");
    unlink("test_config1");
    unlink("test_config2");
    unlink("secure_settings");
    
    return 0;
}
