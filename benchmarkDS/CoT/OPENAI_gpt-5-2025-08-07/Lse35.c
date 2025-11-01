#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define HASH_ITERATIONS 10000
#define MAX_USERS 16

static const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
static const uint64_t FNV_PRIME = 0x100000001b3ULL;

typedef struct {
    char username[65]; // up to 64 chars + null
    char hash[17];     // 16 hex chars + null
    int in_use;
} User;

static User DB[MAX_USERS];

int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len == 0 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!( (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '_' || c == '.' || c == '-' )) {
            return 0;
        }
    }
    return 1;
}

uint64_t fnv1a64_round(uint64_t current, const unsigned char* data, size_t len) {
    uint64_t h = current;
    for (size_t i = 0; i < len; i++) {
        h ^= (uint64_t)data[i];
        h *= FNV_PRIME;
    }
    return h;
}

void to_hex16(uint64_t v, char out[17]) {
    static const char* hex = "0123456789abcdef";
    for (int i = 15; i >= 0; --i) {
        out[i] = hex[(int)(v & 0xFULL)];
        v >>= 4;
    }
    out[16] = '\0';
}

char* hash_password(const char* username, const char* password) {
    const char* prefix = "s@lt-";
    const char* sep = ":";
    size_t len_user = strlen(username);
    size_t len_pass = strlen(password);
    size_t len_prefix = strlen(prefix);
    size_t len_sep = 1;
    size_t data_len = len_prefix + len_user + len_sep + len_pass;

    unsigned char* data = (unsigned char*)malloc(data_len);
    if (!data) return NULL;

    memcpy(data, prefix, len_prefix);
    memcpy(data + len_prefix, username, len_user);
    memcpy(data + len_prefix + len_user, sep, len_sep);
    memcpy(data + len_prefix + len_user + len_sep, password, len_pass);

    uint64_t h = FNV_OFFSET_BASIS;
    for (int i = 0; i < HASH_ITERATIONS; i++) {
        h = fnv1a64_round(h, data, data_len);
        // Mix iteration counter bytes (little-endian style)
        h ^= (uint64_t)(i & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 8) & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 16) & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 24) & 0xFF); h *= FNV_PRIME;
    }

    free(data);

    char* hex = (char*)malloc(17);
    if (!hex) return NULL;
    to_hex16(h, hex);
    return hex;
}

int secure_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return 0;
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t max = len_a > len_b ? len_a : len_b;
    unsigned int diff = (unsigned int)(len_a ^ len_b);
    for (size_t i = 0; i < max; i++) {
        unsigned char ca = (i < len_a) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < len_b) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ca ^ cb);
    }
    return diff == 0;
}

void add_user(const char* username, const char* password) {
    if (!is_valid_username(username)) return;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!DB[i].in_use) {
            char* h = hash_password(username, password);
            if (!h) return;
            strncpy(DB[i].username, username, sizeof(DB[i].username) - 1);
            DB[i].username[sizeof(DB[i].username) - 1] = '\0';
            strncpy(DB[i].hash, h, sizeof(DB[i].hash) - 1);
            DB[i].hash[sizeof(DB[i].hash) - 1] = '\0';
            DB[i].in_use = 1;
            free(h);
            return;
        }
    }
}

const char* find_user_hash(const char* username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (DB[i].in_use && strcmp(DB[i].username, username) == 0) {
            return DB[i].hash;
        }
    }
    return NULL;
}

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "redirect:/login";
    if (!is_valid_username(username)) return "redirect:/login";
    const char* stored = find_user_hash(username);
    if (stored == NULL) return "redirect:/login";
    char* provided = hash_password(username, password);
    if (!provided) return "redirect:/login";
    int ok = secure_equals(stored, provided);
    free(provided);
    return ok ? "redirect:/home" : "redirect:/login";
}

int main(void) {
    add_user("alice", "Pa$$w0rd!");
    add_user("bob", "secret1234");
    add_user("charlie", "CorrectHorseBatteryStaple");

    printf("Test1: %s\n", login("alice", "Pa$$w0rd!"));             // redirect:/home
    printf("Test2: %s\n", login("alice", "wrong"));                  // redirect:/login
    printf("Test3: %s\n", login("dave", "anything"));                // redirect:/login
    printf("Test4: %s\n", login("eve; DROP TABLE users", "x"));      // redirect:/login (invalid username)
    printf("Test5: %s\n", login("bob", "secret1234"));               // redirect:/home

    return 0;
}