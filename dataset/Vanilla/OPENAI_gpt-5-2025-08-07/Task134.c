#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

typedef struct {
    char** aliases;
    unsigned char** keys;
    size_t* key_lens;
    size_t count;
    size_t capacity;
} KeyManager;

/* Randomness */
static void fill_random(unsigned char* buf, size_t len) {
#ifdef _WIN32
    BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(buf, 1, len, f);
        fclose(f);
        return;
    }
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    for (size_t i = 0; i < len; ++i) buf[i] = (unsigned char)(rand() & 0xFF);
#endif
}

/* Base64 */
static const char* B64_TBL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char* b64_encode(const unsigned char* data, size_t len) {
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out[j++] = B64_TBL[(n >> 18) & 63];
        out[j++] = B64_TBL[(n >> 12) & 63];
        out[j++] = B64_TBL[(n >> 6) & 63];
        out[j++] = B64_TBL[n & 63];
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        out[j++] = B64_TBL[(n >> 18) & 63];
        if (i + 1 < len) {
            n |= data[i + 1] << 8;
            out[j++] = B64_TBL[(n >> 12) & 63];
            out[j++] = B64_TBL[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = B64_TBL[(n >> 12) & 63];
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

static int b64_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static unsigned char* b64_decode(const char* in, size_t* out_len) {
    size_t len = 0;
    for (size_t i = 0; in[i]; ++i) {
        char c = in[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '+' || c == '/' || c == '=') {
            len++;
        }
    }
    if (len % 4 != 0) return NULL;

    char* clean = (char*)malloc(len + 1);
    if (!clean) return NULL;
    size_t idx = 0;
    for (size_t i = 0; in[i]; ++i) {
        char c = in[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '+' || c == '/' || c == '=') {
            clean[idx++] = c;
        }
    }
    clean[idx] = '\0';

    size_t padding = 0;
    if (idx >= 2) {
        if (clean[idx - 1] == '=') padding++;
        if (clean[idx - 2] == '=') padding++;
    }
    size_t olen = (idx / 4) * 3 - padding;
    unsigned char* out = (unsigned char*)malloc(olen);
    if (!out) { free(clean); return NULL; }

    size_t j = 0;
    for (size_t i = 0; i < idx; i += 4) {
        int a = clean[i] == '=' ? 0 : b64_index(clean[i]);
        int b = clean[i + 1] == '=' ? 0 : b64_index(clean[i + 1]);
        int c = clean[i + 2] == '=' ? 0 : b64_index(clean[i + 2]);
        int d = clean[i + 3] == '=' ? 0 : b64_index(clean[i + 3]);
        if (a < 0 || b < 0 || c < 0 || d < 0) { free(clean); free(out); return NULL; }
        unsigned int n = (a << 18) | (b << 12) | (c << 6) | d;
        if (j < olen) out[j++] = (n >> 16) & 0xFF;
        if (j < olen) out[j++] = (n >> 8) & 0xFF;
        if (j < olen) out[j++] = n & 0xFF;
    }
    free(clean);
    *out_len = olen;
    return out;
}

/* KeyManager helpers */
static void km_init(KeyManager* km) {
    km->aliases = NULL;
    km->keys = NULL;
    km->key_lens = NULL;
    km->count = 0;
    km->capacity = 0;
}

static void km_free(KeyManager* km) {
    for (size_t i = 0; i < km->count; ++i) {
        free(km->aliases[i]);
        free(km->keys[i]);
    }
    free(km->aliases);
    free(km->keys);
    free(km->key_lens);
    km->aliases = NULL;
    km->keys = NULL;
    km->key_lens = NULL;
    km->count = 0;
    km->capacity = 0;
}

static ssize_t km_find(KeyManager* km, const char* alias) {
    for (size_t i = 0; i < km->count; ++i) {
        if (strcmp(km->aliases[i], alias) == 0) return (ssize_t)i;
    }
    return -1;
}

static int km_ensure_capacity(KeyManager* km, size_t need) {
    if (km->capacity >= need) return 1;
    size_t newcap = km->capacity == 0 ? 4 : km->capacity * 2;
    while (newcap < need) newcap *= 2;
    char** na = (char**)realloc(km->aliases, newcap * sizeof(char*));
    unsigned char** nk = (unsigned char**)realloc(km->keys, newcap * sizeof(unsigned char*));
    size_t* nl = (size_t*)realloc(km->key_lens, newcap * sizeof(size_t));
    if (!na || !nk || !nl) return 0;
    km->aliases = na; km->keys = nk; km->key_lens = nl; km->capacity = newcap;
    return 1;
}

/* API */
int generate_key(KeyManager* km, const char* alias, size_t lengthBytes) {
    if (!alias || !*alias || lengthBytes == 0) return 0;
    if (km_find(km, alias) >= 0) return 0;
    if (!km_ensure_capacity(km, km->count + 1)) return 0;
    unsigned char* key = (unsigned char*)malloc(lengthBytes);
    if (!key) return 0;
    fill_random(key, lengthBytes);
    km->aliases[km->count] = strdup(alias);
    km->keys[km->count] = key;
    km->key_lens[km->count] = lengthBytes;
    km->count++;
    return 1;
}

char* export_key_base64(KeyManager* km, const char* alias) {
    ssize_t idx = km_find(km, alias);
    if (idx < 0) return NULL;
    return b64_encode(km->keys[idx], km->key_lens[idx]);
}

int import_key_base64(KeyManager* km, const char* alias, const char* b64, int overwrite) {
    if (!alias || !*alias || !b64) return 0;
    ssize_t idx = km_find(km, alias);
    if (idx >= 0 && !overwrite) return 0;
    size_t outlen = 0;
    unsigned char* raw = b64_decode(b64, &outlen);
    if (!raw || outlen == 0) { free(raw); return 0; }

    if (idx >= 0) {
        free(km->keys[idx]);
        km->keys[idx] = raw;
        km->key_lens[idx] = outlen;
        return 1;
    } else {
        if (!km_ensure_capacity(km, km->count + 1)) { free(raw); return 0; }
        km->aliases[km->count] = strdup(alias);
        km->keys[km->count] = raw;
        km->key_lens[km->count] = outlen;
        km->count++;
        return 1;
    }
}

char* rotate_key(KeyManager* km, const char* alias, size_t newLengthBytes) {
    if (newLengthBytes == 0) return NULL;
    ssize_t idx = km_find(km, alias);
    if (idx < 0) return NULL;
    unsigned char* key = (unsigned char*)malloc(newLengthBytes);
    if (!key) return NULL;
    fill_random(key, newLengthBytes);
    free(km->keys[idx]);
    km->keys[idx] = key;
    km->key_lens[idx] = newLengthBytes;
    return b64_encode(km->keys[idx], km->key_lens[idx]);
}

int delete_key(KeyManager* km, const char* alias) {
    ssize_t idx = km_find(km, alias);
    if (idx < 0) return 0;
    free(km->aliases[idx]);
    free(km->keys[idx]);
    for (size_t i = (size_t)idx + 1; i < km->count; ++i) {
        km->aliases[i - 1] = km->aliases[i];
        km->keys[i - 1] = km->keys[i];
        km->key_lens[i - 1] = km->key_lens[i];
    }
    km->count--;
    return 1;
}

char* list_aliases(KeyManager* km) {
    size_t total = 0;
    for (size_t i = 0; i < km->count; ++i) total += strlen(km->aliases[i]) + 1;
    char* out = (char*)malloc((total ? total : 1) + 1);
    if (!out) return NULL;
    out[0] = '\0';
    for (size_t i = 0; i < km->count; ++i) {
        strcat(out, km->aliases[i]);
        if (i + 1 < km->count) strcat(out, ",");
    }
    return out;
}

int get_key_length(KeyManager* km, const char* alias) {
    ssize_t idx = km_find(km, alias);
    if (idx < 0) return -1;
    return (int)km->key_lens[idx];
}

/* Tests */
int main(void) {
    KeyManager km; km_init(&km);

    /* Test 1 */
    int t1gen = generate_key(&km, "api", 32);
    char* t1exp = export_key_base64(&km, "api");
    printf("T1 gen=%d len=%d b64prefix=%s\n", t1gen, get_key_length(&km, "api"), t1exp ? (strndup(t1exp, 10)) : "null");
    if (t1exp) free(t1exp);

    /* Test 2 */
    int t2gen = generate_key(&km, "db", 16);
    char* aliases = list_aliases(&km);
    printf("T2 gen=%d aliases=%s\n", t2gen, aliases ? aliases : "");
    free(aliases);

    /* Test 3 */
    char* exported = export_key_base64(&km, "api");
    int t3imp = import_key_base64(&km, "imported", exported ? exported : "", 0);
    printf("T3 import=%d importedLen=%d\n", t3imp, get_key_length(&km, "imported"));
    if (exported) free(exported);

    /* Test 4 */
    char* t4rot = rotate_key(&km, "api", 24);
    char prefix[11]; prefix[10] = '\0';
    if (t4rot) { strncpy(prefix, t4rot, 10); } else { strcpy(prefix, "null"); }
    printf("T4 rotatedLen=%d rotB64prefix=%s\n", get_key_length(&km, "api"), prefix);
    if (t4rot) free(t4rot);

    /* Test 5 */
    int t5del = delete_key(&km, "db");
    aliases = list_aliases(&km);
    printf("T5 delete=%d aliases=%s\n", t5del, aliases ? aliases : "");
    free(aliases);

    km_free(&km);
    return 0;
}