#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <unistd.h>
  #if defined(__linux__)
    #include <sys/random.h>
  #endif
  #include <fcntl.h>
  #include <errno.h>
#endif

#define MAX_KEYS 128
#define ID_LEN 32
#define ALG_NAME_LEN 8

typedef struct {
    char id[ID_LEN + 1];
    char algorithm[ALG_NAME_LEN]; // "AES"
    int bits;                     // 128/192/256
    int exportable;               // boolean
    time_t createdAt;
    uint8_t* keyMaterial;
    size_t keyLen;
    int in_use;
} KeyRecord;

typedef struct {
    KeyRecord store[MAX_KEYS];
} KeyManager;

// secure random
static int secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
  #if defined(__linux__)
    ssize_t n = getrandom(buf, len, 0);
    if (n == (ssize_t)len) return 1;
  #endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static void secure_zero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

static void hex_encode(const uint8_t* in, size_t len, char* out, size_t outsz) {
    static const char* hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < len && j + 2 < outsz; ++i) {
        out[j++] = hex[(in[i] >> 4) & 0xF];
        out[j++] = hex[in[i] & 0xF];
    }
    if (j < outsz) out[j] = '\0';
}

static int base64_encode(const uint8_t* data, size_t len, char** out_str) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return 0;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        uint32_t v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(v >> 18) & 0x3F];
        out[j++] = tbl[(v >> 12) & 0x3F];
        out[j++] = tbl[(v >> 6) & 0x3F];
        out[j++] = tbl[v & 0x3F];
        i += 3;
    }
    if (i < len) {
        uint32_t v = data[i] << 16;
        out[j++] = tbl[(v >> 18) & 0x3F];
        if (i + 1 < len) {
            v |= data[i+1] << 8;
            out[j++] = tbl[(v >> 12) & 0x3F];
            out[j++] = tbl[(v >> 6) & 0x3F];
            out[j++] = '=';
        } else {
            out[j++] = tbl[(v >> 12) & 0x3F];
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    *out_str = out;
    return 1;
}

static void km_init(KeyManager* km) {
    memset(km, 0, sizeof(*km));
}

static int km_find_free_slot(KeyManager* km) {
    for (int i = 0; i < MAX_KEYS; ++i) if (!km->store[i].in_use) return i;
    return -1;
}

static int km_find_index(KeyManager* km, const char* id) {
    for (int i = 0; i < MAX_KEYS; ++i) {
        if (km->store[i].in_use && strncmp(km->store[i].id, id, ID_LEN) == 0) return i;
    }
    return -1;
}

static int create_key(KeyManager* km, const char* algorithm, int bits, int exportable, char* out_id, size_t out_id_sz) {
    if (!algorithm || (strcmp(algorithm, "AES") != 0 && strcmp(algorithm, "aes") != 0)) return 0;
    if (!(bits == 128 || bits == 192 || bits == 256)) return 0;
    int idx = km_find_free_slot(km);
    if (idx < 0) return 0;

    uint8_t rid[16];
    if (!secure_random_bytes(rid, sizeof(rid))) return 0;
    char idhex[ID_LEN + 1] = {0};
    hex_encode(rid, sizeof(rid), idhex, sizeof(idhex));

    size_t keyLen = (size_t)bits / 8;
    uint8_t* mat = (uint8_t*)malloc(keyLen);
    if (!mat) return 0;
    if (!secure_random_bytes(mat, keyLen)) {
        secure_zero(mat, keyLen);
        free(mat);
        return 0;
    }

    KeyRecord* r = &km->store[idx];
    memset(r, 0, sizeof(*r));
    strncpy(r->id, idhex, ID_LEN);
    strncpy(r->algorithm, "AES", ALG_NAME_LEN - 1);
    r->bits = bits;
    r->exportable = exportable ? 1 : 0;
    r->createdAt = time(NULL);
    r->keyMaterial = mat;
    r->keyLen = keyLen;
    r->in_use = 1;

    if (out_id && out_id_sz > 0) {
        strncpy(out_id, r->id, out_id_sz - 1);
        out_id[out_id_sz - 1] = '\0';
    }
    return 1;
}

static int get_key_info(KeyManager* km, const char* id, char* out_buf, size_t out_sz) {
    int idx = km_find_index(km, id);
    if (idx < 0) return 0;
    KeyRecord* r = &km->store[idx];
    int n = snprintf(out_buf, out_sz,
                     "{\"id\":\"%s\",\"algorithm\":\"%s\",\"bits\":%d,\"exportable\":%s,\"createdAt\":%ld}",
                     r->id, r->algorithm, r->bits, r->exportable ? "true" : "false", (long)r->createdAt);
    return (n > 0 && (size_t)n < out_sz) ? 1 : 0;
}

static int list_keys(KeyManager* km, char* out_buf, size_t out_sz) {
    // Return comma-separated IDs
    size_t off = 0;
    for (int i = 0; i < MAX_KEYS; ++i) {
        if (km->store[i].in_use) {
            size_t idlen = strnlen(km->store[i].id, ID_LEN);
            if (off + idlen + 1 >= out_sz) break;
            if (off > 0) out_buf[off++] = ',';
            memcpy(out_buf + off, km->store[i].id, idlen);
            off += idlen;
        }
    }
    if (off < out_sz) out_buf[off] = '\0';
    return 1;
}

static int delete_key(KeyManager* km, const char* id) {
    int idx = km_find_index(km, id);
    if (idx < 0) return 0;
    KeyRecord* r = &km->store[idx];
    if (r->keyMaterial && r->keyLen) {
        secure_zero(r->keyMaterial, r->keyLen);
        free(r->keyMaterial);
    }
    memset(r, 0, sizeof(*r));
    r->in_use = 0;
    return 1;
}

static int rotate_key(KeyManager* km, const char* id) {
    int idx = km_find_index(km, id);
    if (idx < 0) return 0;
    KeyRecord* r = &km->store[idx];
    uint8_t* newMat = (uint8_t*)malloc(r->keyLen);
    if (!newMat) return 0;
    if (!secure_random_bytes(newMat, r->keyLen)) {
        secure_zero(newMat, r->keyLen);
        free(newMat);
        return 0;
    }
    if (r->keyMaterial && r->keyLen) {
        secure_zero(r->keyMaterial, r->keyLen);
        free(r->keyMaterial);
    }
    r->keyMaterial = newMat;
    return 1;
}

static int export_key_base64(KeyManager* km, const char* id, char** out_b64) {
    int idx = km_find_index(km, id);
    if (idx < 0) return 0;
    KeyRecord* r = &km->store[idx];
    if (!r->exportable) return 0;
    return base64_encode(r->keyMaterial, r->keyLen, out_b64);
}

// Test functions demonstrating usage (5 test cases)
int main(void) {
    KeyManager km;
    km_init(&km);

    char id1[ID_LEN + 1] = {0};
    char id2[ID_LEN + 1] = {0};

    // Test case 1: Create exportable AES-256 key
    if (!create_key(&km, "AES", 256, 1, id1, sizeof(id1))) {
        printf("Test1 failed to create key\n");
        return 1;
    }
    char info1[256];
    get_key_info(&km, id1, info1, sizeof(info1));
    printf("Test1 id1: %s\n", id1);
    printf("Info1: %s\n", info1);

    // Test case 2: Create non-exportable AES-128 key
    if (!create_key(&km, "AES", 128, 0, id2, sizeof(id2))) {
        printf("Test2 failed to create key\n");
        return 1;
    }
    char info2[256];
    get_key_info(&km, id2, info2, sizeof(info2));
    printf("Test2 id2: %s\n", id2);
    printf("Info2: %s\n", info2);

    // Test case 3: List keys
    char listbuf[4096];
    list_keys(&km, listbuf, sizeof(listbuf));
    printf("Test3 list: %s\n", listbuf);

    // Test case 4: Export id1 and attempt export id2
    char* exp1 = NULL;
    int ok1 = export_key_base64(&km, id1, &exp1);
    printf("Test4a export id1 length: %d\n", ok1 ? (int)strlen(exp1) : -1);
    if (exp1) {
        // Do not print the key; just free it
        secure_zero(exp1, strlen(exp1));
        free(exp1);
    }
    char* exp2 = NULL;
    int ok2 = export_key_base64(&km, id2, &exp2);
    printf("Test4b export id2 should be 0: %d\n", ok2);
    if (exp2) {
        secure_zero(exp2, strlen(exp2));
        free(exp2);
    }

    // Test case 5: Rotate id1 and delete id2
    int rotated = rotate_key(&km, id1);
    printf("Test5a rotate id1: %s\n", rotated ? "true" : "false");
    int deleted = delete_key(&km, id2);
    printf("Test5b delete id2: %s\n", deleted ? "true" : "false");
    char info2b[256];
    int okinfo = get_key_info(&km, id2, info2b, sizeof(info2b));
    printf("Info2 after delete (should be 0): %d\n", okinfo);

    // Cleanup remaining keys
    delete_key(&km, id1);

    return 0;
}