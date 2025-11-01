#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Step 1-2: Purpose and security considerations (comments only)
// Step 3-5: Implement secure code; includes constant-time compare and SHA-256

// Minimal SHA-256 implementation
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

static const uint32_t k256_c[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTRIGHT_C(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH_C(x,y,z)     (((x) & (y)) ^ (~(x) & (z)))
#define MAJ_C(x,y,z)    (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0_C(x)        (ROTRIGHT_C(x,2) ^ ROTRIGHT_C(x,13) ^ ROTRIGHT_C(x,22))
#define EP1_C(x)        (ROTRIGHT_C(x,6) ^ ROTRIGHT_C(x,11) ^ ROTRIGHT_C(x,25))
#define SIG0_C(x)       (ROTRIGHT_C(x,7) ^ ROTRIGHT_C(x,18) ^ ((x) >> 3))
#define SIG1_C(x)       (ROTRIGHT_C(x,17) ^ ROTRIGHT_C(x,19) ^ ((x) >> 10))

static void sha256_transform_c(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1_C(m[i-2]) + m[i-7] + SIG0_C(m[i-15]) + m[i-16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1_C(e) + CH_C(e,f,g) + k256_c[i] + m[i];
        t2 = EP0_C(a) + MAJ_C(a,b,c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init_c(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

static void sha256_update_c(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform_c(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final_c(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform_c(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform_c(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

static void sha256_hex_c(const uint8_t *data, size_t len, char out_hex[65]) {
    uint8_t hash[32];
    SHA256_CTX ctx;
    sha256_init_c(&ctx);
    sha256_update_c(&ctx, data, len);
    sha256_final_c(&ctx, hash);
    static const char *hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[2*i]   = hex[(hash[i] >> 4) & 0xF];
        out_hex[2*i+1] = hex[hash[i] & 0xF];
    }
    out_hex[64] = '\0';
}

static int ct_equals_c(const char *a, const char *b) {
    if (!a || !b) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; ++i) {
        diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    return diff == 0;
}

static void random_salt_hex(char *out_hex, size_t nbytes) {
    // Not cryptographically strong across all platforms: fallback to rand seeded by time.
    // For demonstration: prefer secure RNGs in production.
    srand((unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)&out_hex);
    static const char *hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned v = rand() & 0xFF;
        out_hex[2*i]   = hex[(v >> 4) & 0xF];
        out_hex[2*i+1] = hex[v & 0xF];
    }
    out_hex[2*nbytes] = '\0';
}

typedef struct {
    char username[64];
    char email[128];
    char saltHex[33];        // 16 bytes salt -> 32 hex chars + '\0'
    char passwordHashHex[65];
    int  loggedIn;
} User;

typedef struct {
    User user;
} SecureLoginSystem;

void init_system(SecureLoginSystem *sys, const char *username, const char *email, const char *password) {
    memset(sys, 0, sizeof(*sys));
    strncpy(sys->user.username, username, sizeof(sys->user.username)-1);
    strncpy(sys->user.email, email, sizeof(sys->user.email)-1);
    random_salt_hex(sys->user.saltHex, 16);
    // hash = sha256(saltHex || password)
    size_t plen = strlen(password);
    size_t slen = strlen(sys->user.saltHex);
    char *buf = (char*)malloc(slen + plen + 1);
    memcpy(buf, sys->user.saltHex, slen);
    memcpy(buf + slen, password, plen);
    buf[slen + plen] = '\0';
    sha256_hex_c((const uint8_t*)buf, slen + plen, sys->user.passwordHashHex);
    free(buf);
    sys->user.loggedIn = 0;
}

int login_user(SecureLoginSystem *sys, const char *username, const char *password) {
    if (!username || !password) return 0;
    if (!ct_equals_c(username, sys->user.username)) return 0;
    size_t plen = strlen(password);
    size_t slen = strlen(sys->user.saltHex);
    char *buf = (char*)malloc(slen + plen + 1);
    memcpy(buf, sys->user.saltHex, slen);
    memcpy(buf + slen, password, plen);
    buf[slen + plen] = '\0';
    char cand[65];
    sha256_hex_c((const uint8_t*)buf, slen + plen, cand);
    free(buf);
    int ok = ct_equals_c(cand, sys->user.passwordHashHex);
    sys->user.loggedIn = ok ? 1 : 0;
    return ok;
}

void logout_user(SecureLoginSystem *sys) {
    sys->user.loggedIn = 0;
}

int change_email(SecureLoginSystem *sys, const char *oldEmail, const char *newEmail, const char *confirmPassword) {
    if (!sys->user.loggedIn) return 0;
    if (!oldEmail || !newEmail || !confirmPassword) return 0;
    if (!ct_equals_c(oldEmail, sys->user.email)) return 0;

    size_t plen = strlen(confirmPassword);
    size_t slen = strlen(sys->user.saltHex);
    char *buf = (char*)malloc(slen + plen + 1);
    memcpy(buf, sys->user.saltHex, slen);
    memcpy(buf + slen, confirmPassword, plen);
    buf[slen + plen] = '\0';
    char cand[65];
    sha256_hex_c((const uint8_t*)buf, slen + plen, cand);
    free(buf);

    if (!ct_equals_c(cand, sys->user.passwordHashHex)) return 0;
    strncpy(sys->user.email, newEmail, sizeof(sys->user.email)-1);
    sys->user.email[sizeof(sys->user.email)-1] = '\0';
    return 1;
}

const char* get_email(SecureLoginSystem *sys) {
    return sys->user.email;
}

int main(void) {
    SecureLoginSystem sys;
    init_system(&sys, "alice", "alice@example.com", "S3cret!42");

    // Test 1: change while not logged in -> fail
    int t1 = change_email(&sys, "alice@example.com", "new1@example.com", "S3cret!42");
    printf("Test1 (change while not logged in): %s | email=%s\n", t1 ? "true" : "false", get_email(&sys));

    // Test 2: login wrong password -> fail
    int t2 = login_user(&sys, "alice", "WrongPass!");
    printf("Test2 (login wrong password): %s | email=%s\n", t2 ? "true" : "false", get_email(&sys));

    // Test 3: login correct password -> succeed
    int t3 = login_user(&sys, "alice", "S3cret!42");
    printf("Test3 (login correct password): %s | email=%s\n", t3 ? "true" : "false", get_email(&sys));

    // Test 4: change with wrong old email -> fail
    int t4 = change_email(&sys, "wrong@example.com", "new2@example.com", "S3cret!42");
    printf("Test4 (wrong old email): %s | email=%s\n", t4 ? "true" : "false", get_email(&sys));

    // Test 5: successful change -> succeed
    int t5 = change_email(&sys, "alice@example.com", "alice.new@example.com", "S3cret!42");
    printf("Test5 (successful change): %s | email=%s\n", t5 ? "true" : "false", get_email(&sys));

    return 0;
}