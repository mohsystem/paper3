
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define MAX_FILENAME 256

/* Base64 encoding lookup table */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/* Base64 encode function with bounds checking */
char* base64_encode(const unsigned char* data, size_t len) {
    if (!data || len == 0) {
        return NULL;
    }
    
    /* Calculate output size and check for overflow */
    size_t output_len = ((len + 2) / 3) * 4;
    if (output_len < len) {
        return NULL; /* Overflow check */
    }
    
    char* result = (char*)calloc(output_len + 1, 1);
    if (!result) {
        return NULL;
    }
    
    size_t i = 0, j = 0;
    while (i < len) {
        unsigned char byte1 = data[i++];
        unsigned char byte2 = (i < len) ? data[i++] : 0;
        unsigned char byte3 = (i < len) ? data[i++] : 0;
        
        result[j++] = base64_chars[byte1 >> 2];
        result[j++] = base64_chars[((byte1 & 0x03) << 4) | (byte2 >> 4)];
        result[j++] = (i > len + 1) ? '=' : base64_chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)];
        result[j++] = (i > len) ? '=' : base64_chars[byte3 & 0x3F];
    }
    
    result[j] = '\\0';
    return result;
}

/* Validates file is actually an image by checking magic bytes */
int is_valid_image_file(const unsigned char* data, size_t len) {
    if (!data || len < 4) {
        return 0;
    }
    
    /* Check JPEG magic bytes */
    if (len >= 2 && data[0] == 0xFF && data[1] == 0xD8) {
        return 1;
    }
    /* Check PNG magic bytes */
    if (len >= 8 && data[0] == 0x89 && data[1] == 0x50 && 
        data[2] == 0x4E && data[3] == 0x47) {
        return 1;
    }
    /* Check GIF magic bytes */
    if (len >= 6 && data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46) {
        return 1;
    }
    
    return 0;
}

/* Extract basename and sanitize filename */
void get_sanitized_filename(const char* filename, char* output, size_t output_size) {
    if (!filename || !output || output_size == 0) {
        return;
    }
    
    /* Find last path separator */
    const char* basename = strrchr(filename, '/');
    if (!basename) {
        basename = strrchr(filename, '\\\\');\n    }\n    basename = basename ? basename + 1 : filename;\n    \n    /* Copy with bounds check and null termination */\n    size_t len = strlen(basename);\n    if (len >= output_size) {\n        len = output_size - 1;\n    }\n    memcpy(output, basename, len);\n    output[len] = '\\0';\n}\n\n/* Validates file extension */\nint has_valid_extension(const char* filename) {\n    if (!filename) {\n        return 0;\n    }\n    \n    const char* ext = strrchr(filename, '.');
    if (!ext) {
        return 0;
    }
    ext++;
    
    /* Case-insensitive comparison */
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 ||
        strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0) {
        return 1;
    }
    
    return 0;
}

const char* upload_image(const char* filename, const unsigned char* image_data, size_t data_len) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    char* base64_image = NULL;
    char sanitized_filename[MAX_FILENAME] = {0};
    const char* result = "Error: Unknown error";
    
    /* Input validation: check filename */
    if (!filename || strlen(filename) == 0) {
        return "Error: Invalid filename";
    }
    
    /* Input validation: check image data */
    if (!image_data || data_len == 0) {
        return "Error: No image data provided";
    }
    
    /* Validate file size to prevent DOS attacks */
    if (data_len > MAX_FILE_SIZE) {
        return "Error: File size exceeds limit";
    }
    
    /* Sanitize filename to prevent path traversal */
    get_sanitized_filename(filename, sanitized_filename, sizeof(sanitized_filename));
    
    /* Validate file extension */
    if (!has_valid_extension(sanitized_filename)) {
        return "Error: Invalid file extension";
    }
    
    /* Validate actual file content by magic bytes */
    if (!is_valid_image_file(image_data, data_len)) {
        return "Error: Invalid image format";
    }
    
    /* Convert to base64 */
    base64_image = base64_encode(image_data, data_len);
    if (!base64_image) {
        return "Error: Failed to encode image";
    }
    
    /* Get database path from environment variable */
    const char* db_path = getenv("DB_PATH");
    if (!db_path) {
        free(base64_image);
        return "Error: Database not configured";
    }
    
    /* Open database connection */
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        free(base64_image);
        return "Error: Database connection failed";
    }
    
    /* Prepare parameterized statement to prevent SQL injection */
    const char* sql = "INSERT INTO images (name, base64_data) VALUES (?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        result = "Error: Failed to prepare statement";
        goto cleanup;
    }
    
    /* Bind parameters safely */
    sqlite3_bind_text(stmt, 1, sanitized_filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, base64_image, -1, SQLITE_TRANSIENT);
    
    /* Execute statement */
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = "Success: Image uploaded";
    } else {
        result = "Error: Database operation failed";
    }
    
cleanup:
    if (stmt) {
        sqlite3_finalize(stmt);
    }
    if (db) {
        sqlite3_close(db);
    }
    if (base64_image) {
        /* Clear sensitive data before freeing */
        memset(base64_image, 0, strlen(base64_image));
        free(base64_image);
    }
    
    return result;
}

int main(void) {
    /* Test case 1: Valid JPEG image */
    unsigned char valid_jpeg[100] = {0};
    valid_jpeg[0] = 0xFF;
    valid_jpeg[1] = 0xD8;
    printf("Test 1: %s\\n", upload_image("test1.jpg", valid_jpeg, sizeof(valid_jpeg)));
    
    /* Test case 2: Valid PNG image */
    unsigned char valid_png[100] = {0};
    valid_png[0] = 0x89;
    valid_png[1] = 0x50;
    valid_png[2] = 0x4E;
    valid_png[3] = 0x47;
    printf("Test 2: %s\\n", upload_image("test2.png", valid_png, sizeof(valid_png)));
    
    /* Test case 3: Invalid extension */
    printf("Test 3: %s\\n", upload_image("test3.exe", valid_jpeg, sizeof(valid_jpeg)));
    
    /* Test case 4: Path traversal attempt */
    printf("Test 4: %s\\n", upload_image("../../etc/passwd.jpg", valid_jpeg, sizeof(valid_jpeg)));
    
    /* Test case 5: NULL input */
    printf("Test 5: %s\\n", upload_image(NULL, NULL, 0));
    
    return 0;
}
