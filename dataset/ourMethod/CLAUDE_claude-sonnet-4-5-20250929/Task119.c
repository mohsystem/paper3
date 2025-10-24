
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

/* Security: Define maximum limits to prevent resource exhaustion */
#define MAX_LINE_SIZE 65536      /* Maximum CSV line size: 64KB */
#define MAX_FIELD_COUNT 1024     /* Maximum fields per row */
#define MAX_FILE_SIZE (10 * 1024 * 1024)  /* Maximum file size: 10MB */

/* CSV field structure - stores individual parsed field */
typedef struct {
    char *value;
    size_t length;
} CSVField;

/* CSV row structure - stores array of fields */
typedef struct {
    CSVField *fields;
    size_t field_count;
    size_t capacity;
} CSVRow;

/* Security: Explicit zero for sensitive data cleanup */
static void explicit_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) *p++ = 0;
}

/* Security: Safe string duplication with bounds checking */
static char* safe_strndup(const char *str, size_t max_len) {
    if (str == NULL) return NULL;
    
    /* Security: Find actual length, capped at max_len */
    size_t len = 0;
    while (len < max_len && str[len] != '\\0') {
        len++;
    }
    
    /* Security: Check for integer overflow before allocation */
    if (len >= SIZE_MAX - 1) {
        return NULL;
    }
    
    /* Security: Allocate and check result */
    char *dup = (char*)malloc(len + 1);
    if (dup == NULL) {
        return NULL;
    }
    
    /* Security: Safe copy with explicit null termination */
    if (len > 0) {
        memcpy(dup, str, len);
    }
    dup[len] = '\\0';
    
    return dup;
}

/* Security: Initialize CSV row structure */
static int csv_row_init(CSVRow *row, size_t initial_capacity) {
    if (row == NULL || initial_capacity == 0 || initial_capacity > MAX_FIELD_COUNT) {
        return -1;
    }
    
    /* Security: Check for integer overflow in allocation size */
    if (initial_capacity > SIZE_MAX / sizeof(CSVField)) {
        return -1;
    }
    
    row->fields = (CSVField*)calloc(initial_capacity, sizeof(CSVField));
    if (row->fields == NULL) {
        return -1;
    }
    
    row->field_count = 0;
    row->capacity = initial_capacity;
    return 0;
}

/* Security: Free CSV row and zero sensitive data */
static void csv_row_free(CSVRow *row) {
    if (row == NULL) return;
    
    if (row->fields != NULL) {
        /* Security: Free each field value and zero memory */
        for (size_t i = 0; i < row->field_count; i++) {
            if (row->fields[i].value != NULL) {
                explicit_zero(row->fields[i].value, row->fields[i].length);
                free(row->fields[i].value);
                row->fields[i].value = NULL;
            }
        }
        free(row->fields);
        row->fields = NULL;
    }
    
    row->field_count = 0;
    row->capacity = 0;
}

/* Security: Add field to row with bounds checking */
static int csv_row_add_field(CSVRow *row, const char *value, size_t length) {
    if (row == NULL || row->fields == NULL) {
        return -1;
    }
    
    /* Security: Enforce maximum field count */
    if (row->field_count >= MAX_FIELD_COUNT) {
        return -1;
    }
    
    /* Security: Grow array if needed with overflow checks */
    if (row->field_count >= row->capacity) {
        size_t new_capacity = row->capacity * 2;
        if (new_capacity > MAX_FIELD_COUNT) {
            new_capacity = MAX_FIELD_COUNT;
        }
        if (new_capacity <= row->capacity || new_capacity > SIZE_MAX / sizeof(CSVField)) {
            return -1;
        }
        
        CSVField *new_fields = (CSVField*)realloc(row->fields, new_capacity * sizeof(CSVField));
        if (new_fields == NULL) {
            return -1;
        }
        
        row->fields = new_fields;
        /* Security: Zero new memory */
        memset(&row->fields[row->capacity], 0, (new_capacity - row->capacity) * sizeof(CSVField));
        row->capacity = new_capacity;
    }
    
    /* Security: Safe string duplication with length limit */
    row->fields[row->field_count].value = safe_strndup(value, length);
    if (row->fields[row->field_count].value == NULL && length > 0) {
        return -1;
    }
    row->fields[row->field_count].length = length;
    row->field_count++;
    
    return 0;
}

/* Security: Parse CSV line handling quoted fields and escapes */
static int csv_parse_line(const char *line, size_t line_len, CSVRow *row) {
    if (line == NULL || row == NULL) {
        return -1;
    }
    
    /* Security: Validate line length */
    if (line_len > MAX_LINE_SIZE) {
        return -1;
    }
    
    size_t pos = 0;
    char field_buffer[MAX_LINE_SIZE];
    
    while (pos < line_len) {
        size_t field_pos = 0;
        int in_quotes = 0;
        
        /* Security: Parse field with bounds checking */
        while (pos < line_len && field_pos < MAX_LINE_SIZE - 1) {
            char c = line[pos];
            
            if (in_quotes) {
                if (c == '"') {
                    /* Security: Check for escaped quote */
                    if (pos + 1 < line_len && line[pos + 1] == '"') {
                        field_buffer[field_pos++] = '"';
                        pos += 2;
                    } else {
                        in_quotes = 0;
                        pos++;
                    }
                } else {
                    field_buffer[field_pos++] = c;
                    pos++;
                }
            } else {
                if (c == '"') {
                    in_quotes = 1;
                    pos++;
                } else if (c == ',') {
                    pos++;
                    break;
                } else if (c == '\\r' || c == '\\n') {
                    break;
                } else {
                    field_buffer[field_pos++] = c;
                    pos++;
                }
            }
        }
        
        /* Security: Null terminate field */
        field_buffer[field_pos] = '\\0';
        
        /* Security: Add field with bounds checking */
        if (csv_row_add_field(row, field_buffer, field_pos) != 0) {
            return -1;
        }
        
        /* Security: Handle end of line */
        if (pos >= line_len) break;
        if (line[pos] == '\\r' || line[pos] == '\\n') break;
    }
    
    return 0;
}

/* Security: Process CSV file with file size and resource limits */
static int csv_process_file(const char *filename) {
    if (filename == NULL) {
        fprintf(stderr, "Error: Invalid filename\\n");
        return -1;
    }
    
    /* Security: Validate filename length to prevent buffer issues */
    size_t filename_len = strlen(filename);
    if (filename_len == 0 || filename_len > 4096) {
        fprintf(stderr, "Error: Invalid filename length\\n");
        return -1;
    }
    
    /* Security: Check for directory traversal patterns */
    if (strstr(filename, "..") != NULL || filename[0] == '/' || 
        (filename_len > 1 && filename[1] == ':')) {
        fprintf(stderr, "Error: Path traversal detected\\n");
        return -1;
    }
    
    /* Security: Open file with read-only access */
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\\n", strerror(errno));
        return -1;
    }
    
    /* Security: Check file size to prevent resource exhaustion */
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: Cannot seek file\\n");
        fclose(file);
        return -1;
    }
    
    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "Error: Cannot get file size\\n");
        fclose(file);
        return -1;
    }
    
    /* Security: Enforce maximum file size */
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File too large (max %d bytes)\\n", MAX_FILE_SIZE);
        fclose(file);
        return -1;
    }
    
    rewind(file);
    
    /* Security: Allocate line buffer with size check */
    char *line_buffer = (char*)malloc(MAX_LINE_SIZE);
    if (line_buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(file);
        return -1;
    }
    
    size_t row_num = 0;
    int result = 0;
    
    /* Security: Process file line by line with bounds checking */
    while (fgets(line_buffer, MAX_LINE_SIZE, file) != NULL) {
        row_num++;
        
        /* Security: Get actual line length */
        size_t line_len = strlen(line_buffer);
        
        /* Security: Skip empty lines */
        if (line_len == 0) continue;
        
        /* Security: Check if line was truncated (no newline and buffer full) */
        if (line_len == MAX_LINE_SIZE - 1 && line_buffer[line_len - 1] != '\\n') {
            fprintf(stderr, "Error: Line %zu exceeds maximum length\\n", row_num);
            result = -1;
            break;
        }
        
        /* Security: Remove trailing newline characters */
        while (line_len > 0 && (line_buffer[line_len - 1] == '\\n' || 
                                line_buffer[line_len - 1] == '\\r')) {
            line_buffer[--line_len] = '\\0';
        }
        
        if (line_len == 0) continue;
        
        /* Security: Initialize row structure */
        CSVRow row;
        if (csv_row_init(&row, 16) != 0) {
            fprintf(stderr, "Error: Failed to initialize row\\n");
            result = -1;
            break;
        }
        
        /* Security: Parse line with bounds checking */
        if (csv_parse_line(line_buffer, line_len, &row) != 0) {
            fprintf(stderr, "Error: Failed to parse line %zu\\n", row_num);
            csv_row_free(&row);
            result = -1;
            break;
        }
        
        /* Output parsed row */
        printf("Row %zu (%zu fields):\\n", row_num, row.field_count);
        for (size_t i = 0; i < row.field_count; i++) {
            printf("  Field %zu: [%s]\\n", i + 1, 
                   row.fields[i].value ? row.fields[i].value : "");
        }
        
        /* Security: Free row resources */
        csv_row_free(&row);
    }
    
    /* Security: Check for read errors */
    if (ferror(file)) {
        fprintf(stderr, "Error: File read error\\n");
        result = -1;
    }
    
    /* Security: Zero sensitive buffer before free */
    explicit_zero(line_buffer, MAX_LINE_SIZE);
    free(line_buffer);
    fclose(file);
    
    return result;
}

/* Test cases */
int main(void) {
    int test_result = 0;
    
    printf("=== CSV Parser Test Cases ===\\n\\n");
    
    /* Test Case 1: Simple CSV */
    printf("Test 1: Simple CSV with 3 fields\\n");
    {
        FILE *test1 = fopen("test1.csv", "wb");
        if (test1) {
            fprintf(test1, "Name,Age,City\\n");
            fprintf(test1, "Alice,30,New York\\n");
            fprintf(test1, "Bob,25,Los Angeles\\n");
            fclose(test1);
            if (csv_process_file("test1.csv") != 0) test_result = -1;
            remove("test1.csv");
        }
    }
    printf("\\n");
    
    /* Test Case 2: Quoted fields with commas */
    printf("Test 2: Quoted fields containing commas\\n");
    {
        FILE *test2 = fopen("test2.csv", "wb");
        if (test2) {
            fprintf(test2, "Product,Description,Price\\n");
            fprintf(test2, "\\"Widget, Blue\\",\\"High quality, durable\\",29.99\\n");
            fprintf(test2, "Gadget,Simple device,15.50\\n");
            fclose(test2);
            if (csv_process_file("test2.csv") != 0) test_result = -1;
            remove("test2.csv");
        }
    }
    printf("\\n");
    
    /* Test Case 3: Escaped quotes */
    printf("Test 3: Fields with escaped quotes\\n");
    {
        FILE *test3 = fopen("test3.csv", "wb");
        if (test3) {
            fprintf(test3, "Title,Quote\\n");
            fprintf(test3, "Book,\\"He said \\"\\"Hello\\"\\" to me\\"\\n");
            fprintf(test3, "Movie,\\"She replied \\"\\"Goodbye\\"\\"\\"\\n");
            fclose(test3);
            if (csv_process_file("test3.csv") != 0) test_result = -1;
            remove("test3.csv");
        }
    }
    printf("\\n");
    
    /* Test Case 4: Empty fields */
    printf("Test 4: CSV with empty fields\\n");
    {
        FILE *test4 = fopen("test4.csv", "wb");
        if (test4) {
            fprintf(test4, "A,B,C,D\\n");
            fprintf(test4, "1,,3,\\n");
            fprintf(test4, ",2,,4\\n");
            fclose(test4);
            if (csv_process_file("test4.csv") != 0) test_result = -1;
            remove("test4.csv");
        }
    }
    printf("\\n");
    
    /* Test Case 5: Security test - path traversal attempt */
    printf("Test 5: Security - reject path traversal\\n");
    {
        /* Security: This should fail due to path traversal check */
        if (csv_process_file("../etc/passwd") == 0) {
            printf("SECURITY FAILURE: Path traversal not blocked!\\n");
            test_result = -1;
        } else {
            printf("Path traversal correctly rejected\\n");
        }
    }
    printf("\\n");
    
    printf("=== All tests completed ===\\n");
    return test_result;
}
