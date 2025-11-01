#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* username;
    uint8_t* salt;
    size_t salt_len;
    uint8_t* hash;
    size_t hash_len;
} UserRecord;

typedef struct {
    UserRecord* items;
    size_t count;
    size_t capacity;
} InMemoryDB;

static void secure_bzero(void* v, size_t n) {
    volatile uint8_t* p = (volatile uint8_t*)v;
    while (n--) *p++ = 0;
}

static void db_init(InMemoryDB* db) {
    db->items = NULL;
    db->count = 0;
    db->capacity = 0;
}

static void db_free(InMemoryDB* db) {
    for (size_t i = 0; i < db->count; i++) {
        free(db->items[i].username);
        free(db->items[i].salt);
        free(db->items[i].hash);
    }
    free(db->items);
    db->items = NULL; db->count = db->capacity = 0;
}

static int db_insert(InMemoryDB* db, const UserRecord* rec) {
    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->items[i].username, rec->username) == 0) return 0;
    }
    if (db->count == db->capacity) {
        size_t newcap = db->capacity ? db->capacity * 2 : 8;
        UserRecord* nitems = (UserRecord*)realloc(db->items, newcap * sizeof(UserRecord));
        if (!nitems) return 0;
        db->items = nitems;
        db->capacity = newcap;
    }
    db->items[db->count++] = *rec;
    return 1;
}

static UserRecord* db_get(InMemoryDB* db, const char* username) {
    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->items[i].username, username) == 0) return &db->items[i];
    }
    return NULL;
}

static int isValidUsername(const char* u) {
    size_t len = u ? strlen(u) : 0;
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

static int isValidPassword(const char* p) {
    size_t len = p ? strlen(p) : 0;
    return len >= 8;
}

/* SHA-256 implementation */
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR((x),2) ^ ROTR((x),13) ^ ROTR((x),22))
#define EP1(x) (ROTR((x),6) ^ ROTR((x),11) ^ ROTR((x),25))
#define SIG0(x) (ROTR((x),7) ^ ROTR((x),18) ^ ((x) >> 3))
#define SIG1(x) (ROTR((x),17) ^ ROTR((x),19) ^ ((x) >> 10))

static const uint32_t k256[64] = {
  0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
  0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
  0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
  0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
  0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
  0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
  0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
  0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; i++) {
        m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 |
               (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
    }
    for (int i = 16; i < 64; i++) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + k256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    uint64_t bitlen = ctx->bitlen + ctx->datalen * 8;
    size_t i = ctx->datalen;

    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;
    for (int j = 7; j >= 0; j--) {
        ctx->data[i++] = (uint8_t)((bitlen >> (j*8)) & 0xFF);
    }
    sha256_transform(ctx, ctx->data);

    for (int j = 0; j < 8; j++) {
        hash[j*4]   = (uint8_t)((ctx->state[j] >> 24) & 0xFF);
        hash[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xFF);
        hash[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xFF);
        hash[j*4+3] = (uint8_t)(ctx->state[j] & 0xFF);
    }
    secure_bzero(ctx, sizeof(*ctx));
}

static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]) {
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];
    if (keylen > 64) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, keylen);
        sha256_final(&c, tk);
        key = tk;
        keylen = 32;
    }
    for (size_t i = 0; i < 64; i++) { k_ipad[i] = 0x36; k_opad[i] = 0x5c; }
    for (size_t i = 0; i < keylen; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }
    uint8_t inner_hash[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    secure_bzero(tk, sizeof(tk));
    secure_bzero(k_ipad, sizeof(k_ipad));
    secure_bzero(k_opad, sizeof(k_opad));
    secure_bzero(inner_hash, sizeof(inner_hash));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out, size_t outlen) {
    const size_t hlen = 32;
    uint32_t blocks = (uint32_t)((outlen + hlen - 1) / hlen);
    uint8_t* U = (uint8_t*)malloc(hlen);
    uint8_t* T = (uint8_t*)malloc(hlen);
    uint8_t* sb = (uint8_t*)malloc(saltlen + 4);
    if (!U || !T || !sb) { free(U); free(T); free(sb); return; }
    memcpy(sb, salt, saltlen);

    for (uint32_t i = 1; i <= blocks; i++) {
        sb[saltlen]   = (uint8_t)((i >> 24) & 0xFF);
        sb[saltlen+1] = (uint8_t)((i >> 16) & 0xFF);
        sb[saltlen+2] = (uint8_t)((i >> 8) & 0xFF);
        sb[saltlen+3] = (uint8_t)(i & 0xFF);

        hmac_sha256(password, passlen, sb, saltlen + 4, U);
        memcpy(T, U, hlen);
        for (uint32_t j = 1; j < iterations; j++) {
            hmac_sha256(password, passlen, U, hlen, U);
            for (size_t k = 0; k < hlen; k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * hlen;
        size_t to_copy = (outlen - offset) < hlen ? (outlen - offset) : hlen;
        memcpy(out + offset, T, to_copy);
    }
    secure_bzero(U, hlen); secure_bzero(T, hlen); secure_bzero(sb, saltlen + 4);
    free(U); free(T); free(sb);
}

static int get_random_bytes(uint8_t* out, size_t len) {
#if defined(_WIN32)
    // Minimal Windows fallback using rand_s
    unsigned int r;
    for (size_t i = 0; i < len; i++) {
        if (rand_s(&r) != 0) return 0;
        out[i] = (uint8_t)(r & 0xFF);
    }
    return 1;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t rd = fread(out, 1, len, f);
    fclose(f);
    return rd == len;
#endif
}

static uint8_t* generateSalt(size_t len) {
    uint8_t* s = (uint8_t*)malloc(len);
    if (!s) return NULL;
    if (!get_random_bytes(s, len)) { free(s); return NULL; }
    return s;
}

static uint8_t* hashPassword(const char* password, const uint8_t* salt, size_t saltlen,
                             uint32_t iterations, size_t dklen) {
    uint8_t* out = (uint8_t*)malloc(dklen);
    if (!out) return NULL;
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, saltlen, iterations, out, dklen);
    return out;
}

static UserRecord createUserRecord(const char* username, const char* password) {
    UserRecord rec;
    rec.username = NULL;
    rec.salt = NULL; rec.hash = NULL;
    rec.salt_len = 16; rec.hash_len = 32;

    rec.username = (char*)malloc(strlen(username) + 1);
    if (!rec.username) return rec;
    strcpy(rec.username, username);

    rec.salt = generateSalt(rec.salt_len);
    if (!rec.salt) return rec;

    rec.hash = hashPassword(password, rec.salt, rec.salt_len, 100000, rec.hash_len);
    return rec;
}

static int registerUser(InMemoryDB* db, const char* username, const char* password) {
    if (!isValidUsername(username) || !isValidPassword(password)) return 0;
    UserRecord rec = createUserRecord(username, password);
    if (!rec.username || !rec.salt || !rec.hash) {
        free(rec.username); free(rec.salt); free(rec.hash);
        return 0;
    }
    if (!db_insert(db, &rec)) {
        free(rec.username); free(rec.salt); free(rec.hash);
        return 0;
    }
    return 1;
}

static void print_hex(const uint8_t* b, size_t n) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < n; i++) {
        putchar(hex[(b[i] >> 4) & 0xF]);
        putchar(hex[b[i] & 0xF]);
    }
}

int main(void) {
    InMemoryDB db; db_init(&db);
    const char* users[5] = {"alice_01","bob_user","charlie_3","bob_user","eve_user"};
    const char* passwords[5] = {"S3cur3P@ssw0rd!","Another$trong1","passw0rdLongEnough","Another$trong1","Y3t@An0th3rP@ss"};

    for (int i = 0; i < 5; i++) {
        int ok = registerUser(&db, users[i], passwords[i]);
        printf("Register '%s': %s\n", users[i], ok ? "true" : "false");
        UserRecord* rec = db_get(&db, users[i]);
        if (rec) {
            printf("  salt(hex)=");
            print_hex(rec->salt, rec->salt_len);
            printf("\n  hash(hex)=");
            print_hex(rec->hash, rec->hash_len);
            printf("\n");
        }
    }
    printf("Total users stored: %zu\n", db.count);
    db_free(&db);
    return 0;
}