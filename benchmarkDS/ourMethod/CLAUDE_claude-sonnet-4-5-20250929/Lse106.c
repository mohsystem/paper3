
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define O_CLOEXEC 0
#define O_NOFOLLOW 0
#endif

/* Secure method to create file with secret info */
char* create_secret_file(const char* secret_info) {
    if (!secret_info) {
        return NULL;
    }
    
    /* Use time and random for unique filename */
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    unsigned long rand_val = ((unsigned long)rand() << 32) | (unsigned long)rand();
    
    /* Allocate memory for filename */
    char* filename = (char*)malloc(256);
    if (!filename) {
        return NULL;
    }
    
    snprintf(filename, 256, "/tmp/secret_%016lx.txt", rand_val);
    
    /* Open file with restrictive permissions (O_CREAT | O_EXCL for atomic creation) */
    /* O_NOFOLLOW prevents following symlinks */
    int fd = open(filename, 
                  O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW,
                  S_IRUSR | S_IWUSR); /* 0600 permissions */
    
    if (fd < 0) {
        free(filename);
        return NULL;
    }
    
    /* Write secret data */
    size_t secret_len = strlen(secret_info);
    ssize_t written = write(fd, secret_info, secret_len);
    
    if (written < 0 || (size_t)written != secret_len) {
        close(fd);
        free(filename);
        return NULL;
    }
    
    /* Ensure data is written to disk */
    if (fsync(fd) < 0) {
        close(fd);
        free(filename);
        return NULL;
    }
    
    close(fd);
    
    /* Change permissions to read-only for owner (0400) */
    if (chmod(filename, S_IRUSR) < 0) {
        free(filename);
        return NULL;
    }
    
    return filename;
}

int main(void) {
    /* Test cases */
    const char* test_secrets[] = {
        "SuperSecret123!",
        "ClassifiedInformation",
        "TopSecretData",
        "ConfidentialInfo",
        "RestrictedAccess"
    };
    
    size_t num_tests = sizeof(test_secrets) / sizeof(test_secrets[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        char* filename = create_secret_file(test_secrets[i]);
        if (filename) {
            printf("%s\\n", filename);
            free(filename);
        } else {
            fprintf(stderr, "Error creating secret file\\n");
        }
    }
    
    return 0;
}
