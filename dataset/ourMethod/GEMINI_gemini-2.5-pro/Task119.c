#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h> // For size_t

// Define limits to prevent resource exhaustion attacks
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define MAX_ROWS 10000
#define MAX_FIELDS_PER_ROW 100
#define MAX_FIELD_SIZE 10240 // 10 KB

// Data structures to hold parsed CSV data
typedef struct {
    char** fields;
    size_t count;
    size_t capacity;
} CsvRow;

typedef struct {
    CsvRow* rows;
    size_t count;
    size_t capacity;
} CsvData;

// State for the parser state machine
typedef enum {
    STATE_START_FIELD,
    STATE_IN_FIELD,
    STATE_IN_QUOTED_FIELD,
    STATE_IN_QUOTED_FIELD_QUOTE
} ParserState;

// Forward declarations
void free_csv_data(CsvData* data);
CsvData* parse_csv(const char* content, size_t content_len);
CsvData* read_and_parse_csv(const char* filename);
void print_csv_data(const CsvData* data);

// Function to free all memory associated with CsvData
void free_csv_data(CsvData* data) {
    if (!data) {
        return;
    }
    if (data->rows) {
        for (size_t i = 0; i < data->count; i++) {
            if (data->rows[i].fields) {
                for (size_t j = 0; j < data->rows[i].count; j++) {
                    free(data->rows[i].fields[j]);
                }
                free(data->rows[i].fields);
            }
        }
        free(data->rows);
    }
    free(data);
}

// Helper function to add a field to a row, handling dynamic resizing
static int add_field_to_row(CsvRow* row, char* field) {
    if (row->count >= MAX_FIELDS_PER_ROW) {
        fprintf(stderr, "Error: Exceeded maximum fields per row (%d).\n", MAX_FIELDS_PER_ROW);
        return -1;
    }
    if (row->count >= row->capacity) {
        size_t new_capacity = (row->capacity == 0) ? 8 : row->capacity * 2;
        if (new_capacity > MAX_FIELDS_PER_ROW) {
            new_capacity = MAX_FIELDS_PER_ROW;
        }
        char** new_fields = realloc(row->fields, new_capacity * sizeof(char*));
        if (!new_fields) {
            perror("realloc failed for fields");
            return -1;
        }
        row->fields = new_fields;
        row->capacity = new_capacity;
    }
    row->fields[row->count++] = field;
    return 0;
}

// Helper function to add a row to the data, handling dynamic resizing
static int add_row_to_data(CsvData* data, CsvRow* row) {
    if (data->count >= MAX_ROWS) {
        fprintf(stderr, "Error: Exceeded maximum rows (%d).\n", MAX_ROWS);
        return -1;
    }
    if (data->count >= data->capacity) {
        size_t new_capacity = (data->capacity == 0) ? 16 : data->capacity * 2;
        if (new_capacity > MAX_ROWS) {
            new_capacity = MAX_ROWS;
        }
        CsvRow* new_rows = realloc(data->rows, new_capacity * sizeof(CsvRow));
        if (!new_rows) {
            perror("realloc failed for rows");
            return -1;
        }
        data->rows = new_rows;
        data->capacity = new_capacity;
    }
    data->rows[data->count++] = *row;
    // Ownership of row content is transferred, so reset the source row
    *row = (CsvRow){0};
    return 0;
}


// Helper to append a character to the dynamic field buffer
static int append_char_to_field(char** buffer, size_t* len, size_t* cap, char c) {
    if (*len + 1 > *cap) {
        if (*cap >= MAX_FIELD_SIZE) {
            fprintf(stderr, "Error: Field size exceeds limit of %d bytes.\n", MAX_FIELD_SIZE);
            return -1;
        }
        size_t new_cap = (*cap == 0) ? 64 : *cap * 2;
        if (new_cap > MAX_FIELD_SIZE) new_cap = MAX_FIELD_SIZE;
        char* new_buf = realloc(*buffer, new_cap);
        if (!new_buf) {
            perror("realloc for field buffer failed");
            return -1;
        }
        *buffer = new_buf;
        *cap = new_cap;
    }
    (*buffer)[(*len)++] = c;
    return 0;
}

// Helper to finalize the current field and add it to the row
static int finalize_field(CsvRow* row, char** buffer, size_t* len, size_t* cap) {
    char* new_field = malloc(*len + 1);
    if (!new_field) {
        perror("malloc for new field failed");
        return -1;
    }
    if (*len > 0) {
        memcpy(new_field, *buffer, *len);
    }
    new_field[*len] = '\0';

    if (add_field_to_row(row, new_field) != 0) {
        free(new_field); // Cleanup on failure
        return -1;
    }
    *len = 0; // Reset buffer for the next field
    return 0;
}

// The core CSV parsing logic using a state machine
CsvData* parse_csv(const char* content, size_t content_len) {
    CsvData* data = calloc(1, sizeof(CsvData));
    if (!data) { perror("calloc for CsvData failed"); return NULL; }

    CsvRow current_row = {0};
    char* field_buffer = NULL;
    size_t field_len = 0;
    size_t field_cap = 0;
    ParserState state = STATE_START_FIELD;
    
    const char* p = content;
    const char* end = content + content_len;

    while (p < end) {
        const char c = *p;
        switch (state) {
            case STATE_START_FIELD:
                if (c == '"') {
                    state = STATE_IN_QUOTED_field_buffer;
                } else if (c == ',') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                } else if (c == '\n' || c == '\r') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                    if (add_row_to_data(data, &current_row) != 0) goto error;
                    if (c == '\r' && p + 1 < end && *(p + 1) == '\n') p++; // Skip LF of CRLF
                } else {
                    if (append_char_to_field(&field_buffer, &field_len, &field_cap, c) != 0) goto error;
                    state = STATE_IN_FIELD;
                }
                break;
            case STATE_IN_FIELD:
                if (c == ',') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                    state = STATE_START_FIELD;
                } else if (c == '\n' || c == '\r') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                    if (add_row_to_data(data, &current_row) != 0) goto error;
                    if (c == '\r' && p + 1 < end && *(p + 1) == '\n') p++;
                    state = STATE_START_FIELD;
                } else {
                    if (append_char_to_field(&field_buffer, &field_len, &field_cap, c) != 0) goto error;
                }
                break;
            case STATE_IN_QUOTED_FIELD:
                if (c == '"') {
                    state = STATE_IN_QUOTED_FIELD_QUOTE;
                } else {
                    if (append_char_to_field(&field_buffer, &field_len, &field_cap, c) != 0) goto error;
                }
                break;
            case STATE_IN_QUOTED_FIELD_QUOTE:
                if (c == '"') { // Escaped quote
                    if (append_char_to_field(&field_buffer, &field_len, &field_cap, '"') != 0) goto error;
                    state = STATE_IN_QUOTED_FIELD;
                } else if (c == ',') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                    state = STATE_START_FIELD;
                } else if (c == '\n' || c == '\r') {
                    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
                    if (add_row_to_data(data, &current_row) != 0) goto error;
                    if (c == '\r' && p + 1 < end && *(p + 1) == '\n') p++;
                    state = STATE_START_FIELD;
                } else {
                    fprintf(stderr, "Malformed CSV: unexpected characters after closing quote.\n");
                    goto error;
                }
                break;
        }
        p++;
    }

    // After the loop, handle any remaining data
    if (state == STATE_IN_QUOTED_FIELD) {
        fprintf(stderr, "Malformed CSV: unterminated quoted field.\n");
        goto error;
    }
    
    // Finalize the last field and row if the file does not end with a newline
    if (finalize_field(&current_row, &field_buffer, &field_len, &field_cap) != 0) goto error;
    if (current_row.count > 0) {
        if (add_row_to_data(data, &current_row) != 0) goto error;
    }

    free(field_buffer);
    return data;

error:
    free(field_buffer);
    // Cleanup current_row if it wasn't successfully added to data
    for (size_t i = 0; i < current_row.count; ++i) free(current_row.fields[i]);
    free(current_row.fields);
    free_csv_data(data);
    return NULL;
}


// Reads a file securely and passes its content to the parser
CsvData* read_and_parse_csv(const char* filename) {
    int fd = -1;
    char* buffer = NULL;
    CsvData* data = NULL;

    if (!filename) {
        fprintf(stderr, "Error: Filename is NULL.\n");
        return NULL;
    }

    // Prevent path traversal attacks
    if (strchr(filename, '/')) {
        fprintf(stderr, "Error: Filename cannot contain '/'.\n");
        return NULL;
    }

    // Secure file opening (avoid TOCTOU)
    fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd == -1) {
        perror("open failed");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        return NULL;
    }

    // Validate file type and size
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        close(fd);
        return NULL;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds the limit of %d bytes.\n", MAX_FILE_SIZE);
        close(fd);
        return NULL;
    }

    if (st.st_size == 0) { // Handle empty file gracefully
        close(fd);
        CsvData* empty_data = calloc(1, sizeof(CsvData));
        if (!empty_data) perror("calloc for empty data failed");
        return empty_data;
    }
    
    if ((unsigned long long)st.st_size >= SIZE_MAX) {
        fprintf(stderr, "Error: File size too large for address space.\n");
        close(fd);
        return NULL;
    }

    size_t buffer_size = st.st_size;
    buffer = malloc(buffer_size + 1);
    if (!buffer) {
        perror("malloc for file buffer failed");
        close(fd);
        return NULL;
    }

    ssize_t total_read = 0;
    while (total_read < st.st_size) {
        ssize_t bytes_read = read(fd, buffer + total_read, buffer_size - total_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue; // Interrupted, try again
            perror("read failed");
            free(buffer);
            close(fd);
            return NULL;
        }
        if (bytes_read == 0) break; // Unexpected EOF
        total_read += bytes_read;
    }
    
    close(fd);

    if (total_read != st.st_size) {
         fprintf(stderr, "Error: Could not read the entire file.\n");
         free(buffer);
         return NULL;
    }
    buffer[total_read] = '\0';

    data = parse_csv(buffer, total_read);
    
    free(buffer);
    return data;
}

// Helper to print CSV data for testing
void print_csv_data(const CsvData* data) {
    if (!data) {
        printf("CSV data is NULL.\n");
        return;
    }
    printf("--- CSV Data: %zu rows ---\n", data->count);
    for (size_t i = 0; i < data->count; i++) {
        printf("Row %zu: [", i + 1);
        for (size_t j = 0; j < data->rows[i].count; j++) {
            printf("\"%s\"", data->rows[i].fields[j]);
            if (j < data->rows[i].count - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
    printf("-------------------------\n\n");
}

// Helper to create a test file
int create_test_file(const char* filename, const char* content) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        perror("Failed to create test file");
        return -1;
    }
    size_t len = strlen(content);
    if (write(fd, content, len) != (ssize_t)len) {
        perror("Failed to write to test file");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int main() {
    // Test cases
    const char* filenames[] = {
        "test1.csv", "test2.csv", "test3.csv", "test4.csv", "test5.csv"
    };
    const char* contents[] = {
        // 1. Simple case without trailing newline
        "header1,header2,header3\n"
        "a,b,c\n"
        "1,2,3",
        // 2. Quoted fields with commas and newlines
        "id,name,description\n"
        "1,\"Doe, John\",\"A person with a\n"
        "multi-line description.\"\n",
        // 3. Escaped quotes and empty fields
        "key,value,,\n"
        "quote,\"He said, \"\"Hello!\"\"\",empty1,\n"
        "end,,,\n",
        // 4. Malformed CSV (unterminated quote) - should fail parsing
        "a,b,\"c,d",
        // 5. Empty file
        ""
    };
    int num_tests = sizeof(filenames) / sizeof(filenames[0]);

    for (int i = 0; i < num_tests; i++) {
        printf(">>> Running Test Case %d ('%s') <<<\n", i + 1, filenames[i]);
        if (create_test_file(filenames[i], contents[i]) != 0) {
            continue;
        }

        CsvData* data = read_and_parse_csv(filenames[i]);
        
        if (data) {
            print_csv_data(data);
            free_csv_data(data);
        } else {
            printf("Failed to parse CSV file '%s' (this may be expected).\n\n", filenames[i]);
        }
        
        remove(filenames[i]);
    }

    return 0;
}