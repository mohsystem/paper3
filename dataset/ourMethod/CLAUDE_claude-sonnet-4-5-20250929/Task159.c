
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

/* Constants for buffer size limits */
#define MIN_BUFFER_SIZE 1
#define MAX_BUFFER_SIZE 1048576  /* 1MB max to prevent excessive memory usage */

/* Secure buffer structure */
typedef struct {
    char* data;          /* Rule#1: Pointer initialized and managed carefully */
    size_t size;         /* Size of allocated buffer */
} SecureBuffer;

/* Rule#1: Initialize buffer with NULL checks and validation */
SecureBuffer* create_buffer(size_t size) {
    SecureBuffer* buffer = NULL;
    
    /* Rule#3: Validate buffer size before allocation */
    if (size < MIN_BUFFER_SIZE || size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size must be between %d and %d bytes\\n", 
                MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
        return NULL;
    }
    
    /* Rule#1: Allocate buffer structure and check for NULL */
    buffer = (SecureBuffer*)malloc(sizeof(SecureBuffer));
    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate buffer structure\\n");
        return NULL;
    }
    
    /* Rule#1: Initialize all fields */
    buffer->data = NULL;
    buffer->size = 0;
    
    /* Rule#1: Allocate data array with calloc (initializes to zero) */
    buffer->data = (char*)calloc(size, sizeof(char));
    if (buffer->data == NULL) {
        fprintf(stderr, "Error: Failed to allocate buffer data\\n");
        free(buffer);  /* Rule#1: Clean up on failure */
        return NULL;
    }
    
    buffer->size = size;
    return buffer;
}

/* Rule#3: Read from buffer with strict bounds checking */
int read_at_index(SecureBuffer* buffer, size_t index, char* output) {
    /* Rule#1: Validate input parameters */
    if (buffer == NULL || buffer->data == NULL || output == NULL) {
        fprintf(stderr, "Error: Invalid buffer or output pointer\\n");
        return 0;
    }
    
    /* Rule#3: Strict bounds checking before buffer access */
    if (index >= buffer->size) {
        fprintf(stderr, "Error: Index %zu out of bounds (buffer size: %zu)\\n", 
                index, buffer->size);
        return 0;
    }
    
    /* Safe read within bounds */
    *output = buffer->data[index];
    return 1;
}

/* Rule#3: Write to buffer with strict bounds checking */
int write_at_index(SecureBuffer* buffer, size_t index, char value) {
    /* Rule#1: Validate input parameters */
    if (buffer == NULL || buffer->data == NULL) {
        fprintf(stderr, "Error: Invalid buffer pointer\\n");
        return 0;
    }
    
    /* Rule#3: Strict bounds checking before buffer access */
    if (index >= buffer->size) {
        fprintf(stderr, "Error: Index %zu out of bounds (buffer size: %zu)\\n", 
                index, buffer->size);
        return 0;
    }
    
    buffer->data[index] = value;
    return 1;
}

/* Rule#1: Properly free allocated memory */
void destroy_buffer(SecureBuffer* buffer) {
    if (buffer != NULL) {
        if (buffer->data != NULL) {
            /* Rule#1: Clear sensitive data before freeing */
            memset(buffer->data, 0, buffer->size);
            free(buffer->data);
            buffer->data = NULL;
        }
        buffer->size = 0;
        free(buffer);
    }
}

/* Rule#4: Initialize buffer with data, respecting boundaries */
int initialize_buffer_data(SecureBuffer* buffer, const char* data) {
    size_t data_len;
    size_t copy_len;
    
    /* Rule#1: Validate parameters */
    if (buffer == NULL || buffer->data == NULL || data == NULL) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return 0;
    }
    
    /* Rule#4: Ensure strings are null-terminated and measure safely */
    data_len = strnlen(data, buffer->size + 1);
    
    /* Rule#4: Copy only up to buffer size to prevent overflow */
    copy_len = (data_len < buffer->size) ? data_len : buffer->size;
    memcpy(buffer->data, data, copy_len);
    
    return 1;
}

/* Rule#6: Validate user input for index */
int get_validated_index(size_t max_size, size_t* index) {
    char input[32];
    char* endptr = NULL;
    unsigned long value;
    size_t i;
    
    /* Rule#1: Initialize variables */
    memset(input, 0, sizeof(input));
    
    printf("Enter index (0-%zu): ", max_size - 1);
    
    /* Rule#1: Use fgets with width limit instead of scanf */
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return 0;
    }
    
    /* Rule#6: Remove trailing newline */
    input[strcspn(input, "\\n")] = '\\0';
    
    /* Rule#6: Validate input is not empty */
    if (strlen(input) == 0) {
        fprintf(stderr, "Error: Empty input\\n");
        return 0;
    }
    
    /* Rule#6: Validate input contains only digits */
    for (i = 0; i < strlen(input); i++) {
        if (!isdigit((unsigned char)input[i])) {
            fprintf(stderr, "Error: Invalid input, only digits allowed\\n");
            return 0;
        }
    }
    
    /* Rule#6: Safe conversion with overflow check */
    errno = 0;
    value = strtoul(input, &endptr, 10);
    
    /* Rule#1: Check return values and validate conversion */
    if (errno == ERANGE || value > SIZE_MAX) {
        fprintf(stderr, "Error: Number too large\\n");
        return 0;
    }
    
    if (endptr == input || *endptr != '\\0') {
        fprintf(stderr, "Error: Invalid number format\\n");
        return 0;
    }
    
    /* Rule#3: Validate index is within bounds */
    if (value >= max_size) {
        fprintf(stderr, "Error: Index must be less than %zu\\n", max_size);
        return 0;
    }
    
    *index = (size_t)value;
    return 1;
}

int main(void) {
    SecureBuffer* buf = NULL;
    char value;
    size_t index;
    
    /* Test Case 1: Valid buffer creation and read */
    printf("=== Test Case 1: Valid buffer creation and read ===\\n");
    buf = create_buffer(100);
    if (buf != NULL) {
        initialize_buffer_data(buf, "Hello, World! This is a secure buffer.");
        if (read_at_index(buf, 7, &value)) {
            printf("Character at index 7: '%c'\\n", value);
        }
        destroy_buffer(buf);
        buf = NULL;
    }
    
    /* Test Case 2: Out of bounds read attempt */
    printf("\\n=== Test Case 2: Out of bounds read attempt ===\\n");
    buf = create_buffer(50);
    if (buf != NULL) {
        initialize_buffer_data(buf, "Test buffer");
        read_at_index(buf, 100, &value); /* Should fail bounds check */
        destroy_buffer(buf);
        buf = NULL;
    }
    
    /* Test Case 3: Boundary read (last valid index) */
    printf("\\n=== Test Case 3: Boundary read (last valid index) ===\\n");
    buf = create_buffer(20);
    if (buf != NULL) {
        initialize_buffer_data(buf, "Boundary test data!!");
        if (read_at_index(buf, 19, &value)) {
            printf("Character at index 19: '%c'\\n", value);
        }
        destroy_buffer(buf);
        buf = NULL;
    }
    
    /* Test Case 4: Write and read back */
    printf("\\n=== Test Case 4: Write and read back ===\\n");
    buf = create_buffer(30);
    if (buf != NULL) {
        if (write_at_index(buf, 15, 'X')) {
            if (read_at_index(buf, 15, &value)) {
                printf("Written and read back: '%c'\\n", value);
            }
        }
        destroy_buffer(buf);
        buf = NULL;
    }
    
    /* Test Case 5: Invalid buffer size */
    printf("\\n=== Test Case 5: Invalid buffer size ===\\n");
    buf = create_buffer(MAX_BUFFER_SIZE + 1); /* Should fail */
    if (buf == NULL) {
        printf("Correctly rejected invalid buffer size\\n");
    } else {
        destroy_buffer(buf);
        buf = NULL;
    }
    
    return 0;
}
