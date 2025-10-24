#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Chain-of-Through steps embedded via secure practices and comments.

// Minimal SHA-256 implementation
typedef uint32_t u32;
typedef uint64_t u64;

static inline u32 ROTR(u32 x, int n){ return (x>>n) | (x<<(32-n)); }
static inline u32 CH(u32 x,u32 y,u32 z){ return (x & y) ^ (~x & z); }
static inline u32 MAJ(u32 x,u32 y,u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline u32 BSIG0(u32 x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline u32 BSIG1(u32 x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline u32 SSIG0(u32 x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x>>3); }
static inline u32 SSIG1(u32 x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x>>10); }

static const u32 K256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256(const unsigned char* data, size_t len, unsigned char out[32]){
    u32 H[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };
    size_t new_len = len + 1;
    while ((new_len % 64) != 56) new_len++;
    size_t total = new_len + 8;
    unsigned char* msg = (unsigned char*)malloc(total);
    if (!msg) exit(1);
    memcpy(msg, data, len);
    msg[len] = 0x80;
    memset(msg + len + 1, 0, new_len - (len + 1));
    u64 bitlen = (u64)len * 8;
    for (int i=0;i<8;i++){
        msg[new_len + 7 - i] = (unsigned char)((bitlen >> (i*8)) & 0xFF);
    }
    for (size_t off=0; off<total; off+=64){
        u32 w[64];
        for (int i=0;i<16;i++){
            w[i] = ((u32)msg[off + i*4] << 24) | ((u32)msg[off + i*4 + 1] << 16) |
                   ((u32)msg[off + i*4 + 2] << 8) | (u32)msg[off + i*4 + 3];
        }
        for (int i=16;i<64;i++){
            w[i] = SSIG1(w[i-2]) + w[i-7] + SSIG0(w[i-15]) + w[i-16];
        }
        u32 a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for (int i=0;i<64;i++){
            u32 T1 = h + BSIG1(e) + CH(e,f,g) + K256[i] + w[i];
            u32 T2 = BSIG0(a) + MAJ(a,b,c);
            h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a = T1 + T2;
        }
        H[0]+=a; H[1]+=b; H[2]+=c; H[3]+=d; H[4]+=e; H[5]+=f; H[6]+=g; H[7]+=h;
    }
    free(msg);
    for (int i=0;i<8;i++){
        out[i*4]   = (H[i] >> 24) & 0xFF;
        out[i*4+1] = (H[i] >> 16) & 0xFF;
        out[i*4+2] = (H[i] >> 8) & 0xFF;
        out[i*4+3] = H[i] & 0xFF;
    }
}

static void hmac_sha256(const unsigned char* key, size_t keylen,
                        const unsigned char* data, size_t datalen,
                        unsigned char out[32]){
    const size_t block = 64;
    unsigned char k[64];
    if (keylen > block){
        sha256(key, keylen, k);
        memset(k+32, 0, 32);
    } else {
        memcpy(k, key, keylen);
        if (keylen < block) memset(k + keylen, 0, block - keylen);
    }
    unsigned char o[64], i[64];
    for (int j=0;j<64;j++){ o[j] = k[j] ^ 0x5c; i[j] = k[j] ^ 0x36; }
    unsigned char* ibuf = (unsigned char*)malloc(64 + datalen);
    memcpy(ibuf, i, 64);
    memcpy(ibuf + 64, data, datalen);
    unsigned char inner[32];
    sha256(ibuf, 64 + datalen, inner);
    free(ibuf);

    unsigned char obuf[64 + 32];
    memcpy(obuf, o, 64);
    memcpy(obuf + 64, inner, 32);
    sha256(obuf, 96, out);
}

static void pbkdf2_hmac_sha256(const unsigned char* password, size_t passlen,
                               const unsigned char* salt, size_t saltlen,
                               int iterations, unsigned char* dk, size_t dkLen){
    size_t hLen = 32;
    size_t l = (dkLen + hLen - 1) / hLen;
    size_t r = dkLen - (l - 1) * hLen;
    for (size_t i=1;i<=l;i++){
        unsigned char ctr[4] = { (unsigned char)((i>>24)&0xFF), (unsigned char)((i>>16)&0xFF),
                                 (unsigned char)((i>>8)&0xFF), (unsigned char)(i&0xFF) };
        unsigned char* Si = (unsigned char*)malloc(saltlen + 4);
        memcpy(Si, salt, saltlen);
        memcpy(Si + saltlen, ctr, 4);
        unsigned char Ui[32];
        hmac_sha256(password, passlen, Si, saltlen+4, Ui);
        unsigned char Ti[32];
        memcpy(Ti, Ui, 32);
        for (int j=2;j<=iterations;j++){
            hmac_sha256(password, passlen, Ui, 32, Ui);
            for (int k=0;k<32;k++) Ti[k] ^= Ui[k];
        }
        size_t offset = (i-1)*hLen;
        size_t clen = (i==l)? r : hLen;
        memcpy(dk + offset, Ti, clen);
        free(Si);
    }
}

static int constTimeEq(const unsigned char* a, size_t alen, const unsigned char* b, size_t blen){
    size_t len = (alen > blen) ? alen : blen;
    unsigned char r = 0;
    for (size_t i=0;i<len;i++){
        unsigned char x = i<alen ? a[i] : 0;
        unsigned char y = i<blen ? b[i] : 0;
        r |= (unsigned char)(x ^ y);
    }
    return (r == 0) && (alen == blen);
}

static int isUpper(char c){ return c>='A' && c<='Z'; }
static int isLower(char c){ return c>='a' && c<='z'; }
static int isDigitC(char c){ return c>='0' && c<='9'; }

static int isPasswordStrong(const char* p){
    if (!p) return 0;
    size_t len = strlen(p);
    if (len < 10 || len > 100) return 0;
    int up=0,lo=0,di=0,sp=0;
    for (size_t i=0;i<len;i++){
        char c = p[i];
        if (isUpper(c)) up=1;
        else if (isLower(c)) lo=1;
        else if (isDigitC(c)) di=1;
        else sp=1;
    }
    if (!(up && lo && di && sp)) return 0;
    char lower[128];
    size_t n = len < 127 ? len : 127;
    for (size_t i=0;i<n;i++){
        char c = p[i];
        if (c>='A' && c<='Z') lower[i] = c - 'A' + 'a';
        else lower[i] = c;
    }
    lower[n] = '\0';
    const char* bad[] = {"password","12345","qwerty","letmein","admin"};
    for (int i=0;i<5;i++){
        if (strstr(lower, bad[i])) return 0;
    }
    return 1;
}

static int random_bytes(unsigned char* out, size_t n){
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(out, 1, n, f);
    fclose(f);
    return r == n;
}

static void base64url(const unsigned char* data, size_t len, char* out, size_t outsz){
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    int val = 0; int valb = -6;
    size_t j=0;
    for (size_t i=0;i<len;i++){
        val = (val<<8) + data[i];
        valb += 8;
        while (valb >= 0){
            if (j+1 < outsz) out[j++] = tbl[(val>>valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6){
        if (j+1 < outsz) out[j++] = tbl[((val << 8) >> (valb + 8)) & 0x3F];
    }
    if (j < outsz) out[j] = '\0';
}

// User store
typedef struct User {
    char email[128];
    unsigned char salt[16];
    unsigned char passhash[32];
    int iterations;
    unsigned char tokenhash[32];
    int has_token;
    time_t token_expiry;
    struct User* next;
} User;

static User* USERS = NULL;

static User* find_user(const char* email){
    User* cur = USERS;
    while (cur){
        if (strcmp(cur->email, email) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

static int createUserC(const char* email, const char* password){
    if (!email || !password) return 0;
    if (find_user(email)) return 0;
    if (!isPasswordStrong(password)) return 0;
    User* u = (User*)calloc(1, sizeof(User));
    if (!u) return 0;
    strncpy(u->email, email, sizeof(u->email)-1);
    if (!random_bytes(u->salt, 16)){
        free(u); return 0;
    }
    u->iterations = 100000;
    pbkdf2_hmac_sha256((const unsigned char*)password, strlen(password),
                       u->salt, 16, u->iterations, u->passhash, 32);
    u->has_token = 0;
    u->token_expiry = 0;
    u->next = USERS;
    USERS = u;
    return 1;
}

static char* requestResetC(const char* email){
    User* u = find_user(email);
    if (!u) return NULL;
    unsigned char tok[32];
    if (!random_bytes(tok, 32)) return NULL;
    char* tokenStr = (char*)malloc(64);
    if (!tokenStr) return NULL;
    base64url(tok, 32, tokenStr, 64);
    unsigned char th[32];
    sha256((const unsigned char*)tokenStr, strlen(tokenStr), th);
    memcpy(u->tokenhash, th, 32);
    u->has_token = 1;
    u->token_expiry = time(NULL) + 600;
    return tokenStr; // caller frees
}

static int resetPasswordC(const char* token, const char* newPassword){
    if (!token || !newPassword) return 0;
    unsigned char th[32];
    sha256((const unsigned char*)token, strlen(token), th);
    time_t now = time(NULL);
    User* cur = USERS;
    while (cur){
        if (cur->has_token && cur->token_expiry >= now &&
            constTimeEq(cur->tokenhash, 32, th, 32)){
            if (!isPasswordStrong(newPassword)) return 0;
            unsigned char salt[16];
            if (!random_bytes(salt, 16)) return 0;
            memcpy(cur->salt, salt, 16);
            pbkdf2_hmac_sha256((const unsigned char*)newPassword, strlen(newPassword),
                               cur->salt, 16, 100000, cur->passhash, 32);
            // invalidate token
            cur->has_token = 0;
            cur->token_expiry = 0;
            memset(cur->tokenhash, 0, 32);
            return 1;
        }
        cur = cur->next;
    }
    // If token matched but expired, also invalidate
    cur = USERS;
    while (cur){
        if (cur->has_token && constTimeEq(cur->tokenhash, 32, th, 32)){
            cur->has_token = 0; cur->token_expiry = 0; memset(cur->tokenhash,0,32);
            break;
        }
        cur = cur->next;
    }
    return 0;
}

int main(){
    printf("C tests:\n");
    // 1) Unknown user
    char* t1 = requestResetC("unknown@example.com");
    printf("Test1 token for unknown user should be NULL: %s\n", t1 ? t1 : "NULL");
    if (t1) free(t1);

    // 2) Create and wrong token
    int created = createUserC("alice@example.com", "OldPassw0rd!");
    printf("Test2 created user: %s\n", created ? "true" : "false");
    char* token = requestResetC("alice@example.com");
    printf("Test2 got token: %s\n", token ? "true" : "false");
    int wrong = resetPasswordC("badtoken", "NewPassw0rd!");
    printf("Test2 reset with wrong token: %s\n", wrong ? "true" : "false");

    // 3) Weak password
    int weak = token ? resetPasswordC(token, "weak") : 0;
    printf("Test3 reset with weak password: %s\n", weak ? "true" : "false");

    // 4) Successful reset
    int ok = token ? resetPasswordC(token, "N3w_Str0ngPass!") : 0;
    printf("Test4 reset with correct token and strong password: %s\n", ok ? "true" : "false");

    // 5) Reuse token; new token and reset
    int reuse = token ? resetPasswordC(token, "AnotherStr0ng!") : 0;
    printf("Test5 reuse token: %s\n", reuse ? "true" : "false");
    if (token) free(token);
    char* token2 = requestResetC("alice@example.com");
    int ok2 = token2 ? resetPasswordC(token2, "UltraStr0ng#2025") : 0;
    printf("Test5 new token reset: %s\n", ok2 ? "true" : "false");
    if (token2) free(token2);

    return 0;
}