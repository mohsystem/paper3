#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Minimal SHA-256 implementation in C
typedef uint32_t u32;
typedef uint64_t u64;

static inline u32 rotr(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }
static inline u32 ch(u32 x, u32 y, u32 z) { return (x & y) ^ (~x & z); }
static inline u32 maj(u32 x, u32 y, u32 z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline u32 big_sigma0(u32 x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
static inline u32 big_sigma1(u32 x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
static inline u32 small_sigma0(u32 x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
static inline u32 small_sigma1(u32 x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

static const u32 K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    u32 h0 = 0x6a09e667ul, h1 = 0xbb67ae85ul, h2 = 0x3c6ef372ul, h3 = 0xa54ff53aul;
    u32 h4 = 0x510e527ful, h5 = 0x9b05688cul, h6 = 0x1f83d9abul, h7 = 0x5be0cd19ul;

    size_t new_len = len + 1;
    while ((new_len % 64) != 56) new_len++;
    size_t total_len = new_len + 8;
    uint8_t* msg = (uint8_t*)malloc(total_len);
    if (!msg) return;
    memcpy(msg, data, len);
    msg[len] = 0x80;
    memset(msg + len + 1, 0, new_len - len - 1);
    u64 bitlen = (u64)len * 8ull;
    for (int i = 0; i < 8; ++i) {
        msg[new_len + i] = (uint8_t)((bitlen >> (8 * (7 - i))) & 0xff);
    }

    for (size_t chunk = 0; chunk < total_len; chunk += 64) {
        u32 w[64];
        for (int i = 0; i < 16; ++i) {
            size_t idx = chunk + i * 4;
            w[i] = ((u32)msg[idx] << 24) | ((u32)msg[idx + 1] << 16) | ((u32)msg[idx + 2] << 8) | (u32)msg[idx + 3];
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = small_sigma1(w[i - 2]) + w[i - 7] + small_sigma0(w[i - 15]) + w[i - 16];
        }
        u32 a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;
        for (int i = 0; i < 64; ++i) {
            u32 t1 = h + big_sigma1(e) + ch(e, f, g) + K256[i] + w[i];
            u32 t2 = big_sigma0(a) + maj(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e; h5 += f; h6 += g; h7 += h;
    }
    free(msg);

    u32 hs[8] = {h0,h1,h2,h3,h4,h5,h6,h7};
    for (int i = 0; i < 8; ++i) {
        out[i*4+0] = (uint8_t)((hs[i] >> 24) & 0xff);
        out[i*4+1] = (uint8_t)((hs[i] >> 16) & 0xff);
        out[i*4+2] = (uint8_t)((hs[i] >> 8) & 0xff);
        out[i*4+3] = (uint8_t)(hs[i] & 0xff);
    }
}

static int const_time_eq(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i = 0; i < n; ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

static int valid_email(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;
    // Simple validation: must contain one '@' and a '.' after '@'
    const char* at = strchr(email, '@');
    if (!at || at == email) return 0;
    const char* dot = strrchr(at, '.');
    if (!dot || dot <= at + 1 || dot == email + len - 1) return 0;
    return 1;
}

static void to_lower_str(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static int rand_bytes(uint8_t* out, size_t n) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t r = fread(out, 1, n, f);
        fclose(f);
        if (r == n) return 1;
    }
    // fallback (not ideal)
    for (size_t i = 0; i < n; ++i) out[i] = (uint8_t)(rand() & 0xFF);
    return 1;
}

#define MAX_USERS 32
#define MAX_SESSIONS 64

typedef struct {
    char email[128];
    uint8_t salt[16];
    uint8_t hash[32];
    int used;
} User;

typedef struct {
    char token[65]; // hex of 32 bytes -> 64 chars + null
    char email_lower[128];
    int used;
} Session;

typedef struct {
    User users[MAX_USERS];
    Session sessions[MAX_SESSIONS];
} AuthSystem;

static void init_auth(AuthSystem* a) {
    memset(a, 0, sizeof(*a));
}

static int find_user_idx(AuthSystem* a, const char* email_lower) {
    for (int i = 0; i < MAX_USERS; ++i) {
        if (a->users[i].used && strcmp(a->users[i].email, email_lower) == 0) {
            return i;
        }
    }
    return -1;
}

static int email_exists(AuthSystem* a, const char* email_lower) {
    return find_user_idx(a, email_lower) >= 0;
}

static int register_user(AuthSystem* a, const char* email, const char* password) {
    if (!valid_email(email) || !password || !*password) return 0;
    char email_lower[128];
    strncpy(email_lower, email, sizeof(email_lower)-1);
    email_lower[sizeof(email_lower)-1] = 0;
    to_lower_str(email_lower);
    if (email_exists(a, email_lower)) return 0;
    int slot = -1;
    for (int i = 0; i < MAX_USERS; ++i) if (!a->users[i].used) { slot = i; break; }
    if (slot < 0) return 0;
    User* u = &a->users[slot];
    memset(u, 0, sizeof(*u));
    strncpy(u->email, email_lower, sizeof(u->email)-1);
    rand_bytes(u->salt, 16);
    // hash = SHA256(salt || password)
    uint8_t buf[16 + 256];
    size_t pwlen = strlen(password);
    if (pwlen > 256) pwlen = 256;
    memcpy(buf, u->salt, 16);
    memcpy(buf + 16, password, pwlen);
    sha256(buf, 16 + pwlen, u->hash);
    u->used = 1;
    return 1;
}

static const char* login_user(AuthSystem* a, const char* email, const char* password) {
    static char token_buf[65];
    token_buf[0] = 0;
    if (!email || !password) return token_buf;
    char email_lower[128];
    strncpy(email_lower, email, sizeof(email_lower)-1);
    email_lower[sizeof(email_lower)-1] = 0;
    to_lower_str(email_lower);
    int idx = find_user_idx(a, email_lower);
    if (idx < 0) return token_buf;
    User* u = &a->users[idx];
    uint8_t buf[16 + 256], hash[32];
    size_t pwlen = strlen(password);
    if (pwlen > 256) pwlen = 256;
    memcpy(buf, u->salt, 16);
    memcpy(buf + 16, password, pwlen);
    sha256(buf, 16 + pwlen, hash);
    if (!const_time_eq(hash, u->hash, 32)) return token_buf;

    // Create session
    int sidx = -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) if (!a->sessions[i].used) { sidx = i; break; }
    if (sidx < 0) return token_buf;
    uint8_t rb[32];
    rand_bytes(rb, sizeof(rb));
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        token_buf[i*2] = hex[rb[i] >> 4];
        token_buf[i*2+1] = hex[rb[i] & 0xF];
    }
    token_buf[64] = 0;
    a->sessions[sidx].used = 1;
    strncpy(a->sessions[sidx].token, token_buf, sizeof(a->sessions[sidx].token)-1);
    strncpy(a->sessions[sidx].email_lower, email_lower, sizeof(a->sessions[sidx].email_lower)-1);
    return token_buf;
}

static int change_email(AuthSystem* a, const char* session_token, const char* old_email, const char* new_email, const char* confirm_password) {
    if (!session_token || !old_email || !new_email || !confirm_password) return 0;
    if (!valid_email(new_email)) return 0;

    int sidx = -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (a->sessions[i].used && strcmp(a->sessions[i].token, session_token) == 0) {
            sidx = i; break;
        }
    }
    if (sidx < 0) return 0;

    char old_lower[128], new_lower[128];
    strncpy(old_lower, old_email, sizeof(old_lower)-1); old_lower[sizeof(old_lower)-1] = 0; to_lower_str(old_lower);
    strncpy(new_lower, new_email, sizeof(new_lower)-1); new_lower[sizeof(new_lower)-1] = 0; to_lower_str(new_lower);

    if (strcmp(a->sessions[sidx].email_lower, old_lower) != 0) return 0;

    int uidx = find_user_idx(a, old_lower);
    if (uidx < 0) return 0;
    if (email_exists(a, new_lower)) return 0;

    User* u = &a->users[uidx];

    uint8_t buf[16 + 256], hash[32];
    size_t pwlen = strlen(confirm_password);
    if (pwlen > 256) pwlen = 256;
    memcpy(buf, u->salt, 16);
    memcpy(buf + 16, confirm_password, pwlen);
    sha256(buf, 16 + pwlen, hash);
    if (!const_time_eq(hash, u->hash, 32)) return 0;

    // Update email
    strncpy(u->email, new_lower, sizeof(u->email)-1);
    // Update session mapping
    strncpy(a->sessions[sidx].email_lower, new_lower, sizeof(a->sessions[sidx].email_lower)-1);
    return 1;
}

// 5 test cases
int main(void) {
    AuthSystem auth;
    init_auth(&auth);

    printf("Register Alice: %s\n", register_user(&auth, "alice@example.com", "CorrectHorseBatteryStaple!") ? "true" : "false");
    printf("Register Bob: %s\n", register_user(&auth, "bob@example.com", "B0bStrong#Pass") ? "true" : "false");

    const char* token = login_user(&auth, "alice@example.com", "CorrectHorseBatteryStaple!");
    printf("Alice token present: %s\n", (token && token[0]) ? "true" : "false");

    int t1 = change_email(&auth, "badtoken", "alice@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!");
    printf("Test1 (invalid session): %s\n", t1 ? "true" : "false");

    int t2 = change_email(&auth, token, "wrong@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!");
    printf("Test2 (wrong old email): %s\n", t2 ? "true" : "false");

    int t3 = change_email(&auth, token, "alice@example.com", "alice_new@example.com", "wrongpass");
    printf("Test3 (wrong password): %s\n", t3 ? "true" : "false");

    int t4 = change_email(&auth, token, "alice@example.com", "bob@example.com", "CorrectHorseBatteryStaple!");
    printf("Test4 (email exists): %s\n", t4 ? "true" : "false");

    int t5 = change_email(&auth, token, "alice@example.com", "alice2@example.com", "CorrectHorseBatteryStaple!");
    printf("Test5 (success): %s\n", t5 ? "true" : "false");

    const char* token2 = login_user(&auth, "alice2@example.com", "CorrectHorseBatteryStaple!");
    printf("Login with new email works: %s\n", (token2 && token2[0]) ? "true" : "false");

    return 0;
}