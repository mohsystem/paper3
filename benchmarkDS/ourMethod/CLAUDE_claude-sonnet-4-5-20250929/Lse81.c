
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * Security measures applied:
 * - malloc result checked for NULL (Rules#8)
 * - Buffer size validated before use
 * - fgets used instead of gets to prevent overflow (C-specific guidance)
 * - Input length validated and bounded
 * - Null termination ensured
 * - Memory freed exactly once
 * - Error handling with generic messages (Rules#11)
 */

char* allocate_buffer(void) {
    /* Allocate buffer with size check */
    char *buf = (char *)malloc(1024);
    if (buf == NULL) {
        /* Rules#8: Check allocation results */
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    /* Rules#8: Initialize allocated memory */
    memset(buf, 0, 1024);
    return buf;
}

int read_characters(char *buf) {
    /* Rules#2: Validate input parameter */
    if (buf == NULL) {
        fprintf(stderr, "Invalid buffer\\n");
        return -1;
    }
    
    /* Rules#8: Bound all reads - read max 10 characters + newline + null */
    /* Using fgets is safer than gets (C-specific guidance) */
    printf("Enter up to 10 characters: ");
    fflush(stdout);
    
    if (fgets(buf, 12, stdin) == NULL) {
        /* C-specific: Check return value */
        fprintf(stderr, "Input read failed\\n");
        return -1;
    }
    
    /* Rules#8: Ensure null termination */
    buf[11] = '\\0';
    
    /* Remove trailing newline if present */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\\n') {
        buf[len - 1] = '\\0';
    }
    
    /* Rules#2: Validate length constraint - only keep first 10 chars */
    if (strlen(buf) > 10) {
        buf[10] = '\\0';
    }
    
    return 0;
}

int main(void) {
    /* Test case 1: Normal operation */
    printf("Test 1: Normal input\\n");
    char *buffer1 = allocate_buffer();
    if (buffer1 != NULL) {
        if (read_characters(buffer1) == 0) {
            printf("Read: '%s'\\n", buffer1);
        }
        free(buffer1);
    }
    printf("\\n");
    
    /* Test case 2: Empty input */
    printf("Test 2: Empty input (just press Enter)\\n");
    char *buffer2 = allocate_buffer();
    if (buffer2 != NULL) {
        if (read_characters(buffer2) == 0) {
            printf("Read: '%s'\\n", buffer2);
        }
        free(buffer2);
    }
    printf("\\n");
    
    /* Test case 3: Exactly 10 characters */
    printf("Test 3: Enter exactly 10 characters\\n");
    char *buffer3 = allocate_buffer();
    if (buffer3 != NULL) {
        if (read_characters(buffer3) == 0) {
            printf("Read: '%s'\\n", buffer3);
        }
        free(buffer3);
    }
    printf("\\n");
    
    /* Test case 4: More than 10 characters (should truncate) */
    printf("Test 4: Enter more than 10 characters (will be truncated)\\n");
    char *buffer4 = allocate_buffer();
    if (buffer4 != NULL) {
        if (read_characters(buffer4) == 0) {
            printf("Read: '%s'\\n", buffer4);
        }
        free(buffer4);
    }
    printf("\\n");
    
    /* Test case 5: Special characters */
    printf("Test 5: Enter special characters\\n");
    char *buffer5 = allocate_buffer();
    if (buffer5 != NULL) {
        if (read_characters(buffer5) == 0) {
            printf("Read: '%s'\\n", buffer5);
        }
        free(buffer5);
    }
    
    return 0;
}
