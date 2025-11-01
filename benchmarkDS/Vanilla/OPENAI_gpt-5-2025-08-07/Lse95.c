#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define SALT_LEN 16
#define MAX_USERS 100
#define USERNAME_MAX 64
#define STORED_MAX 80  // 16 (hash) + 1 (:) + 32 (salt hex) + 1 (\0) = 50, reserve extra

typedef struct {
    char username[USERNAME_MAX];
    char stored[STORED_MAX]; // "hashhex:salthex"
} UserRecord;

static UserRecord DB[MAX_USERS];
static int DB_COUNT = 0;
static int RNG_SEEDED = 0;

static void get_random_bytes(unsigned char* out, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t r = read(fd, out, len);
        close(fd);
        if (r == (ssize_t)len) return;
    }
    if (!RNG_SEEDED) {
        srand((unsigned int)time(NULL));
        RNG_SEEDED = 1;
    }
    for (size_t i = 0; i < len; ++i) out[i] = (unsigned char)(rand() & 0xFF);
}

static void bytes_to_hex(const unsigned char* in, size_t len, char* out_hex) {
    static const char* HEX = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[2*i]   = HEX[(in[i] >> 4) & 0xF];
        out_hex[2*i+1] = HEX[in[i] & 0xF];
    }
    out_hex[2*len] = '\0';
}

static int hex_to_bytes(const char* hex, unsigned char* out_bytes, size_t out_len_expected) {
    size_t len = strlen(hex);
    if (len % 2 != 0) return 0;
    size_t out_len = len / 2;
    if (out_len > out_len_expected) return 0;
    for (size_t i = 0; i < out_len; ++i) {
        char h = hex[2*i];
        char l = hex[2*i + 1];
        int hi = (h >= '0' && h <= '9') ? h - '0' :
                 (h >= 'a' && h <= 'f') ? h - 'a' + 10 :
                 (h >= 'A' && h <= 'F') ? h - 'A' + 10 : -1;
        int lo = (l >= '0' && l <= '9') ? l - '0' :
                 (l >= 'a' && l <= 'f') ? l - 'a' + 10 :
                 (l >= 'A' && l <= 'F') ? l - 'A' + 10 : -1;
        if (hi < 0 || lo < 0) return 0;
        out_bytes[i] = (unsigned char)((hi << 4) | lo);
    }
    return (int)out_len;
}

// FNV-1a 64-bit over password bytes followed by salt bytes
static unsigned long long fnv1a64(const unsigned char* pass, size_t pass_len, const unsigned char* salt, size_t salt_len) {
    const unsigned long long FNV_OFFSET = 0xcbf29ce484222325ULL;
    const unsigned long long FNV_PRIME  = 0x100000001b3ULL;
    unsigned long long h = FNV_OFFSET;
    for (size_t i = 0; i < pass_len; ++i) {
        h ^= (unsigned long long)(pass[i]);
        h *= FNV_PRIME;
    }
    for (size_t i = 0; i < salt_len; ++i) {
        h ^= (unsigned long long)(salt[i]);
        h *= FNV_PRIME;
    }
    return h;
}

static void ull_to_hex16(unsigned long long v, char* out_hex16) {
    static const char* HEX = "0123456789abcdef";
    for (int i = 15; i >= 0; --i) {
        out_hex16[i] = HEX[v & 0xF];
        v >>= 4;
    }
    out_hex16[16] = '\0';
}

static int constant_time_eq(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

int register_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    // Check duplicate
    for (int i = 0; i < DB_COUNT; ++i) {
        if (strncmp(DB[i].username, username, USERNAME_MAX) == 0) return 0;
    }
    if (DB_COUNT >= MAX_USERS) return 0;
    unsigned char salt[SALT_LEN];
    get_random_bytes(salt, SALT_LEN);
    unsigned long long h = fnv1a64((const unsigned char*)password, strlen(password), salt, SALT_LEN);
    char hash_hex[17];
    ull_to_hex16(h, hash_hex);
    char salt_hex[SALT_LEN*2 + 1];
    bytes_to_hex(salt, SALT_LEN, salt_hex);
    // Store "hash:salt"
    snprintf(DB[DB_COUNT].username, USERNAME_MAX, "%s", username);
    snprintf(DB[DB_COUNT].stored, STORED_MAX, "%s:%s", hash_hex, salt_hex);
    DB_COUNT++;
    return 1;
}

int login_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    int idx = -1;
    for (int i = 0; i < DB_COUNT; ++i) {
        if (strncmp(DB[i].username, username, USERNAME_MAX) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return 0;
    char* stored = DB[idx].stored;
    char* colon = strchr(stored, ':');
    if (!colon) return 0;
    size_t hash_len = (size_t)(colon - stored);
    char stored_hash[17];
    if (hash_len != 16) return 0; // FNV-1a 64 hex length
    memcpy(stored_hash, stored, 16);
    stored_hash[16] = '\0';
    const char* salt_hex = colon + 1;
    unsigned char salt[SALT_LEN];
    int read_len = hex_to_bytes(salt_hex, salt, SALT_LEN);
    if (read_len != SALT_LEN) return 0;
    unsigned long long h = fnv1a64((const unsigned char*)password, strlen(password), salt, SALT_LEN);
    char calc_hash[17];
    ull_to_hex16(h, calc_hash);
    return constant_time_eq(stored_hash, calc_hash) ? 1 : 0;
}

int main() {
    // 5 test cases
    printf("Register alice/password123: %s\n", register_user("alice", "password123") ? "true" : "false");
    printf("Login alice/password123: %s\n", login_user("alice", "password123") ? "true" : "false");
    printf("Login alice/wrong: %s\n", login_user("alice", "wrong") ? "true" : "false");
    printf("Register alice/another: %s\n", register_user("alice", "another") ? "true" : "false");
    printf("Register bob/S3cret!: %s\n", register_user("bob", "S3cret!") ? "true" : "false");
    return 0;
}