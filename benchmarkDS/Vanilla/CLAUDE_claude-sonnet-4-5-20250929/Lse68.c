
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(output_length + 1);
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
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++)
        encoded_data[output_length - 1 - i] = '=';
    
    encoded_data[output_length] = '\\0';
    return encoded_data;
}

const char* get_filename(const char* path) {
    const char* filename = strrchr(path, '/');
    if (!filename) filename = strrchr(path, '\\\\');
    return filename ? filename + 1 : path;
}

void init_database() {
    sqlite3* db;
    char* err_msg = 0;
    
    int rc = sqlite3_open("images.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS images ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "image_data TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

int upload_image(const char* image_path) {
    FILE* file = fopen(image_path, "rb");
    if (!file) {
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* image_data = malloc(file_size);
    if (!image_data) {
        fclose(file);
        return 0;
    }
    
    fread(image_data, 1, file_size, file);
    fclose(file);
    
    char* base64_image = base64_encode(image_data, file_size);
    free(image_data);
    
    if (!base64_image) {
        return 0;
    }
    
    const char* image_name = get_filename(image_path);
    
    sqlite3* db;
    int rc = sqlite3_open("images.db", &db);
    if (rc != SQLITE_OK) {
        free(base64_image);
        return 0;
    }
    
    const char* sql = "INSERT INTO images (name, image_data) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        free(base64_image);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, image_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, base64_image, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(base64_image);
    
    return rc == SQLITE_DONE ? 1 : 0;
}

int main() {
    init_database();
    
    printf("Test 1: %d\\n", upload_image("test_image1.jpg"));
    printf("Test 2: %d\\n", upload_image("test_image2.png"));
    printf("Test 3: %d\\n", upload_image("non_existent.jpg"));
    printf("Test 4: %d\\n", upload_image("images/photo.jpg"));
    printf("Test 5: %d\\n", upload_image("sample.bmp"));
    
    return 0;
}
