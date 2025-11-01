/* Chain-of-Through process in code generation:
1) Problem understanding: Implement upload_image to read image bytes, base64-encode, and insert into a database.
2) Security requirements: Validate image name, cap file size, safe file I/O, and avoid buffer overflows.
3) Secure coding generation: Careful allocation, bounds checks, and safe API choices.
4) Code review: Validate all inputs, check errors, and ensure memory is freed.
5) Secure code output: Final code reflecting mitigations. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Simple in-memory database */
typedef struct {
    char* name;
    char* b64;
} ImageRecord;

typedef struct {
    ImageRecord* records;
    size_t count;
    size_t capacity;
} SafeDatabase;

static void db_init(SafeDatabase* db) {
    db->records = NULL;
    db->count = 0;
    db->capacity = 0;
}

static void db_free(SafeDatabase* db) {
    if (!db) return;
    for (size_t i = 0; i < db->count; i++) {
        free(db->records[i].name);
        free(db->records[i].b64);
    }
    free(db->records);
    db->records = NULL;
    db->count = 0;
    db->capacity = 0;
}

static bool db_insert(SafeDatabase* db, const char* name, const char* b64) {
    if (!db || !name || !b64) return false;
    if (db->count == db->capacity) {
        size_t newcap = db->capacity == 0 ? 8 : db->capacity * 2;
        ImageRecord* nr = (ImageRecord*)realloc(db->records, newcap * sizeof(ImageRecord));
        if (!nr) return false;
        db->records = nr;
        db->capacity = newcap;
    }
    size_t nlen = strlen(name);
    size_t blen = strlen(b64);
    char* ncopy = (char*)malloc(nlen + 1);
    char* bcopy = (char*)malloc(blen + 1);
    if (!ncopy || !bcopy) {
        free(ncopy);
        free(bcopy);
        return false;
    }
    memcpy(ncopy, name, nlen + 1);
    memcpy(bcopy, b64, blen + 1);
    db->records[db->count].name = ncopy;
    db->records[db->count].b64 = bcopy;
    db->count++;
    return true;
}

static bool is_valid_name(const char* name) {
    if (!name) return false;
    size_t len = strlen(name);
    if (len < 1 || len > 100) return false;
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

/* Base64 encoding */
static char* base64_encode(const unsigned char* data, size_t input_length) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (!data || input_length == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t out_len = 4 * ((input_length + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i + 3 <= input_length) {
        uint32_t n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < input_length) {
        uint32_t n = 0;
        int rem = (int)(input_length - i);
        n |= data[i] << 16;
        if (rem == 2) n |= data[i+1] << 8;
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        if (rem == 2) {
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

static bool read_file_capped(const char* path, unsigned char** out_data, size_t* out_len, size_t max_bytes) {
    if (!path || !out_data || !out_len) return false;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return false; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return false; }
    size_t usz = (size_t)sz;
    if ((size_t)sz != usz) { fclose(f); return false; } // overflow check
    if (usz > max_bytes) { fclose(f); return false; }
    unsigned char* buf = (unsigned char*)malloc(usz ? usz : 1);
    if (!buf) { fclose(f); return false; }
    size_t rd = fread(buf, 1, usz, f);
    fclose(f);
    if (rd != usz) { free(buf); return false; }
    *out_data = buf;
    *out_len = usz;
    return true;
}

/* upload_image: reads an image from file_path, converts to Base64, inserts into db with image_name. Returns 1 on success, 0 on failure. */
int upload_image(SafeDatabase* db, const char* image_name, const char* file_path) {
    if (!db || !is_valid_name(image_name) || !file_path) return 0;
    const size_t MAX_BYTES = 5u * 1024u * 1024u; // 5 MiB
    unsigned char* data = NULL;
    size_t data_len = 0;
    if (!read_file_capped(file_path, &data, &data_len, MAX_BYTES)) {
        return 0;
    }
    char* b64 = base64_encode(data, data_len);
    free(data);
    if (!b64) return 0;
    bool ok = db_insert(db, image_name, b64);
    free(b64);
    return ok ? 1 : 0;
}

/* Helper to create a test file with deterministic content */
static int write_bytes_to_file(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    size_t wr = fwrite(data, 1, len, f);
    fclose(f);
    return wr == len ? 1 : 0;
}

int main(void) {
    SafeDatabase db;
    db_init(&db);

    const char* names[5] = {"img1.png", "photo-02.jpg", "scan_03.tif", "icon-4.bmp", "graph5.svg"};
    char paths[5][64];

    for (int i = 0; i < 5; i++) {
        size_t sz = 128u + (size_t)i * 64u;
        unsigned char* content = (unsigned char*)malloc(sz);
        if (!content) {
            db_free(&db);
            return 1;
        }
        for (size_t j = 0; j < sz; j++) {
            content[j] = (unsigned char)(((i * 37) + (int)j) & 0xFF);
        }
        snprintf(paths[i], sizeof(paths[i]), "c_test_%d.bin", i + 1);
        if (!write_bytes_to_file(paths[i], content, sz)) {
            free(content);
            db_free(&db);
            return 1;
        }
        free(content);
    }

    // 5 test cases
    for (int i = 0; i < 5; i++) {
        int ok = upload_image(&db, names[i], paths[i]);
        printf("Test %d inserted=%s\n", i + 1, ok ? "true" : "false");
    }

    printf("Total records: %zu\n", db.count);
    for (size_t i = 0; i < db.count; i++) {
        char preview[21];
        size_t blen = strlen(db.records[i].b64);
        size_t plen = blen < 20 ? blen : 20;
        memcpy(preview, db.records[i].b64, plen);
        preview[plen] = '\0';
        printf("Name=%s, b64_preview=%s...\n", db.records[i].name, preview);
    }

    db_free(&db);
    return 0;
}