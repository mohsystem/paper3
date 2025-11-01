/* Chain-of-Through Steps via comments.
   Minimal SHA-256 implementation and simple auth logic in C.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

/* --- Minimal SHA-256 implementation --- */
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX_C;

static uint32_t ROTR32(uint32_t a, uint32_t b){ return (a >> b) | (a << (32-b)); }
static uint32_t CH32(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static uint32_t MAJ32(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t EP032(uint32_t x){ return ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22); }
static uint32_t EP132(uint32_t x){ return ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25); }
static uint32_t SIG032(uint32_t x){ return ROTR32(x,7) ^ ROTR32(x,18) ^ (x >> 3); }
static uint32_t SIG132(uint32_t x){ return ROTR32(x,17) ^ ROTR32(x,19) ^ (x >> 10); }

static const uint32_t K256C[64] = {
  0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
  0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
  0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
  0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
  0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
  0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
  0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
  0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static void sha256_transform_c(SHA256_CTX_C *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0,j=0; i<16; ++i, j+=4) {
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j+1] << 16) | ((uint32_t)data[j+2] << 8) | (uint32_t)data[j+3];
    }
    for (; i<64; ++i) {
        m[i] = SIG132(m[i-2]) + m[i-7] + SIG032(m[i-15]) + m[i-16];
    }
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (i=0; i<64; ++i) {
        t1 = h + EP132(e) + CH32(e,f,g) + K256C[i] + m[i];
        t2 = EP032(a) + MAJ32(a,b,c);
        h=g; g=f; f=e; e = d + t1;
        d=c; c=b; b=a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init_c(SHA256_CTX_C *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667u; ctx->state[1]=0xbb67ae85u; ctx->state[2]=0x3c6ef372u; ctx->state[3]=0xa54ff53au;
    ctx->state[4]=0x510e527fu; ctx->state[5]=0x9b05688cu; ctx->state[6]=0x1f83d9abu; ctx->state[7]=0x5be0cd19u;
}

static void sha256_update_c(SHA256_CTX_C *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0; i<len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform_c(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final_c(SHA256_CTX_C *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform_c(ctx, ctx->data);
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
    sha256_transform_c(ctx, ctx->data);
    for (i=0; i<4; ++i) {
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

static void sha256_bytes(const uint8_t* data, size_t len, uint8_t out[32]) {
    SHA256_CTX_C ctx;
    sha256_init_c(&ctx);
    sha256_update_c(&ctx, data, len);
    sha256_final_c(&ctx, out);
}

/* Constant-time compare */
static int ct_equals(const uint8_t* a, size_t alen, const uint8_t* b, size_t blen) {
    size_t len = alen > blen ? alen : blen;
    uint8_t diff = 0;
    for (size_t i=0; i<len; ++i) {
        uint8_t va = i < alen ? a[i] : 0;
        uint8_t vb = i < blen ? b[i] : 0;
        diff |= (uint8_t)(va ^ vb);
    }
    return (diff == 0) && (alen == blen);
}

/* Hash password with salt and iterations */
static void hash_password_c(const uint8_t* salt, size_t saltlen, const char* password, uint8_t out[32]) {
    size_t pwlen = password ? strlen(password) : 0;
    size_t inlen = saltlen + pwlen + saltlen;
    uint8_t* buf = (uint8_t*)malloc(inlen);
    memcpy(buf, salt, saltlen);
    if (pwlen) memcpy(buf+saltlen, password, pwlen);
    memcpy(buf+saltlen+pwlen, salt, saltlen);
    sha256_bytes(buf, inlen, out);
    uint8_t temp[64];
    for (int i=0; i<999; ++i) {
        memcpy(temp, out, 32);
        memcpy(temp+32, salt, saltlen > 32 ? 32 : saltlen);
        sha256_bytes(temp, 32 + (saltlen > 32 ? 32 : saltlen), out);
    }
    memset(buf, 0, inlen);
    free(buf);
}

/* Very simple email validation (not exhaustive) */
static int is_valid_email_c(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;
    const char* at = strchr(email, '@');
    if (!at) return 0;
    if (at == email) return 0;
    const char* dot = strrchr(at, '.');
    if (!dot) return 0;
    if (dot <= at+1) return 0;
    if (dot == email + len - 1) return 0;
    // basic allowed chars
    for (size_t i=0; i<len; ++i) {
        unsigned char c = (unsigned char)email[i];
        if (!(isalnum(c) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-' || c=='@'))
            return 0;
    }
    return 1;
}

typedef struct {
    char username[64];
    char email[256];
    uint8_t salt[16];
    uint8_t password_hash[32];
} UserC;

typedef struct {
    UserC* currentUser;
} AuthServiceC;

/* Random bytes: try /dev/urandom, else fallback to rand() */
static void random_bytes(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(out, 1, len, f);
        fclose(f);
        return;
    }
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&f);
    for (size_t i=0; i<len; ++i) {
        out[i] = (uint8_t)(rand() & 0xFF);
    }
}

/* Register default user */
static UserC register_default_user_c(const char* username, const char* email, const char* password) {
    UserC u;
    memset(&u, 0, sizeof(u));
    snprintf(u.username, sizeof(u.username), "%s", username ? username : "");
    snprintf(u.email, sizeof(u.email), "%s", email ? email : "");
    random_bytes(u.salt, sizeof(u.salt));
    hash_password_c(u.salt, sizeof(u.salt), password ? password : "", u.password_hash);
    return u;
}

static void auth_init(AuthServiceC* a) { a->currentUser = NULL; }

static int login_c(AuthServiceC* a, UserC* u, const char* username, const char* password) {
    if (!a || !u || !username || !password) return 0;
    if (strcmp(u->username, username) != 0) return 0;
    uint8_t computed[32];
    hash_password_c(u->salt, sizeof(u->salt), password, computed);
    int ok = ct_equals(u->password_hash, 32, computed, 32);
    if (ok) a->currentUser = u;
    memset(computed, 0, sizeof(computed));
    return ok;
}

static void logout_c(AuthServiceC* a) { if (a) a->currentUser = NULL; }

static const char* change_email_c(AuthServiceC* a, const char* old_email, const char* new_email, const char* confirm_password) {
    static char msg[256];
    if (!a || !a->currentUser) {
        snprintf(msg, sizeof(msg), "Error: You must be logged in to change your email.");
        return msg;
    }
    if (!old_email || !new_email || !confirm_password) {
        snprintf(msg, sizeof(msg), "Error: Missing required fields.");
        return msg;
    }
    if (!is_valid_email_c(old_email) || !is_valid_email_c(new_email)) {
        snprintf(msg, sizeof(msg), "Error: Invalid email format.");
        return msg;
    }
    if (strcmp(a->currentUser->email, old_email) != 0) {
        snprintf(msg, sizeof(msg), "Error: The provided old email does not match our records.");
        return msg;
    }
    // Case-insensitive equality
    int same = 0;
    if (strlen(old_email) == strlen(new_email)) {
        same = 1;
        for (size_t i=0; i<strlen(old_email); ++i) {
            if (tolower((unsigned char)old_email[i]) != tolower((unsigned char)new_email[i])) { same = 0; break; }
        }
    }
    if (same) {
        snprintf(msg, sizeof(msg), "Error: New email must be different from old email.");
        return msg;
    }
    uint8_t computed[32];
    hash_password_c(a->currentUser->salt, sizeof(a->currentUser->salt), confirm_password, computed);
    int ok = ct_equals(a->currentUser->password_hash, 32, computed, 32);
    memset(computed, 0, sizeof(computed));
    if (!ok) {
        snprintf(msg, sizeof(msg), "Error: Invalid credentials.");
        return msg;
    }
    snprintf(a->currentUser->email, sizeof(a->currentUser->email), "%s", new_email);
    snprintf(msg, sizeof(msg), "Success: Email changed to %s.", new_email);
    return msg;
}

static void user_snapshot_c(UserC* u, char* out, size_t outlen) {
    if (!u) { snprintf(out, outlen, "null"); return; }
    char salt_hex[16*2+1];
    for (size_t i=0; i<16; ++i) sprintf(&salt_hex[i*2], "%02x", u->salt[i]);
    salt_hex[32] = '\0';
    snprintf(out, outlen, "User(username=%s, email=%s, salt_hex=%s)", u->username, u->email, salt_hex);
}

int main(void) {
    UserC user = register_default_user_c("alice", "alice@old.com", "P@ssw0rd!");
    AuthServiceC auth;
    auth_init(&auth);

    int login1 = login_c(&auth, &user, "alice", "P@ssw0rd!");
    const char* res1 = change_email_c(&auth, "alice@old.com", "alice@new.com", "P@ssw0rd!");
    char snap[256]; user_snapshot_c(&user, snap, sizeof(snap));
    printf("T1 login=%s -> %s | %s\n", login1 ? "true" : "false", res1, snap);
    logout_c(&auth);

    const char* res2 = change_email_c(&auth, "alice@new.com", "alice@newer.com", "P@ssw0rd!");
    user_snapshot_c(&user, snap, sizeof(snap));
    printf("T2 -> %s | %s\n", res2, snap);

    int login3 = login_c(&auth, &user, "alice", "P@ssw0rd!");
    const char* res3 = change_email_c(&auth, "alice@new.com", "alice@fresh.com", "WRONGPASS");
    user_snapshot_c(&user, snap, sizeof(snap));
    printf("T3 login=%s -> %s | %s\n", login3 ? "true" : "false", res3, snap);
    logout_c(&auth);

    int login4 = login_c(&auth, &user, "alice", "P@ssw0rd!");
    const char* res4 = change_email_c(&auth, "alice@new.com", "alice@new.com", "P@ssw0rd!");
    user_snapshot_c(&user, snap, sizeof(snap));
    printf("T4 login=%s -> %s | %s\n", login4 ? "true" : "false", res4, snap);
    logout_c(&auth);

    int login5 = login_c(&auth, &user, "alice", "P@ssw0rd!");
    const char* res5 = change_email_c(&auth, "wrong@old.com", "alice@final.com", "P@ssw0rd!");
    user_snapshot_c(&user, snap, sizeof(snap));
    printf("T5 login=%s -> %s | %s\n", login5 ? "true" : "false", res5, snap);
    logout_c(&auth);

    return 0;
}