#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define SALT_LEN 16
#define HASH_LEN 32 /* 256-bit */
#define ITERATIONS 210000u
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128

/* Secure zeroization */
static void secure_memzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
}

/* Random bytes using /dev/urandom; fail closed */
static int random_bytes(uint8_t *buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) { close(fd); return -1; }
        off += (size_t)r;
    }
    close(fd);
    return 0;
}

/* -------- SHA-256 -------- */
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
} SHA256_CTX;

static const uint32_t K256[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static inline uint32_t ROTR32(uint32_t x, uint32_t n){ return (x>>n)|(x<<(32-n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR32(x,7) ^ ROTR32(x,18) ^ (x>>3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR32(x,17) ^ ROTR32(x,19) ^ (x>>10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i] = ((uint32_t)data[i*4]<<24)|((uint32_t)data[i*4+1]<<16)|((uint32_t)data[i*4+2]<<8)|((uint32_t)data[i*4+3]);
    }
    for (int i=16;i<64;i++){
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX *ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667u; ctx->state[1]=0xbb67ae85u; ctx->state[2]=0x3c6ef372u; ctx->state[3]=0xa54ff53au;
    ctx->state[4]=0x510e527fu; ctx->state[5]=0x9b05688cu; ctx->state[6]=0x1f83d9abu; ctx->state[7]=0x5be0cd19u;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]){
    uint32_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if (i > 56){
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;
    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i=0;i<4;i++){
        for (int j=0;j<8;j++){
            hash[i + j*4] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xffu);
        }
    }
}

/* HMAC-SHA256 */
static void hmac_sha256(const uint8_t *key, size_t keylen, const uint8_t *data, size_t len, uint8_t out[32]) {
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];
    memset(tk, 0, sizeof(tk));

    if (keylen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        key = tk;
        keylen = 32;
    }

    memset(k_ipad, 0x36, sizeof(k_ipad));
    memset(k_opad, 0x5c, sizeof(k_opad));
    for (size_t i=0;i<keylen;i++){
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    SHA256_CTX ctx;
    uint8_t inner[32];
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, inner);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner, 32);
    sha256_final(&ctx, out);

    secure_memzero(inner, sizeof(inner));
    secure_memzero(k_ipad, sizeof(k_ipad));
    secure_memzero(k_opad, sizeof(k_opad));
    secure_memzero(tk, sizeof(tk));
}

/* PBKDF2-HMAC-SHA256 */
static void pbkdf2_hmac_sha256(const uint8_t *password, size_t plen, const uint8_t *salt, size_t slen,
                               uint32_t iterations, uint8_t *out, size_t dklen) {
    uint32_t blocks = (uint32_t)((dklen + 31u) / 32u);
    uint8_t U[32];
    uint8_t T[32];

    for (uint32_t i=1;i<=blocks;i++){
        uint8_t *salt_block = (uint8_t*)malloc(slen + 4);
        if (!salt_block) exit(1);
        memcpy(salt_block, salt, slen);
        salt_block[slen]   = (uint8_t)(i >> 24);
        salt_block[slen+1] = (uint8_t)(i >> 16);
        salt_block[slen+2] = (uint8_t)(i >> 8);
        salt_block[slen+3] = (uint8_t)(i);

        hmac_sha256(password, plen, salt_block, slen + 4, U);
        memcpy(T, U, 32);
        for (uint32_t j=1;j<iterations;j++){
            hmac_sha256(password, plen, U, 32, U);
            for (size_t k=0;k<32;k++) T[k] ^= U[k];
        }

        size_t offset = (size_t)(i - 1u) * 32u;
        size_t clen = dklen - offset;
        if (clen > 32) clen = 32;
        memcpy(out + offset, T, clen);

        secure_memzero(salt_block, slen + 4);
        free(salt_block);
    }
    secure_memzero(U, sizeof(U));
    secure_memzero(T, sizeof(T));
}

/* Validation */
static int is_valid_username(const char *u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 64) return 0;
    for (size_t i=0;i<len;i++){
        char c = u[i];
        if (!( (c>='A'&&c<='Z') || (c>='a'&&c<='z') || (c>='0'&&c<='9') || c=='.' || c=='_' || c=='-' ))
            return 0;
    }
    return 1;
}
static int is_valid_password_input(const char *p) {
    if (!p) return 0;
    size_t len = strlen(p);
    return (len >= MIN_PASSWORD_LEN && len <= MAX_PASSWORD_LEN) ? 1 : 0;
}

/* Constant-time compare */
static int constant_time_equals(const uint8_t *a, const uint8_t *b, size_t len){
    uint8_t diff = 0;
    for (size_t i=0;i<len;i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

/* User store for a single user (demo) */
typedef struct {
    char username[65];
    uint8_t salt[SALT_LEN];
    uint8_t hash[HASH_LEN];
    uint32_t iterations;
    int present;
} User;

static User g_user = {0};

static int create_user(const char *username, const char *password) {
    if (!is_valid_username(username) || !is_valid_password_input(password)) return 0;
    memset(&g_user, 0, sizeof(g_user));
    strncpy(g_user.username, username, sizeof(g_user.username)-1);
    if (random_bytes(g_user.salt, SALT_LEN) != 0) return 0;
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       g_user.salt, SALT_LEN, ITERATIONS, g_user.hash, HASH_LEN);
    g_user.iterations = ITERATIONS;
    g_user.present = 1;
    return 1;
}

static int verify_login(const char *username, const char *password) {
    if (!is_valid_username(username) || !is_valid_password_input(password)) return 0;
    if (!g_user.present) return 0;
    if (strncmp(username, g_user.username, sizeof(g_user.username)) != 0) return 0;
    uint8_t computed[HASH_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       g_user.salt, SALT_LEN, g_user.iterations, computed, HASH_LEN);
    int ok = constant_time_equals(computed, g_user.hash, HASH_LEN);
    secure_memzero(computed, sizeof(computed));
    return ok;
}

/* Public API */
const char* login(const char *username, const char *password) {
    if (verify_login(username, password)) {
        return "Redirect:/session";
    }
    return "Invalid username or password.";
}

/* Generate a strong password for tests; do not print it */
static void generate_strong_password(char *out, size_t outlen) {
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *digits = "0123456789";
    const char *special = "!@#$%^&*()-_=+[]{}:,.?/~";
    const char *all = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{}:,.?/~";
    size_t Lupper = strlen(upper), Llower = strlen(lower), Ldigits = strlen(digits), Lspecial = strlen(special), Lall = strlen(all);

    if (outlen < 9) { /* ensure room plus null */
        if (outlen > 0) out[0] = '\0';
        return;
    }
    uint8_t rnd;
    if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
    out[0] = upper[rnd % Lupper];
    if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
    out[1] = lower[rnd % Llower];
    if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
    out[2] = digits[rnd % Ldigits];
    if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
    out[3] = special[rnd % Lspecial];
    for (size_t i=4; i< (outlen-1); i++){
        if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
        out[i] = all[rnd % Lall];
    }
    out[outlen-1] = '\0';
    /* simple shuffle */
    for (size_t i=0;i<outlen-1;i++){
        if (random_bytes(&rnd, 1) != 0) { out[0] = '\0'; return; }
        size_t j = rnd % (outlen-1);
        char tmp = out[i]; out[i] = out[j]; out[j] = tmp;
    }
}

int main(void) {
    const char *user = "alice";
    char correct[17];
    generate_strong_password(correct, sizeof(correct));
    if (correct[0] == '\0') {
        printf("User setup failed\n");
        return 1;
    }
    if (!create_user(user, correct)) {
        printf("User setup failed\n");
        secure_memzero(correct, sizeof(correct));
        return 1;
    }

    const char *usernames[5] = {
        user,           /* correct */
        user,           /* wrong password */
        "bob",          /* non-existent */
        "",             /* invalid username */
        "al"            /* too short username */
    };
    const char *passwords[5] = {
        correct,
        "WrongPass!123",
        "Anything123!",
        "short",
        "Password!234"
    };

    for (int i=0;i<5;i++){
        const char *res = login(usernames[i], passwords[i]);
        printf("Test %d: %s\n", i+1, res);
    }

    secure_memzero(correct, sizeof(correct));
    return 0;
}