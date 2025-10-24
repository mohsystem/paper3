#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

#define DB_FILE "users_c.db"
#define SALT_LEN 16
#define HASH_ITERATIONS 100000
#define MAX_NAME_LEN 100

// -------------------- SHA-256 implementation --------------------
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
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

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]){
    uint32_t a,b,c,d,e,f,g,h,t1,t2,m[64];
    for (int i=0,j=0;i<16;i++,j+=4)
        m[i] = (uint32_t)data[j]<<24 | (uint32_t)data[j+1]<<16 | (uint32_t)data[j+2]<<8 | (uint32_t)data[j+3];
    for (int i=16;i<64;i++)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (int i=0;i<64;i++){
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
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

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]){
    uint32_t i = (uint32_t)ctx->datalen;
    if (ctx->datalen < 56){
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
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
        hash[i]    = (ctx->state[0] >> (24 - i * 8)) & 0xff;
        hash[i+4]  = (ctx->state[1] >> (24 - i * 8)) & 0xff;
        hash[i+8]  = (ctx->state[2] >> (24 - i * 8)) & 0xff;
        hash[i+12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
        hash[i+16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
        hash[i+20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
        hash[i+24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
        hash[i+28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
    }
}

// -------------------- Utilities --------------------
static void toHex(const uint8_t* data, size_t n, char* out){
    static const char* hexd="0123456789abcdef";
    for (size_t i=0;i<n;i++){
        out[i*2] = hexd[(data[i]>>4) & 0xF];
        out[i*2+1] = hexd[data[i] & 0xF];
    }
    out[n*2] = '\0';
}

static int isValidEmail(const char* email){
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;
    const char* at = strchr(email, '@');
    if (!at || at == email || at == email + len - 1) return 0;
    if (strchr(at+1, '@')) return 0;
    const char* dot = strchr(at+1, '.');
    if (!dot || dot == at+1 || dot == email + len - 1) return 0;
    for (const char* p=email; *p; ++p){
        char c = *p;
        if (!(isalnum((unsigned char)c) || c=='.' || c=='-' || c=='_' || c=='+' || c=='%' || c=='@')) return 0;
    }
    return 1;
}

static int isValidName(const char* name){
    size_t len = strlen(name);
    if (len == 0 || len > MAX_NAME_LEN) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (const char* p = name; *p; ++p){
        char c = *p;
        if (!(isalpha((unsigned char)c) || c==' ' || c=='\'' || c=='-')) return 0;
    }
    return 1;
}

static int validatePassword(const char* password, char* err, size_t errsz){
    size_t len = strlen(password);
    if (len < 8){ snprintf(err, errsz, "Error: Password too short."); return 0; }
    int lo=0, up=0, di=0, sp=0;
    for (const char* p=password; *p; ++p){
        unsigned char c = (unsigned char)*p;
        if (islower(c)) lo=1;
        else if (isupper(c)) up=1;
        else if (isdigit(c)) di=1;
        else sp=1;
    }
    if (!(lo && up && di && sp)){ snprintf(err, errsz, "Error: Password must include upper, lower, digit, and special."); return 0; }
    return 1;
}

static void trim(char* s){
    size_t len = strlen(s);
    size_t a = 0;
    while (a < len && isspace((unsigned char)s[a])) a++;
    size_t b = len;
    while (b > a && isspace((unsigned char)s[b-1])) b--;
    if (a > 0) memmove(s, s + a, b - a);
    s[b - a] = '\0';
}

static int emailExists(const char* email){
    FILE* f = fopen(DB_FILE, "r");
    if (!f) return 0;
    char line[2048];
    char lowerEmail[512]; snprintf(lowerEmail, sizeof(lowerEmail), "%s", email);
    for (char* p = lowerEmail; *p; ++p) *p = (char)tolower((unsigned char)*p);
    while (fgets(line, sizeof(line), f)){
        char* saveptr;
        char* name = strtok_r(line, "|", &saveptr);
        char* em   = strtok_r(NULL, "|", &saveptr);
        if (em){
            // make lowercase copy
            char emlow[512]; snprintf(emlow, sizeof(emlow), "%s", em);
            // strip newline
            for (char* p=emlow; *p; ++p){ if (*p=='\n' || *p=='\r') { *p=0; break; } }
            for (char* p=emlow; *p; ++p) *p = (char)tolower((unsigned char)*p);
            if (strcmp(emlow, lowerEmail) == 0){ fclose(f); return 1; }
        }
    }
    fclose(f);
    return 0;
}

static void secureRandomBytes(uint8_t* out, size_t n){
    FILE* ur = fopen("/dev/urandom", "rb");
    if (ur){
        size_t r = fread(out, 1, n, ur);
        fclose(ur);
        if (r == n) return;
    }
    srand((unsigned int)(time(NULL) ^ (uintptr_t)&out));
    for (size_t i=0;i<n;i++){
        out[i] = (uint8_t)(rand() & 0xFF);
    }
}

static void sha256_simple(const uint8_t* data, size_t len, uint8_t out[32]){
    SHA256_CTX ctx; sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

static void hashPassword(const char* password, const uint8_t* salt, size_t saltLen, int iterations, uint8_t out[32]){
    size_t pwdLen = strlen(password);
    size_t bufLen = saltLen + pwdLen;
    uint8_t* buf = (uint8_t*)malloc(bufLen);
    memcpy(buf, salt, saltLen);
    memcpy(buf + saltLen, password, pwdLen);
    sha256_simple(buf, bufLen, out);
    for (int i=1;i<iterations;i++){
        sha256_simple(out, 32, out);
    }
    // zero sensitive
    memset(buf, 0, bufLen);
    free(buf);
}

// Public API: returns static buffer message (not thread-safe)
char* registerUser(const char* name, const char* email, const char* password){
    static char msg[256];
    if (!name || !email || !password){ snprintf(msg, sizeof(msg), "Error: Null input."); return msg; }

    char nbuf[256], ebuf[512];
    snprintf(nbuf, sizeof(nbuf), "%s", name);
    snprintf(ebuf, sizeof(ebuf), "%s", email);
    trim(nbuf); trim(ebuf);
    if (!isValidName(nbuf)){ snprintf(msg, sizeof(msg), "Error: Name contains invalid characters or length."); return msg; }
    if (!isValidEmail(ebuf)){ snprintf(msg, sizeof(msg), "Error: Invalid email format."); return msg; }
    if (strchr(nbuf, '|') || strchr(ebuf, '|') || strchr(nbuf, '\n') || strchr(ebuf, '\n')){ snprintf(msg, sizeof(msg), "Error: Invalid characters in input."); return msg; }

    char perr[128];
    if (!validatePassword(password, perr, sizeof(perr))){ snprintf(msg, sizeof(msg), "%s", perr); return msg; }

    // lowercase email for uniqueness
    for (char* p=ebuf; *p; ++p) *p = (char)tolower((unsigned char)*p);

    if (emailExists(ebuf)){ snprintf(msg, sizeof(msg), "Error: Email already registered."); return msg; }

    uint8_t salt[SALT_LEN];
    secureRandomBytes(salt, SALT_LEN);

    uint8_t hash[32];
    hashPassword(password, salt, SALT_LEN, HASH_ITERATIONS, hash);

    FILE* f = fopen(DB_FILE, "a");
    if (!f){ snprintf(msg, sizeof(msg), "Error: Cannot open database file."); return msg; }

    char saltHex[SALT_LEN*2+1];
    char hashHex[32*2+1];
    toHex(salt, SALT_LEN, saltHex);
    toHex(hash, 32, hashHex);

    // Escape newline
    for (char* p=nbuf; *p; ++p){ if (*p=='\n' || *p=='\r') *p=' '; }
    for (char* p=ebuf; *p; ++p){ if (*p=='\n' || *p=='\r') *p=' '; }

    fprintf(f, "%s|%s|%s|%s\n", nbuf, ebuf, saltHex, hashHex);
    fclose(f);

    snprintf(msg, sizeof(msg), "Success: User registered.");
    return msg;
}

int main(void){
    remove(DB_FILE);

    const char* names[5] = {
        "Alice Johnson",
        "Bob O'Neil",
        "Invalid|Name",
        "Charly",
        "Dana Smith"
    };
    const char* emails[5] = {
        "alice@example.com",
        "bob.oneil@example.org",
        "bademail@@example.com",
        "alice@example.com",
        "dana.smith@example.co"
    };
    const char* passwords[5] = {
        "Str0ng!Pass",
        "An0ther$Good",
        "weakpass",
        "Str0ng!Pass",
        "Y3t@NotherGood"
    };

    for (int i=0;i<5;i++){
        printf("Test %d: %s\n", i+1, registerUser(names[i], emails[i], passwords[i]));
    }
    return 0;
}