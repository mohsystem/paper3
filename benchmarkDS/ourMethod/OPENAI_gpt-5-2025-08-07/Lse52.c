#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    unsigned char salt[16];
    unsigned char hash[32];
    uint32_t iterations;
} Credential;

typedef struct {
    char username[33];
    int has_user;
} Session;

static const char* index_page(void) {
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body><h1>Welcome</h1><a href=\"/login\">Login</a></body></html>";
}

static const char* login_page(void) {
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body><h1>Login</h1><form action=\"/do_login\" method=\"post\">Username: <input type=\"text\" name=\"username\" maxlength=\"32\" /><br/>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" /><br/><button type=\"submit\">Login</button></form></body></html>";
}

static const char* secret_page(const Session* s) {
    if (s && s->has_user) {
        return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Secret settings</body></html>";
    }
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Unauthorized</body></html>";
}

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        int ok = (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= '0' && c <= '9') ||
                 c == '_' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

static int is_valid_password(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    return len >= 12 && len <= 128;
}

/* ---- Minimal SHA-256 implementation ---- */
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

#define ROTLEFT(a,b)  (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z)     (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z)    (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)        (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)        (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)       (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)       (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0,j=0; i<16; ++i, j+=4)
        m[i] = ( (uint32_t)data[j] << 24 ) | ( (uint32_t)data[j+1] << 16 ) | ( (uint32_t)data[j+2] << 8 ) | ( (uint32_t)data[j+3] );
    for ( ; i<64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (i=0; i<64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a=t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667;
    ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372;
    ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f;
    ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab;
    ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0; i<len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
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
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8 );
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i=0; i<4; ++i) {
        for (int j=0; j<8; ++j) {
            hash[i + (j*4)] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xFF);
        }
    }
}

/* HMAC-SHA256 */
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]) {
    uint8_t k_ipad[64]; memset(k_ipad, 0x36, sizeof(k_ipad));
    uint8_t k_opad[64]; memset(k_opad, 0x5c, sizeof(k_opad));
    uint8_t tk[32];
    const uint8_t* used_key = key;
    size_t used_len = keylen;

    if (keylen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        used_key = tk;
        used_len = 32;
    }

    uint8_t kopad[64]; memset(kopad, 0, sizeof(kopad));
    uint8_t kipad[64]; memset(kipad, 0, sizeof(kipad));
    memcpy(kopad, used_key, used_len);
    memcpy(kipad, used_key, used_len);

    for (size_t i=0;i<64;i++){ kopad[i] ^= 0x5c; kipad[i] ^= 0x36; }

    SHA256_CTX ctx;
    uint8_t inner_hash[32];

    sha256_init(&ctx);
    sha256_update(&ctx, kipad, 64);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, kopad, 64);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    memset(tk, 0, sizeof(tk));
    memset(kopad, 0, sizeof(kopad));
    memset(kipad, 0, sizeof(kipad));
}

/* PBKDF2-HMAC-SHA256 (dkLen bytes) */
static void pbkdf2_hmac_sha256(const char* password, const uint8_t* salt, size_t saltlen, uint32_t iterations, uint8_t* out, size_t dkLen) {
    const size_t hLen = 32;
    size_t l = (dkLen + hLen - 1) / hLen;
    size_t r = dkLen - (l - 1) * hLen;

    uint8_t* asalt = (uint8_t*)malloc(saltlen + 4);
    if (!asalt) exit(1);
    memcpy(asalt, salt, saltlen);

    uint8_t* U = (uint8_t*)malloc(hLen);
    uint8_t* T = (uint8_t*)malloc(hLen);
    if (!U || !T) exit(1);

    for (size_t i = 1; i <= l; i++) {
        asalt[saltlen]   = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen+3] = (uint8_t)(i & 0xff);

        hmac_sha256((const uint8_t*)password, strlen(password), asalt, saltlen + 4, U);
        memcpy(T, U, hLen);

        for (uint32_t j = 1; j < iterations; j++) {
            hmac_sha256((const uint8_t*)password, strlen(password), U, hLen, U);
            for (size_t k = 0; k < hLen; k++) {
                T[k] ^= U[k];
            }
        }

        size_t clen = (i == l) ? r : hLen;
        memcpy(out + (i - 1) * hLen, T, clen);
    }

    memset(U, 0, hLen);
    memset(T, 0, hLen);
    free(U);
    free(T);
    memset(asalt, 0, saltlen + 4);
    free(asalt);
}

static int constant_time_equals(const unsigned char* a, const unsigned char* b, size_t n) {
    unsigned char diff = 0;
    for (size_t i = 0; i < n; i++) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

static void gen_salt(unsigned char* out, size_t n) {
    /* Use /dev/urandom if available, else fallback to rand seeded by time (not ideal but better than nothing for demo) */
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t rd = fread(out, 1, n, f);
        fclose(f);
        if (rd == n) return;
    }
    /* Fallback */
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < n; i++) out[i] = (unsigned char)(rand() & 0xFF);
}

static void create_credential(const char* password, Credential* cred) {
    cred->iterations = 210000;
    gen_salt(cred->salt, sizeof(cred->salt));
    pbkdf2_hmac_sha256(password, cred->salt, sizeof(cred->salt), cred->iterations, cred->hash, sizeof(cred->hash));
}

static int verify_password(const char* password, const Credential* cred) {
    unsigned char out[32];
    pbkdf2_hmac_sha256(password, cred->salt, sizeof(cred->salt), cred->iterations, out, sizeof(out));
    int ok = constant_time_equals(out, cred->hash, sizeof(out));
    memset(out, 0, sizeof(out));
    return ok;
}

/* do_login: sets session username on success, returns message */
static const char* do_login(Session* session,
                            const char* username,
                            const char* password,
                            const char* stored_username,
                            const Credential* stored_cred) {
    if (!session || !username || !password || !stored_username || !stored_cred) return "ERROR:Invalid session/store";
    if (!is_valid_username(username)) return "ERROR:Invalid username";
    if (!is_valid_password(password)) return "ERROR:Invalid password";
    if (strcmp(username, stored_username) != 0) return "ERROR:Invalid credentials";
    if (!verify_password(password, stored_cred)) return "ERROR:Invalid credentials";
    snprintf(session->username, sizeof(session->username), "%s", username);
    session->has_user = 1;
    return "REDIRECT:/secret";
}

/* Random password generator for tests */
static void random_password(char* out, size_t n) {
    const char* alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*()-_=+";
    size_t alen = strlen(alphabet);
    FILE* f = fopen("/dev/urandom", "rb");
    for (size_t i = 0; i < n; i++) {
        unsigned char b = 0;
        if (f && fread(&b, 1, 1, f) == 1) {
            out[i] = alphabet[b % alen];
        } else {
            out[i] = alphabet[rand() % (int)alen];
        }
    }
    if (f) fclose(f);
    out[n] = '\0';
}

int main(void) {
    const char* stored_username = "alice";
    char password[64];
    random_password(password, 14); /* not printed */

    Credential cred;
    create_credential(password, &cred);

    Session session;
    memset(&session, 0, sizeof(session));

    int passed = 0;
    int total = 5;

    /* Test 1: Success */
    const char* r1 = do_login(&session, stored_username, password, stored_username, &cred);
    int t1 = (strcmp(r1, "REDIRECT:/secret") == 0) &&
             session.has_user &&
             strcmp(session.username, stored_username) == 0 &&
             strstr(secret_page(&session), "Secret settings") != NULL;
    if (t1) passed++;
    memset(&session, 0, sizeof(session));

    /* Test 2: Wrong password */
    const char* r2 = do_login(&session, stored_username, "WrongPassword123!", stored_username, &cred);
    int t2 = (strncmp(r2, "ERROR:", 6) == 0) && !session.has_user;
    if (t2) passed++;

    /* Test 3: Wrong username */
    const char* r3 = do_login(&session, "bob", password, stored_username, &cred);
    int t3 = (strncmp(r3, "ERROR:", 6) == 0) && !session.has_user;
    if (t3) passed++;

    /* Test 4: Invalid username format */
    const char* r4 = do_login(&session, "bad<user>", password, stored_username, &cred);
    int t4 = (strcmp(r4, "ERROR:Invalid username") == 0);
    if (t4) passed++;

    /* Test 5: Short password */
    const char* r5 = do_login(&session, stored_username, "shortPwd1!", stored_username, &cred);
    int t5 = (strcmp(r5, "ERROR:Invalid password") == 0);
    if (t5) passed++;

    printf("Index page sample: %s\n", strstr(index_page(), "Login") ? "OK" : "FAIL");
    printf("Login page sample: %s\n", strstr(login_page(), "form") ? "OK" : "FAIL");
    printf("Tests passed: %d/%d\n", passed, total);

    /* Zero sensitive buffers */
#if defined(__STDC_LIB_EXT1__)
    memset_s(password, sizeof(password), 0, sizeof(password));
#else
    volatile unsigned char *p = (volatile unsigned char *)password;
    for (size_t i = 0; i < sizeof(password); i++) p[i] = 0;
#endif
    return 0;
}