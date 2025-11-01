
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <ctype.h>

#define DB_PATH "images.db"
#define MAX_FILE_SIZE (5 * 1024 * 1024)

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1);
    
    if (encoded_data == NULL) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }
    
    int mod_table[] = {0, 2, 1};
    for (i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
    
    encoded_data[output_length] = '\\0';
    return encoded_data;
}

int has_valid_extension(const char* filename) {
    if (filename == NULL) return 0;
    
    size_t len = strlen(filename);
    if (len < 4) return 0;
    
    char lower[256];
    for (size_t i = 0; i < len && i < 255; i++) {
        lower[i] = tolower(filename[i]);
    }
    lower[len] = '\\0';
    
    return (strstr(lower, ".jpg") != NULL || 
            strstr(lower, ".jpeg") != NULL || 
            strstr(lower, ".png") != NULL || 
            strstr(lower, ".gif") != NULL);
}

const char* get_filename(const char* path) {
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\\\');
    const char* filename = path;
    
    if (last_slash != NULL) filename = last_slash + 1;
    if (last_backslash != NULL && last_backslash > filename) filename = last_backslash + 1;
    
    return filename;
}

void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS images ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "image_data TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

int upload_image(const char* image_path) {
    if (image_path == NULL || strlen(image_path) == 0) {
        fprintf(stderr, "Invalid image path\\n");
        return 0;
    }
    
    struct stat st;
    if (stat(image_path, &st) != 0) {
        fprintf(stderr, "File does not exist\\n");
        return 0;
    }
    
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File size exceeds 5MB limit\\n");
        return 0;
    }
    
    if (!has_valid_extension(image_path)) {
        fprintf(stderr, "Invalid file type. Only JPG, PNG, and GIF allowed\\n");
        return 0;
    }
    
    FILE* file = fopen(image_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\\n");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* buffer = (unsigned char*)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed\\n");
        return 0;
    }
    
    fread(buffer, 1, file_size, file);
    fclose(file);
    
    char* base64_image = base64_encode(buffer, file_size);
    free(buffer);
    
    if (base64_image == NULL) {
        fprintf(stderr, "Base64 encoding failed\\n");
        return 0;
    }
    
    sqlite3* db;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        free(base64_image);
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO images (name, image_data) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(base64_image);
        return 0;
    }
    
    const char* filename = get_filename(image_path);
    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, base64_image, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(base64_image);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed\\n");
        return 0;
    }
    
    printf("Image uploaded successfully: %s\\n", filename);
    return 1;
}

int main() {
    init_database();
    
    printf("Test Case 1: Valid image (create a test.jpg file first)\\n");
    upload_image("test.jpg");
    
    printf("\\nTest Case 2: NULL path\\n");
    upload_image(NULL);
    
    printf("\\nTest Case 3: Empty path\\n");
    upload_image("");
    
    printf("\\nTest Case 4: Non-existent file\\n");
    upload_image("nonexistent.jpg");
    
    printf("\\nTest Case 5: Invalid file type\\n");
    upload_image("test.txt");
    
    return 0;
}
