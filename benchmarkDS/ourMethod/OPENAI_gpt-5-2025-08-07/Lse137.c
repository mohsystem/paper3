#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Minimal secure implementation for standalone: SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256
// Randomness via /dev/urandom
// Note: This is a demo; do not log secrets.

// ===== Secure zero =====
static void secure_zero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

// ===== Random bytes =====
static bool get_random_bytes(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    return r == len;
}

// ===== SHA-256 =====
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256Ctx;

static uint32_t rotr(uint32_t x, uint32_t n){ return (x>>n) | (x<<(32-n)); }
static uint32_t ch(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static uint32_t maj(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t bsig0(uint32_t x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
static uint32_t bsig1(uint32_t x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
static uint32_t ssig0(uint32_t x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
static uint32_t ssig1(uint32_t x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

static const uint32_t K256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_init(SHA256Ctx* ctx){
    ctx->datalen=0; ctx->bitlen=0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}
static void sha256_transform(SHA256Ctx* ctx, const uint8_t data[]){
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++){
        m[i] = ssig1(m[i-2]) + m[i-7] + ssig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1 = h + bsig1(e) + ch(e,f,g) + K256[i] + m[i];
        uint32_t t2 = bsig0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a = t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}
static void sha256_update(SHA256Ctx* ctx, const uint8_t* data, size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}
static void sha256_final(SHA256Ctx* ctx, uint8_t out[32]){
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i<64) ctx->data[i++]=0;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i<56) ctx->data[i++]=0;
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++){
        out[j*4+0] = (uint8_t)(ctx->state[j] >> 24);
        out[j*4+1] = (uint8_t)(ctx->state[j] >> 16);
        out[j*4+2] = (uint8_t)(ctx->state[j] >> 8);
        out[j*4+3] = (uint8_t)(ctx->state[j]);
    }
}
static void sha256_vec(const uint8_t* data, size_t len, uint8_t out[32]){
    SHA256Ctx ctx; sha256_init(&ctx);
    if (len) sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

// ===== HMAC-SHA256 =====
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* msg, size_t msglen, uint8_t out[32]){
    const size_t block = 64;
    uint8_t k[64]; memset(k, 0, sizeof(k));
    if (keylen > block){
        sha256_vec(key, keylen, k);
    } else {
        memcpy(k, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i=0;i<64;i++){ ipad[i] = 0x36 ^ k[i]; opad[i] = 0x5c ^ k[i]; }
    uint8_t inner_in[64 + msglen];
    memcpy(inner_in, ipad, 64);
    if (msglen) memcpy(inner_in+64, msg, msglen);
    uint8_t ih[32];
    sha256_vec(inner_in, 64 + msglen, ih);

    uint8_t outer_in[64 + 32];
    memcpy(outer_in, opad, 64);
    memcpy(outer_in+64, ih, 32);
    sha256_vec(outer_in, 96, out);
    secure_zero(k, sizeof(k));
    secure_zero(ipad, sizeof(ipad));
    secure_zero(opad, sizeof(opad));
    secure_zero(inner_in, sizeof(inner_in));
    secure_zero(outer_in, sizeof(outer_in));
    secure_zero(ih, sizeof(ih));
}

// ===== PBKDF2-HMAC-SHA256 =====
static void pbkdf2_hmac_sha256(const uint8_t* pw, size_t pwlen, const uint8_t* salt, size_t saltlen, uint32_t iterations, uint8_t* out, size_t dkLen){
    uint32_t blocks = (uint32_t)((dkLen + 31) / 32);
    uint8_t U[32], T[32];
    uint8_t* saltblk = (uint8_t*)malloc(saltlen + 4);
    if (!saltblk) return;
    memcpy(saltblk, salt, saltlen);
    for (uint32_t i=1;i<=blocks;i++){
        saltblk[saltlen+0] = (uint8_t)(i >> 24);
        saltblk[saltlen+1] = (uint8_t)(i >> 16);
        saltblk[saltlen+2] = (uint8_t)(i >> 8);
        saltblk[saltlen+3] = (uint8_t)(i);
        hmac_sha256(pw, pwlen, saltblk, saltlen+4, U);
        memcpy(T, U, 32);
        for (uint32_t j=1;j<iterations;j++){
            hmac_sha256(pw, pwlen, U, 32, U);
            for (int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i-1)*32;
        size_t cp = (dkLen - offset) < 32 ? (dkLen - offset) : 32;
        memcpy(out + offset, T, cp);
    }
    secure_zero(U, sizeof(U));
    secure_zero(T, sizeof(T));
    secure_zero(saltblk, saltlen+4);
    free(saltblk);
}

// ===== Helpers =====
static bool constant_time_eq(const uint8_t* a, const uint8_t* b, size_t n){
    uint8_t r = 0;
    for (size_t i=0;i<n;i++) r |= (a[i] ^ b[i]);
    return r == 0;
}
static bool constant_time_eq_str(const char* a, const char* b){
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return false;
    uint8_t r=0;
    for (size_t i=0;i<la;i++) r |= ((uint8_t)a[i] ^ (uint8_t)b[i]);
    return r==0;
}
static bool validate_identifier(const char* s, size_t maxLen){
    if (!s) return false;
    size_t n = strlen(s);
    if (n==0 || n>maxLen) return false;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c=='_' || c=='-' || c=='.' || c=='@')) return false;
    }
    return true;
}
static bool strong_password(const char* pw){
    if (!pw) return false;
    size_t n = strlen(pw);
    if (n < 12 || n > 256) return false;
    int up=0, lo=0, di=0, sp=0;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)pw[i];
        if (isupper(c)) up=1;
        else if (islower(c)) lo=1;
        else if (isdigit(c)) di=1;
        else sp=1;
    }
    return up && lo && di && sp;
}
static char hex2c(char a, char b){
    int v=0;
    if (a>='0'&&a<='9') v=(a-'0')<<4;
    else if (a>='A'&&a<='F') v=(a-'A'+10)<<4;
    else if (a>='a'&&a<='f') v=(a-'a'+10)<<4;
    if (b>='0'&&b<='9') v|=(b-'0');
    else if (b>='A'&&b<='F') v|=(b-'A'+10);
    else if (b>='a'&&b<='f') v|=(b-'a'+10);
    return (char)v;
}
static char* url_decode(const char* s){
    size_t n = strlen(s);
    char* out = (char*)malloc(n+1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<n;i++){
        if (s[i]=='%' && i+2<n && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])){
            out[j++] = hex2c(s[i+1], s[i+2]);
            i+=2;
        } else if (s[i]=='+'){
            out[j++] = ' ';
        } else {
            out[j++] = s[i];
        }
    }
    out[j]=0;
    return out;
}
typedef struct KV {
    char* key;
    char* val;
} KV;
typedef struct Params {
    KV* arr;
    size_t len;
} Params;

static void free_params(Params* p){
    if (!p) return;
    for (size_t i=0;i<p->len;i++){
        free(p->arr[i].key);
        free(p->arr[i].val);
    }
    free(p->arr);
    p->arr=NULL; p->len=0;
}
static const char* get_param(const Params* p, const char* key){
    for (size_t i=0;i<p->len;i++){
        if (strcmp(p->arr[i].key, key)==0) return p->arr[i].val;
    }
    return NULL;
}
static Params parse_params(const char* url){
    Params res; res.arr=NULL; res.len=0;
    const char* q = strchr(url, '?');
    if (!q) return res;
    q++;
    char* dup = strdup(q);
    if (!dup) return res;
    char* saveptr=NULL;
    char* token = strtok_r(dup, "&", &saveptr);
    while (token){
        char* eq = strchr(token, '=');
        if (eq && eq != token) {
            *eq = 0;
            char* kraw = token;
            char* vraw = eq+1;
            char* k = url_decode(kraw);
            char* v = url_decode(vraw);
            if (k && v){
                KV* nArr = (KV*)realloc(res.arr, (res.len+1)*sizeof(KV));
                if (nArr){
                    res.arr = nArr;
                    res.arr[res.len].key = k;
                    res.arr[res.len].val = v;
                    res.len++;
                    k = NULL; v = NULL;
                }
            }
            if (k) free(k);
            if (v) free(v);
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
    free(dup);
    return res;
}
static char* url_encode_safe(const char* s){
    static const char* safe = "-_.~";
    size_t n = strlen(s);
    // Worst case: 3x
    char* out = (char*)malloc(n*3 + 1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)s[i];
        if (isalnum(c) || strchr(safe, c)) {
            out[j++] = (char)c;
        } else {
            static const char* hex = "0123456789ABCDEF";
            out[j++]='%';
            out[j++]=hex[c>>4];
            out[j++]=hex[c&0xF];
        }
    }
    out[j]=0;
    return out;
}
static char* rand_token(size_t n){
    uint8_t* b = (uint8_t*)malloc(n);
    if (!b) return NULL;
    if (!get_random_bytes(b, n)) memset(b, 0, n);
    char* out = (char*)malloc(n*2+1);
    if (!out){ free(b); return NULL; }
    for (size_t i=0;i<n;i++){
        sprintf(out + i*2, "%02x", b[i]);
    }
    out[n*2] = 0;
    free(b);
    return out;
}

// ===== User DB =====
typedef struct {
    char username[65];
    uint8_t salt[16];
    uint8_t hash[32];
    int moderator;
    char session[129];
} User;

typedef struct Node {
    User u;
    struct Node* next;
} Node;

static Node* DB = NULL;

static void free_db(void){
    Node* cur = DB;
    while (cur){
        Node* nx = cur->next;
        secure_zero(cur, sizeof(*cur));
        free(cur);
        cur = nx;
    }
    DB = NULL;
}

static User* find_user(const char* username){
    Node* cur = DB;
    while (cur){
        if (strncmp(cur->u.username, username, 64)==0 && cur->u.username[64]=='\0') return &cur->u;
        if (strcmp(cur->u.username, username)==0) return &cur->u;
        cur = cur->next;
    }
    return NULL;
}

static void add_user(const char* username, const char* pw_env, int moderator, const char* sess_env){
    if (!validate_identifier(username, 64)) return;
    if (!pw_env || !strong_password(pw_env)) return;

    Node* node = (Node*)calloc(1, sizeof(Node));
    if (!node) return;
    strncpy(node->u.username, username, 64);
    node->u.username[64] = 0;

    if (!get_random_bytes(node->u.salt, sizeof(node->u.salt))) { free(node); return; }

    uint8_t dk[32];
    pbkdf2_hmac_sha256((const uint8_t*)pw_env, strlen(pw_env), node->u.salt, sizeof(node->u.salt), 210000, dk, 32);
    memcpy(node->u.hash, dk, 32);
    secure_zero(dk, sizeof(dk));

    if (sess_env && validate_identifier(sess_env, 128)) {
        strncpy(node->u.session, sess_env, 128);
        node->u.session[128] = 0;
    } else {
        char* tok = rand_token(16);
        if (!tok){ free(node); return; }
        strncpy(node->u.session, tok, 128);
        node->u.session[128] = 0;
        free(tok);
    }
    node->u.moderator = moderator ? 1 : 0;

    node->next = DB;
    DB = node;
}

static bool auth_mod(const char* username, const char* password, const char* session){
    if (!validate_identifier(username, 64)) return false;
    if (!password || strlen(password) < 1 || strlen(password) > 256) return false;
    if (!validate_identifier(session, 128)) return false;

    User* u = find_user(username);
    if (!u) return false;

    if (!constant_time_eq_str(u->session, session)) return false;

    uint8_t dk[32];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), u->salt, sizeof(u->salt), 210000, dk, 32);
    bool ok = constant_time_eq(dk, u->hash, 32);
    secure_zero(dk, sizeof(dk));
    return ok && (u->moderator == 1);
}

static bool check_mod(const char* url){
    if (strncmp(url, "/check_mod", 10) != 0) return false;
    Params p = parse_params(url);
    const char* username = get_param(&p, "username");
    const char* password = get_param(&p, "password");
    const char* session  = get_param(&p, "session");
    bool res = false;
    if (username && password && session) {
        res = auth_mod(username, password, session);
    }
    free_params(&p);
    return res;
}

int main(void){
    const char* mod_user = getenv("MOD_USER"); if (!mod_user) mod_user = "mod1";
    const char* mod_pass = getenv("MOD_PASS");
    const char* mod_sess = getenv("MOD_SESSION");
    const char* reg_user = getenv("USER_USER"); if (!reg_user) reg_user = "user1";
    const char* reg_pass = getenv("USER_PASS");
    const char* reg_sess = getenv("USER_SESSION");

    add_user(mod_user, mod_pass, 1, mod_sess);
    add_user(reg_user, reg_pass, 0, reg_sess);

    // Build 5 tests
    char* tests[5] = {0};
    size_t tcount = 0;

    // 1) Moderator correct (if env provided)
    User* mu = find_user(mod_user);
    if (mu && mod_pass){
        char* enc = url_encode_safe(mod_pass);
        size_t len = 64 + strlen(mod_user) + strlen(enc) + strlen(mu->session);
        tests[tcount] = (char*)malloc(len + 1);
        if (tests[tcount]) snprintf(tests[tcount], len+1, "/check_mod?username=%s&password=%s&session=%s", mod_user, enc, mu->session);
        free(enc);
    } else {
        tests[tcount] = strdup("/check_mod?username=mod1&password=WrongPass123!&session=invalid");
    }
    tcount++;

    // 2) Moderator wrong password
    if (mu){
        size_t len = 64 + strlen(mod_user) + strlen(mu->session);
        tests[tcount] = (char*)malloc(len + 64);
        if (tests[tcount]) snprintf(tests[tcount], len+64, "/check_mod?username=%s&password=WrongPass123!&session=%s", mod_user, mu->session);
    } else {
        tests[tcount] = strdup("/check_mod?username=mod1&password=WrongPass123!&session=wrong");
    }
    tcount++;

    // 3) Moderator correct password but wrong session
    if (mu && mod_pass){
        char* enc = url_encode_safe(mod_pass);
        char* bad = rand_token(4);
        size_t len = 64 + strlen(mod_user) + strlen(enc) + strlen(bad) + 16;
        tests[tcount] = (char*)malloc(len + 1);
        if (tests[tcount]) snprintf(tests[tcount], len+1, "/check_mod?username=%s&password=%s&session=bad%s", mod_user, enc, bad);
        free(enc); free(bad);
    } else {
        tests[tcount] = strdup("/check_mod?username=nouser&password=SomethingStrong1!&session=bad");
    }
    tcount++;

    // 4) Non-existent
    tests[tcount++] = strdup("/check_mod?username=ghost&password=NopePass1!&session=anything123");

    // 5) Regular user correct creds should be false
    User* ru = find_user(reg_user);
    if (ru && reg_pass){
        char* enc = url_encode_safe(reg_pass);
        size_t len = 64 + strlen(reg_user) + strlen(enc) + strlen(ru->session);
        tests[tcount] = (char*)malloc(len + 1);
        if (tests[tcount]) snprintf(tests[tcount], len+1, "/check_mod?username=%s&password=%s&session=%s", reg_user, enc, ru->session);
        free(enc);
    } else {
        tests[tcount] = strdup("/check_mod?username=user1&password=WrongPass123!&session=wrong");
    }
    tcount++;

    for (size_t i=0;i<tcount;i++){
        bool res = check_mod(tests[i]);
        printf("Test %zu: %s\n", i+1, res ? "true" : "false");
        free(tests[i]);
    }

    free_db();
    return 0;
}