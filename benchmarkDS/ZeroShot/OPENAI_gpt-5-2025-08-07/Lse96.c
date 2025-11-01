#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// Simple in-memory DB
struct kv {
    char* key;
    char* value;
};
static struct kv DB[128];
static size_t DB_LEN = 0;

static void db_put(const char* key, const char* value){
    for(size_t i=0;i<DB_LEN;i++){
        if(strcmp(DB[i].key, key) == 0){
            free(DB[i].value);
            DB[i].value = _strdup(value);
            return;
        }
    }
    if (DB_LEN < 128){
        DB[DB_LEN].key = _strdup(key);
        DB[DB_LEN].value = _strdup(value);
        DB_LEN++;
    }
}
static const char* db_get(const char* key){
    for(size_t i=0;i<DB_LEN;i++){
        if(strcmp(DB[i].key, key) == 0){
            return DB[i].value;
        }
    }
    return NULL;
}

#ifndef _MSC_VER
#define _strdup strdup
#endif

// SHA-256 implementation (same as in C++ block, adapted to C)
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static const uint32_t SHA256_K[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i*4] << 24 |
               (uint32_t)data[i*4 + 1] << 16 |
               (uint32_t)data[i*4 + 2] << 8 |
               (uint32_t)data[i*4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + SHA256_K[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
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

static void sha256_init(SHA256_CTX* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len){
    for(size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if(i > 56){
        while(i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while(i < 56) ctx->data[i++] = 0x00;
    ctx->bitlen += ctx->datalen * 8;
    for(int j = 7; j >= 0; --j){
        ctx->data[56 + (7 - j)] = (uint8_t)((ctx->bitlen >> (j*8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);
    for(i=0;i<4;i++){
        for(int j=0;j<8;j++){
            hash[i + (j*4)] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xff);
        }
    }
}

static void hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]){
    uint8_t k_ipad[64] = {0};
    uint8_t k_opad[64] = {0};
    uint8_t tk[32];
    if (key_len > 64) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, key_len);
        sha256_final(&c, tk);
        key = tk;
        key_len = 32;
    }
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    uint8_t inner[32];
    SHA256_CTX inner_ctx;
    sha256_init(&inner_ctx);
    sha256_update(&inner_ctx, k_ipad, 64);
    sha256_update(&inner_ctx, data, data_len);
    sha256_final(&inner_ctx, inner);

    SHA256_CTX outer_ctx;
    sha256_init(&outer_ctx);
    sha256_update(&outer_ctx, k_opad, 64);
    sha256_update(&outer_ctx, inner, 32);
    sha256_final(&outer_ctx, out);

    memset(k_ipad,0,sizeof(k_ipad));
    memset(k_opad,0,sizeof(k_opad));
    memset(tk,0,sizeof(tk));
    memset(inner,0,sizeof(inner));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t pass_len,
                               const uint8_t* salt, size_t salt_len,
                               uint32_t iterations,
                               uint8_t* out, size_t out_len){
    uint32_t blocks = (uint32_t)((out_len + 31) / 32);
    uint8_t U[32], T[32];
    uint8_t* salt_block = (uint8_t*)malloc(salt_len + 4);
    memcpy(salt_block, salt, salt_len);

    for(uint32_t i = 1; i <= blocks; ++i) {
        salt_block[salt_len] = (uint8_t)((i >> 24) & 0xff);
        salt_block[salt_len+1] = (uint8_t)((i >> 16) & 0xff);
        salt_block[salt_len+2] = (uint8_t)((i >> 8) & 0xff);
        salt_block[salt_len+3] = (uint8_t)(i & 0xff);
        hmac_sha256(password, pass_len, salt_block, salt_len + 4, U);
        memcpy(T, U, 32);
        for(uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, pass_len, U, 32, U);
            for(int k = 0; k < 32; ++k) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * 32;
        size_t clen = (out_len - offset > 32) ? 32 : (out_len - offset);
        memcpy(out + offset, T, clen);
    }
    memset(U,0,sizeof(U));
    memset(T,0,sizeof(T));
    if (salt_block) { memset(salt_block,0,salt_len+4); free(salt_block); }
}

static int secure_random_bytes(uint8_t* buf, size_t len){
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t rd = fread(buf, 1, len, f);
    fclose(f);
    return rd == len;
#endif
}

static char* to_hex(const uint8_t* data, size_t len){
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len*2 + 1);
    for(size_t i=0;i<len;i++){
        out[2*i] = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

static uint8_t hex_val(char c){
    if(c>='0'&&c<='9') return (uint8_t)(c-'0');
    if(c>='a'&&c<='f') return (uint8_t)(c-'a'+10);
    if(c>='A'&&c<='F') return (uint8_t)(c-'A'+10);
    return 0xff;
}

static uint8_t* from_hex(const char* h, size_t* out_len){
    size_t len = strlen(h);
    if(len % 2 != 0) return NULL;
    *out_len = len/2;
    uint8_t* out = (uint8_t*)malloc(*out_len);
    for(size_t i=0;i<*out_len;i++){
        uint8_t hi = hex_val(h[2*i]);
        uint8_t lo = hex_val(h[2*i+1]);
        if(hi==0xff || lo==0xff){ free(out); return NULL; }
        out[i] = (uint8_t)((hi<<4)|lo);
    }
    return out;
}

static int constant_time_equals(const uint8_t* a, const uint8_t* b, size_t len){
    uint8_t r = 0;
    for(size_t i=0;i<len;i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

#define SALT_LEN 16
#define ITERATIONS 210000
#define KEY_LEN 32

// Register function: returns malloc'd string (hex(salt)||hex(hash)); caller must free
char* register_user(const char* username, const char* password){
    if(!username || strlen(username) == 0) return NULL;
    if(!password || strlen(password) < 8) return NULL;

    uint8_t salt[SALT_LEN];
    if(!secure_random_bytes(salt, SALT_LEN)) return NULL;

    uint8_t hash[KEY_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       salt, SALT_LEN, ITERATIONS, hash, KEY_LEN);

    char* salt_hex = to_hex(salt, SALT_LEN);
    char* hash_hex = to_hex(hash, KEY_LEN);
    size_t rec_len = strlen(salt_hex) + strlen(hash_hex);
    char* record = (char*)malloc(rec_len + 1);
    strcpy(record, salt_hex);
    strcat(record, hash_hex);

    db_put(username, record);

    // cleanup
    memset(hash,0,sizeof(hash));
    memset(salt,0,sizeof(salt));
    free(salt_hex);
    free(hash_hex);

    return record; // caller frees
}

int verify_user(const char* username, const char* password){
    const char* record = db_get(username);
    if(!record) return 0;
    size_t rec_len = strlen(record);
    if (rec_len < SALT_LEN*2) return 0;

    char* salt_hex = (char*)malloc(SALT_LEN*2 + 1);
    strncpy(salt_hex, record, SALT_LEN*2);
    salt_hex[SALT_LEN*2] = '\0';

    const char* hash_hex = record + SALT_LEN*2;

    size_t salt_bin_len=0, hash_bin_len=0;
    uint8_t* salt_bin = from_hex(salt_hex, &salt_bin_len);
    uint8_t* expected = from_hex(hash_hex, &hash_bin_len);
    if(!salt_bin || !expected || hash_bin_len != KEY_LEN || salt_bin_len != SALT_LEN){
        free(salt_hex); if(salt_bin) free(salt_bin); if(expected) free(expected); return 0;
    }

    uint8_t actual[KEY_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       salt_bin, SALT_LEN, ITERATIONS, actual, KEY_LEN);

    int ok = constant_time_equals(expected, actual, KEY_LEN);

    memset(actual,0,sizeof(actual));
    free(salt_hex);
    free(salt_bin);
    free(expected);
    return ok;
}

const char* get_stored_record(const char* username){
    return db_get(username);
}

int main(void){
    // 5 test cases
    const char* users[5] = {"alice","bob","charlie","dave","eve"};
    const char* passwords[5] = {"password1!","S33cr3t!","superlongpassword123","Pa$$w0rd123","Secure123!"};

    for(int i=0;i<5;i++){
        char* rec = register_user(users[i], passwords[i]);
        if(rec){
            printf("%s -> %s\n", users[i], rec);
            free(rec);
        } else {
            printf("%s -> registration failed\n", users[i]);
        }
    }

    printf("Verify alice correct: %s\n", verify_user("alice","password1!") ? "true":"false");
    printf("Verify alice wrong: %s\n", verify_user("alice","wrongpass") ? "true":"false");
    printf("Verify bob correct: %s\n", verify_user("bob","S33cr3t!") ? "true":"false");
    printf("Stored for charlie: %s\n", get_stored_record("charlie"));
    printf("Verify eve correct: %s\n", verify_user("eve","Secure123!") ? "true":"false");

    return 0;
}