
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/* Function to create a file and make it executable */
/* Returns: 0 on success, -1 on failure */
int createExecutableScript(const char* filename) {
    FILE* file = NULL;
    const char* shebang = "#!/bin/bash\\n";
    const char* comment = "# Script created by createExecutableScript\\n";
    size_t filename_len = 0;
    const size_t MAX_FILENAME_LEN = 255;
    
    /* Input validation: ensure filename is not NULL and not empty */
    if (filename == NULL || filename[0] == '\\0') {
        fprintf(stderr, "Error: Invalid filename provided\\n");
        return -1;
    }
    
    /* Validate filename length to prevent buffer overflows */
    filename_len = strlen(filename);
    if (filename_len > MAX_FILENAME_LEN) {
        fprintf(stderr, "Error: Filename too long\\n");
        return -1;
    }
    
    /* Security: Validate filename doesn't contain path traversal sequences */\n    /* This prevents directory traversal attacks */\n    if (strstr(filename, "..") != NULL || \n        strchr(filename, '/') != NULL ||\n        strchr(filename, '\\\\') != NULL) {
        fprintf(stderr, "Error: Filename must not contain path separators or traversal sequences\\n");
        return -1;
    }
    
    /* Create file with exclusive create flag to prevent TOCTOU attacks */
    /* "wx" mode: create new file for writing, fail if exists */
    /* This is the secure way to create files in C (CWE-367 mitigation) */
    file = fopen(filename, "wx");
    if (file == NULL) {
        /* If file exists, try to overwrite with "w" mode for this example */
        /* In production, you may want to fail here instead */
        file = fopen(filename, "w");
        if (file == NULL) {
            fprintf(stderr, "Error: Failed to create file: %s\\n", strerror(errno));
            return -1;
        }
    }
    
    /* Write shell script header */
    if (fputs(shebang, file) == EOF) {
        fprintf(stderr, "Error: Failed to write shebang to file\\n");
        fclose(file);
        return -1;
    }
    
    if (fputs(comment, file) == EOF) {
        fprintf(stderr, "Error: Failed to write comment to file\\n");
        fclose(file);
        return -1;
    }
    
    /* Flush the output buffer to ensure data is written */
    if (fflush(file) != 0) {
        fprintf(stderr, "Error: Failed to flush file buffer\\n");
        fclose(file);
        return -1;
    }
    
    /* Close the file and check for errors */
    if (fclose(file) != 0) {
        fprintf(stderr, "Error: Failed to close file: %s\\n", strerror(errno));
        return -1;
    }
    
    /* Make the file executable (owner: rwx, group: r-x, others: r-x) */
    /* Using 0755 permissions: owner can read/write/execute, others can read/execute */
    if (chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        fprintf(stderr, "Error: Failed to set executable permissions: %s\\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

int main(void) {
    /* Test case 1: Create script.sh successfully */
    printf("Test 1: Creating script.sh\\n");
    if (createExecutableScript("script.sh") == 0) {
        printf("Success: script.sh created and made executable\\n");
    } else {
        printf("Failed to create script.sh\\n");
    }
    
    /* Test case 2: Try with null pointer (should fail) */
    printf("\\nTest 2: Null pointer test\\n");
    if (createExecutableScript(NULL) == -1) {
        printf("Correctly rejected null pointer\\n");
    }
    
    /* Test case 3: Try with empty string (should fail) */
    printf("\\nTest 3: Empty string test\\n");
    if (createExecutableScript("") == -1) {
        printf("Correctly rejected empty string\\n");
    }
    
    /* Test case 4: Try with path traversal (should fail) */
    printf("\\nTest 4: Path traversal test\\n");
    if (createExecutableScript("../script.sh") == -1) {
        printf("Correctly rejected path traversal attempt\\n");
    }
    
    /* Test case 5: Create another valid script */
    printf("\\nTest 5: Creating test_script.sh\\n");
    if (createExecutableScript("test_script.sh") == 0) {
        printf("Success: test_script.sh created and made executable\\n");
    } else {
        printf("Failed to create test_script.sh\\n");
    }
    
    return 0;
}
