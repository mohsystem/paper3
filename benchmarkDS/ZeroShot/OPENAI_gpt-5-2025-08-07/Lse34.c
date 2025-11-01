#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Minimal SHA-256 + HMAC + PBKDF2 implementation */

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static const uint32_t K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 | (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(m[i-15],7) ^ rotr(m[i-15],18) ^ (m[i-15] >> 3);
        uint32_t s1 = rotr(m[i-2],17) ^ rotr(m[i-2],19) ^ (m[i-2] >> 10);
        m[i] = m[i-16] + s0 + m[i-7] + s1;
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + K256[i] + m[i];
        uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul; ctx->state[1] = 0xbb67ae85ul; ctx->state[2] = 0x3c6ef372ul; ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful; ctx->state[5] = 0x9b05688cul; ctx->state[6] = 0x1f83d9abul; ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >>  8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i*8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i*8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i*8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i*8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i*8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i*8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i*8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i*8)) & 0xff);
    }
}

static void sha256_raw(const uint8_t* data, size_t len, uint8_t out[32]) {
    SHA256_CTX ctx; sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

static void hmac_sha256(const uint8_t* key, size_t keyLen, const uint8_t* data, size_t dataLen, uint8_t out[32]) {
    const size_t blockSize = 64;
    uint8_t k[64]; memset(k, 0, sizeof(k));
    if (keyLen > blockSize) {
        sha256_raw(key, keyLen, k);
        keyLen = 32;
    } else {
        memcpy(k, key, keyLen);
    }
    uint8_t o_key_pad[64], i_key_pad[64];
    for (size_t i = 0; i < blockSize; ++i) {
        o_key_pad[i] = k[i] ^ 0x5c;
        i_key_pad[i] = k[i] ^ 0x36;
    }
    uint8_t inner_buf[64 + 1024]; // dataLen may exceed; handle dynamically
    size_t inner_len = 64 + dataLen;
    uint8_t* inner = (uint8_t*)malloc(inner_len);
    memcpy(inner, i_key_pad, 64);
    memcpy(inner + 64, data, dataLen);
    uint8_t inner_hash[32];
    sha256_raw(inner, inner_len, inner_hash);
    free(inner);

    uint8_t outer[64 + 32];
    memcpy(outer, o_key_pad, 64);
    memcpy(outer + 64, inner_hash, 32);
    sha256_raw(outer, sizeof(outer), out);
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t pwdLen, const uint8_t* salt, size_t saltLen, uint32_t iterations, uint8_t* dk, size_t dkLen) {
    const size_t hLen = 32;
    uint32_t l = (uint32_t)((dkLen + hLen - 1) / hLen);
    for (uint32_t i = 1; i <= l; ++i) {
        uint8_t* sb = (uint8_t*)malloc(saltLen + 4);
        memcpy(sb, salt, saltLen);
        sb[saltLen+0] = (uint8_t)(i >> 24);
        sb[saltLen+1] = (uint8_t)(i >> 16);
        sb[saltLen+2] = (uint8_t)(i >> 8);
        sb[saltLen+3] = (uint8_t)(i);
        uint8_t u[32], t[32];
        hmac_sha256(password, pwdLen, sb, saltLen + 4, u);
        memcpy(t, u, 32);
        for (uint32_t j = 2; j <= iterations; ++j) {
            hmac_sha256(password, pwdLen, u, 32, u);
            for (size_t k = 0; k < 32; ++k) t[k] ^= u[k];
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t clen = (dkLen - offset > hLen) ? hLen : (dkLen - offset);
        memcpy(dk + offset, t, clen);
        free(sb);
        memset(u, 0, sizeof(u));
        memset(t, 0, sizeof(t));
    }
}

static int secure_memcmp(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t diff = 0;
    for (size_t i = 0; i < n; ++i) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff; // 0 if equal
}

static int get_secure_random_bytes(uint8_t* out, size_t n) {
#if defined(_WIN32)
    /* Minimal Windows implementation using BCryptGenRandom */
    #include <windows.h>
    #include <bcrypt.h>
    NTSTATUS status = BCryptGenRandom(NULL, out, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return -1;
    size_t r = fread(out, 1, n, f);
    fclose(f);
    return r == n ? 0 : -1;
#endif
}

typedef struct {
    char* username;
    uint8_t* salt;
    uint8_t* hash;
    size_t saltLen;
    size_t hashLen;
} UserRecord;

typedef struct {
    UserRecord* records;
    size_t count;
} Database;

static UserRecord create_user_record(const char* username, const char* password) {
    UserRecord ur;
    ur.username = strdup(username);
    ur.saltLen = 16;
    ur.hashLen = 32;
    ur.salt = (uint8_t*)malloc(ur.saltLen);
    ur.hash = (uint8_t*)malloc(ur.hashLen);
    if (get_secure_random_bytes(ur.salt, ur.saltLen) != 0) {
        /* fallback insecure random if necessary */
        for (size_t i = 0; i < ur.saltLen; ++i) ur.salt[i] = (uint8_t)rand();
    }
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), ur.salt, ur.saltLen, 100000, ur.hash, ur.hashLen);
    return ur;
}

static const char* authenticate_and_redirect(const Database* db, const char* username, const char* password) {
    if (!db || !username || !password || username[0] == '\0' || password[0] == '\0') return "redirect:/login";
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->records[i].username, username) == 0) {
            uint8_t computed[32];
            pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), db->records[i].salt, db->records[i].saltLen, 100000, computed, 32);
            int eq = secure_memcmp(computed, db->records[i].hash, 32);
            memset(computed, 0, sizeof(computed));
            return eq == 0 ? "redirect:/dashboard" : "redirect:/login";
        }
    }
    return "redirect:/login";
}

int main() {
    Database db;
    db.count = 2;
    db.records = (UserRecord*)calloc(db.count, sizeof(UserRecord));
    db.records[0] = create_user_record("alice", "S3curePass!");
    db.records[1] = create_user_record("bob", "hunter2?");

    printf("%s\n", authenticate_and_redirect(&db, "alice", "S3curePass!")); // dashboard
    printf("%s\n", authenticate_and_redirect(&db, "alice", "wrongpass"));   // login
    printf("%s\n", authenticate_and_redirect(&db, "unknown", "anything"));  // login
    printf("%s\n", authenticate_and_redirect(&db, "bob", "hunter2?"));      // dashboard
    printf("%s\n", authenticate_and_redirect(&db, "", ""));                 // login

    // cleanup
    for (size_t i = 0; i < db.count; ++i) {
        free(db.records[i].username);
        if (db.records[i].salt) { memset(db.records[i].salt, 0, db.records[i].saltLen); free(db.records[i].salt); }
        if (db.records[i].hash) { memset(db.records[i].hash, 0, db.records[i].hashLen); free(db.records[i].hash); }
    }
    free(db.records);
    return 0;
}