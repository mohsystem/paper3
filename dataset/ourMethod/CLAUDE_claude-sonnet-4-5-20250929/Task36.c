
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Validates that the file path doesn't contain directory traversal patterns */\nbool is_valid_file_path(const char* path) {\n    size_t len;\n    size_t i;\n    \n    /* Check for NULL pointer to prevent crashes */\n    if (path == NULL) {\n        return false;\n    }\n    \n    len = strlen(path);\n    \n    /* Reject empty or overly long paths */\n    if (len == 0 || len > 4096) {\n        return false;\n    }\n    \n    /* Check for null bytes in the middle of string (injection attempt) */\n    for (i = 0; i < len; i++) {\n        if (path[i] == '\\0' && i < len - 1) {\n            return false;\n        }\n    }\n    \n    /* Reject directory traversal patterns */\n    if (strstr(path, "..") != NULL) {\n        return false;\n    }\n    \n    /* Reject absolute paths to prevent accessing system files */\n    if (path[0] == '/' || (len > 1 && path[1] == ':')) {\n        return false;\n    }\n    \n    return true;\n}\n\n/* Safely reads file contents with memory limits to prevent resource exhaustion */\nbool read_file_contents(const char* filename) {\n    FILE* file = NULL;\n    long file_size = 0;\n    size_t bytes_read = 0;\n    char buffer[4096];\n    const long MAX_FILE_SIZE = 10 * 1024 * 1024; /* 10MB limit */\n    \n    /* Validate input path to prevent directory traversal (CWE-22) */\n    if (!is_valid_file_path(filename)) {\n        fprintf(stderr, "Error: Invalid file path\\n");\n        return false;\n    }\n    \n    /* Open file in binary mode for safe reading, using "rb" mode */\n    file = fopen(filename, "rb");\n    \n    /* Check if file opened successfully */\n    if (file == NULL) {\n        fprintf(stderr, "Error: Could not open file '%s'\
", filename);\n        return false;\n    }\n    \n    /* Get file size to validate before reading */\n    if (fseek(file, 0, SEEK_END) != 0) {\n        fprintf(stderr, "Error: Could not seek to end of file\
");\n        fclose(file);\n        return false;\n    }\n    \n    file_size = ftell(file);\n    if (file_size < 0) {\n        fprintf(stderr, "Error: Could not determine file size\
");\n        fclose(file);\n        return false;\n    }\n    \n    /* Validate file size to prevent resource exhaustion */\n    if (file_size > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: File too large (max 10MB)\
");\n        fclose(file);\n        return false;\n    }\n    \n    /* Seek back to beginning */\n    if (fseek(file, 0, SEEK_SET) != 0) {\n        fprintf(stderr, "Error: Could not seek to beginning of file\
");\n        fclose(file);\n        return false;\n    }\n    \n    /* Read and print file contents in chunks with boundary checks (CWE-119, CWE-120) */\n    while (!feof(file) && !ferror(file)) {\n        /* Clear buffer before reading to prevent data leakage */\n        memset(buffer, 0, sizeof(buffer));\n        \n        /* Read with size limit to prevent buffer overflow */\n        bytes_read = fread(buffer, 1, sizeof(buffer), file);\n        \n        /* Validate bytes read is within bounds */\n        if (bytes_read > 0 && bytes_read <= sizeof(buffer)) {\n            /* Write exact number of bytes read to prevent over-read */\n            if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {\n                fprintf(stderr, "\
Error: Failed to write to stdout\
");\n                fclose(file);\n                return false;\n            }\n        }\n        \n        /* Check for read errors */\n        if (ferror(file)) {\n            fprintf(stderr, "\
Error: Failed to read file\
");\n            fclose(file);\n            return false;\n        }\n    }\n    \n    /* Ensure file is closed to prevent resource leak */\n    fclose(file);\n    return true;\n}\n\nint main(void) {\n    FILE* test_file = NULL;\n    \n    /* Test case 1: Valid file */\n    printf("=== Test 1: Reading valid file ===\
");\n    test_file = fopen("test1.txt", "w");\n    if (test_file != NULL) {\n        fprintf(test_file, "Hello, World!\
This is a test file.");\n        fclose(test_file);\n        read_file_contents("test1.txt");\n        printf("\
\
");\n    }\n    \n    /* Test case 2: Non-existent file */\n    printf("=== Test 2: Non-existent file ===\
");\n    read_file_contents("nonexistent.txt");\n    printf("\
");\n    \n    /* Test case 3: Directory traversal attempt (should be rejected) */\n    printf("=== Test 3: Directory traversal attempt ===\
");\n    read_file_contents("../etc/passwd");\n    printf("\
");\n    \n    /* Test case 4: Empty filename */\n    printf("=== Test 4: Empty filename ===\
");\n    read_file_contents("");\n    printf("\
");\n    \n    /* Test case 5: File with special characters */\n    printf("=== Test 5: File with numbers and underscores ===\
");\n    test_file = fopen("test_file_123.txt", "w");\n    if (test_file != NULL) {\n        fprintf(test_file, "File with special characters in name.");\n        fclose(test_file);\n        read_file_contents("test_file_123.txt");\n        printf("\
\
");
    }
    
    return 0;
}
