#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

/*
High-level notes:
- In-memory key manager with secure RNG from OS.
- Simple dynamic array map label->KeyRecord.
- Keys zeroized on delete/rotate. Caller copies returned key as needed.
*/

typedef struct {
    char* label;
    uint8_t* key;
    size_t key_len;
    int version;
} KeyRecord;

typedef struct {
    KeyRecord* items;
    size_t count;
    size_t cap;
} KeyManager;

static int secure_random_bytes(uint8_t* out, size_t len) {
#if defined(_WIN32)
    NTSTATUS st = BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return st == 0 ? 1 : 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, out + off, len - off);
        if (r <= 0) { close(fd); return 0; }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static void secure_zero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) { *p++ = 0; }
}

static void km_init(KeyManager* km) {
    km->items = NULL;
    km->count = 0;
    km->cap = 0;
}

static void km_free(KeyManager* km) {
    if (!km) return;
    for (size_t i = 0; i < km->count; i++) {
        if (km->items[i].key) {
            secure_zero(km->items[i].key, km->items[i].key_len);
            free(km->items[i].key);
        }
        free(km->items[i].label);
    }
    free(km->items);
    km->items = NULL;
    km->count = 0;
    km->cap = 0;
}

static int km_find_index(KeyManager* km, const char* label) {
    for (size_t i = 0; i < km->count; i++) {
        if (strcmp(km->items[i].label, label) == 0) return (int)i;
    }
    return -1;
}

static int km_ensure_cap(KeyManager* km) {
    if (km->count < km->cap) return 1;
    size_t ncap = km->cap == 0 ? 4 : km->cap * 2;
    KeyRecord* nitems = (KeyRecord*)realloc(km->items, ncap * sizeof(KeyRecord));
    if (!nitems) return 0;
    km->items = nitems;
    km->cap = ncap;
    return 1;
}

// Returns version (1 if created, existing version if already present). On error returns -1.
static int km_create_key(KeyManager* km, const char* label, size_t size_bytes) {
    if (!label || !*label || size_bytes < 16) return -1;
    int idx = km_find_index(km, label);
    if (idx >= 0) {
        return km->items[idx].version;
    }
    if (!km_ensure_cap(km)) return -1;
    uint8_t* key = (uint8_t*)malloc(size_bytes);
    if (!key) return -1;
    if (!secure_random_bytes(key, size_bytes)) { free(key); return -1; }
    KeyRecord rec;
    rec.label = _strdup(label);
#if !defined(_WIN32)
    if (!rec.label) rec.label = strdup(label);
#endif
    if (!rec.label) { secure_zero(key, size_bytes); free(key); return -1; }
    rec.key = key;
    rec.key_len = size_bytes;
    rec.version = 1;
    km->items[km->count++] = rec;
    return rec.version;
}

// Allocates and returns a copy of the key. Sets *out_len. Caller must free return pointer. Returns NULL if not found.
static uint8_t* km_get_key_copy(KeyManager* km, const char* label, size_t* out_len) {
    int idx = km_find_index(km, label);
    if (idx < 0) return NULL;
    KeyRecord* rec = &km->items[idx];
    if (!rec->key || rec->key_len == 0) return NULL;
    uint8_t* out = (uint8_t*)malloc(rec->key_len);
    if (!out) return NULL;
    memcpy(out, rec->key, rec->key_len);
    if (out_len) *out_len = rec->key_len;
    return out;
}

// Rotates key for label; returns new version or -1 on error
static int km_rotate_key(KeyManager* km, const char* label, size_t size_bytes) {
    if (size_bytes < 16) return -1;
    int idx = km_find_index(km, label);
    if (idx < 0) return -1;
    KeyRecord* rec = &km->items[idx];
    uint8_t* nkey = (uint8_t*)malloc(size_bytes);
    if (!nkey) return -1;
    if (!secure_random_bytes(nkey, size_bytes)) { free(nkey); return -1; }
    if (rec->key) {
        secure_zero(rec->key, rec->key_len);
        free(rec->key);
    }
    rec->key = nkey;
    rec->key_len = size_bytes;
    rec->version += 1;
    return rec->version;
}

// Deletes key; returns 1 if deleted else 0
static int km_delete_key(KeyManager* km, const char* label) {
    int idx = km_find_index(km, label);
    if (idx < 0) return 0;
    KeyRecord* rec = &km->items[idx];
    if (rec->key) {
        secure_zero(rec->key, rec->key_len);
        free(rec->key);
    }
    free(rec->label);
    // move last into idx
    if (idx != (int)(km->count - 1)) {
        km->items[idx] = km->items[km->count - 1];
    }
    km->count -= 1;
    return 1;
}

// Returns array of strings "label:vX". Sets *out_count. Caller must free each string and the array.
static char** km_list_keys(KeyManager* km, size_t* out_count) {
    if (out_count) *out_count = km->count;
    if (km->count == 0) return NULL;
    char** arr = (char**)calloc(km->count, sizeof(char*));
    if (!arr) return NULL;
    for (size_t i = 0; i < km->count; i++) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s:v%d", km->items[i].label, km->items[i].version);
        arr[i] = _strdup(buf);
#if !defined(_WIN32)
        if (!arr[i]) arr[i] = strdup(buf);
#endif
        if (!arr[i]) {
            for (size_t j = 0; j < i; j++) free(arr[j]);
            free(arr);
            return NULL;
        }
    }
    return arr;
}

static void print_keys(KeyManager* km) {
    size_t n = 0;
    char** list = km_list_keys(km, &n);
    printf("[");
    for (size_t i = 0; i < n; i++) {
        printf("%s", list[i]);
        if (i + 1 < n) printf(", ");
        free(list[i]);
    }
    free(list);
    printf("]");
}

int main(void) {
    KeyManager km;
    km_init(&km);

    // Test 1: create 'alpha' and list
    int v1 = km_create_key(&km, "alpha", 32);
    printf("Created alpha v%d keys=", v1); print_keys(&km); printf("\n");

    // Test 2: get key copy
    size_t klen = 0;
    uint8_t* kcopy = km_get_key_copy(&km, "alpha", &klen);
    printf("alpha key len=%zu\n", kcopy ? klen : 0u);
    free(kcopy);

    // Test 3: rotate 'alpha'
    int v2 = km_rotate_key(&km, "alpha", 32);
    printf("Rotated alpha to v%d\n", v2);

    // Test 4: create 'beta' and list
    km_create_key(&km, "beta", 32);
    printf("Keys now="); print_keys(&km); printf("\n");

    // Test 5: delete 'alpha' and verify
    int del = km_delete_key(&km, "alpha");
    size_t out_len = 0;
    uint8_t* after = km_get_key_copy(&km, "alpha", &out_len);
    printf("Deleted alpha? %s exists=%s\n", del ? "true" : "false", after ? "yes" : "no");
    free(after);

    km_free(&km);
    return 0;
}