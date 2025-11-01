#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

static char* base64_encode(const unsigned char* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i += 3) {
        size_t rem = len - i;
        unsigned int b0 = data[i];
        unsigned int b1 = rem > 1 ? data[i + 1] : 0;
        unsigned int b2 = rem > 2 ? data[i + 2] : 0;
        unsigned int val = (b0 << 16) | (b1 << 8) | b2;

        out[j++] = tbl[(val >> 18) & 0x3F];
        out[j++] = tbl[(val >> 12) & 0x3F];
        out[j++] = (rem > 1) ? tbl[(val >> 6) & 0x3F] : '=';
        out[j++] = (rem > 2) ? tbl[val & 0x3F] : '=';
    }
    out[j] = '\0';
    return out;
}

static int read_file(const char* path, unsigned char** out_buf, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return 0; }
    rewind(f);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return 0; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { free(buf); return 0; }
    *out_buf = buf;
    *out_len = (size_t)sz;
    return 1;
}

long long upload_image(const char* db_path, const char* image_path, const char* image_name) {
    unsigned char* data = NULL;
    size_t data_len = 0;
    if (!read_file(image_path, &data, &data_len)) {
        return -1;
    }

    char* b64 = base64_encode(data, data_len);
    free(data);
    if (!b64) {
        return -1;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        sqlite3_close(db);
        free(b64);
        return -1;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS images ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT NOT NULL,"
        " data_base64 TEXT NOT NULL"
        ");";
    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        free(b64);
        return -1;
    }

    const char* insert_sql = "INSERT INTO images(name, data_base64) VALUES (?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        free(b64);
        return -1;
    }

    if (sqlite3_bind_text(stmt, 1, image_name, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, b64, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        free(b64);
        return -1;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        free(b64);
        return -1;
    }
    sqlite3_finalize(stmt);
    free(b64);

    long long rowid = (long long)sqlite3_last_insert_rowid(db);
    sqlite3_close(db);
    return rowid;
}

static void write_dummy(const char* path, size_t size, unsigned int seed) {
    srand(seed);
    unsigned char* buf = (unsigned char*)malloc(size);
    if (!buf) return;
    for (size_t i = 0; i < size; ++i) buf[i] = (unsigned char)(rand() % 256);
    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(buf, 1, size, f);
        fclose(f);
    }
    free(buf);
}

int main(void) {
    const char* db_path = "c_images.db";
    for (int i = 1; i <= 5; ++i) {
        char img_path[64];
        snprintf(img_path, sizeof(img_path), "c_img_%d.bin", i);
        write_dummy(img_path, (size_t)(23 + i), (unsigned int)(400 + i));
        char name[64];
        snprintf(name, sizeof(name), "c_image_%d", i);
        long long id = upload_image(db_path, img_path, name);
        printf("C inserted row id: %lld\n", id);
    }
    return 0;
}