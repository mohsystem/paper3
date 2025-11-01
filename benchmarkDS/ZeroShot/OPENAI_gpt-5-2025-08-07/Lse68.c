#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

#define MAX_IMAGE_SIZE (5 * 1024 * 1024) // 5 MB

static int is_valid_image_name(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static char* base64_encode(const unsigned char* data, size_t len) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (data == NULL || len == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    if (len > (SIZE_MAX / 4) * 3 - 2) { // overflow guard for 4*((len+2)/3)+1
        return NULL;
    }
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i + 2 < len) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        out[j++] = tbl[(n >> 18) & 63];
        if (i + 1 < len) {
            n |= (data[i + 1] << 8);
            out[j++] = tbl[(n >> 12) & 63];
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = tbl[(n >> 12) & 63];
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

static int init_db(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS images ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "data TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP)";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    return rc;
}

long long upload_image(sqlite3* db, const char* image_name, const unsigned char* image_bytes, size_t image_len) {
    if (!db) return -1;
    if (!is_valid_image_name(image_name)) return -1;
    if (!image_bytes || image_len == 0 || image_len > MAX_IMAGE_SIZE) return -1;

    char* b64 = base64_encode(image_bytes, image_len);
    if (!b64) return -1;

    const char* sql = "INSERT INTO images(name, data) VALUES(?, ?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    long long id = -1;

    if (rc == SQLITE_OK && stmt) {
        rc = sqlite3_bind_text(stmt, 1, image_name, -1, SQLITE_TRANSIENT);
        if (rc == SQLITE_OK) rc = sqlite3_bind_text(stmt, 2, b64, -1, SQLITE_TRANSIENT);

        if (rc == SQLITE_OK) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) {
                id = (long long)sqlite3_last_insert_rowid(db);
            }
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    free(b64);
    return id;
}

static unsigned char* repeat(unsigned char b, size_t count) {
    unsigned char* buf = (unsigned char*)malloc(count ? count : 1);
    if (!buf) return NULL;
    for (size_t i = 0; i < count; ++i) buf[i] = b;
    return buf;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Failed to open DB\n");
        return 1;
    }
    if (init_db(db) != SQLITE_OK) {
        fprintf(stderr, "Failed to init DB\n");
        sqlite3_close(db);
        return 1;
    }

    unsigned char img1[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char img2[] = {0xFF, 0xD8, 0xFF, 0xE0};
    unsigned char img3[] = {'G','I','F','8','9','a'};
    unsigned char* img4 = repeat(0xAB, 1024);
    unsigned char* img5 = repeat(0xCD, 2048);

    long long id1 = upload_image(db, "sample1.png", img1, sizeof img1);
    long long id2 = upload_image(db, "photo-02.jpg", img2, sizeof img2);
    long long id3 = upload_image(db, "icon.gif", img3, sizeof img3);
    long long id4 = upload_image(db, "large_block.bin", img4, img4 ? 1024 : 0);
    long long id5 = upload_image(db, "data_chunk.dat", img5, img5 ? 2048 : 0);

    printf("Inserted IDs: %lld, %lld, %lld, %lld, %lld\n", id1, id2, id3, id4, id5);

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM images", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int cnt = sqlite3_column_int(stmt, 0);
            printf("Total rows: %d\n", cnt);
        }
        sqlite3_finalize(stmt);
    }

    if (img4) free(img4);
    if (img5) free(img5);
    sqlite3_close(db);
    return 0;
}