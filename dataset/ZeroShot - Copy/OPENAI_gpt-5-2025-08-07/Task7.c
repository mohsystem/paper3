#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

// -------- Utility: hex --------
static char* bytes_to_hex(const uint8_t* data, size_t n) {
    static const char* hexd = "0123456789abcdef";
    char* out = (char*)malloc(n*2 + 1);
    if (!out) return NULL;
    for (size_t i=0;i<n;i++){
        out[2*i] = hexd[data[i]>>4];
        out[2*i+1] = hexd[data[i]&0xF];
    }
    out[n*2] = '\0';
    return out;
}
static int hex_to_bytes(const char* hex, uint8_t** out, size_t* outlen) {
    size_t len = strlen(hex);
    if (len % 2 != 0) return 0;
    size_t n = len/2;
    uint8_t* buf = (uint8_t*)malloc(n);
    if (!buf) return 0;
    for (size_t i=0;i<n;i++){
        char a = hex[2*i], b = hex[2*i+1];
        int hi = isdigit(a)? a - '0' : (isxdigit(a)? (tolower(a)-'a'+10) : -1);
        int lo = isdigit(b)? b - '0' : (isxdigit(b)? (tolower(b)-'a'+10) : -1);
        if (hi<0 || lo<0) { free(buf); return 0; }
        buf[i] = (uint8_t)((hi<<4) | lo);
    }
    *out = buf; *outlen = n; return 1;
}

// -------- Utility: URL encode/decode --------
static char* url_encode(const char* s) {
    size_t len = strlen(s);
    // Worst case: all encoded -> 3x
    char* out = (char*)malloc(3*len + 1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)s[i];
        if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~'){
            out[j++] = c;
        } else {
            static const char *hex="0123456789ABCDEF";
            out[j++]='%';
            out[j++]=hex[c>>4];
            out[j++]=hex[c&0xF];
        }
    }
    out[j]='\0';
    return out;
}
static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<len;){
        if (s[i]=='%' && i+2<len && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
            char a=s[i+1], b=s[i+2];
            int hi = isdigit(a)? a-'0' : (tolower(a)-'a'+10);
            int lo = isdigit(b)? b-'0' : (tolower(b)-'a'+10);
            out[j++] = (char)((hi<<4)|lo);
            i+=3;
        } else if (s[i]=='+') {
            out[j++]=' '; i++;
        } else {
            out[j++]=s[i++];
        }
    }
    out[j]='\0';
    return out;
}

// -------- SHA-256 --------
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

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

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0,j=0; i<16; ++i, j+=4)
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    for (; i<64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (i=0; i<64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0;i<len;i++){
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
    uint32_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i<56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i<64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
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
    sha256_transform(ctx, ctx->data);
    for (i=0;i<4;i++){
        hash[i]      = (ctx->state[0] >> (24 - i*8)) & 0xFF;
        hash[i+4]    = (ctx->state[1] >> (24 - i*8)) & 0xFF;
        hash[i+8]    = (ctx->state[2] >> (24 - i*8)) & 0xFF;
        hash[i+12]   = (ctx->state[3] >> (24 - i*8)) & 0xFF;
        hash[i+16]   = (ctx->state[4] >> (24 - i*8)) & 0xFF;
        hash[i+20]   = (ctx->state[5] >> (24 - i*8)) & 0xFF;
        hash[i+24]   = (ctx->state[6] >> (24 - i*8)) & 0xFF;
        hash[i+28]   = (ctx->state[7] >> (24 - i*8)) & 0xFF;
    }
}

// ---- KDF: iterated salted SHA-256 ----
static void kdf_iter_sha256(const char* password, const uint8_t* salt, size_t saltlen, int iterations, uint8_t out[32]) {
    if (iterations < 1) iterations = 1;
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, salt, saltlen);
    sha256_update(&ctx, (const uint8_t*)password, strlen(password));
    sha256_final(&ctx, out);
    for (int i=1;i<iterations;i++){
        sha256_init(&ctx);
        sha256_update(&ctx, out, 32);
        sha256_final(&ctx, out);
    }
}

// ---- Constant-time compare ----
static int ct_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i=0;i<n;i++) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

// ---- Client ----
char* build_login_request(const char* username, const char* password) {
    if (!username || !password) return strdup("");
    size_t ulen = strlen(username), plen = strlen(password);
    if (ulen == 0 || ulen > 128 || plen == 0 || plen > 256) return strdup("");
    char* ue = url_encode(username);
    char* pe = url_encode(password);
    if (!ue || !pe) { free(ue); free(pe); return strdup(""); }
    size_t outlen = strlen(ue) + strlen(pe) + 4;
    char* out = (char*)malloc(outlen);
    if (!out) { free(ue); free(pe); return strdup(""); }
    snprintf(out, outlen, "u=%s&p=%s", ue, pe);
    free(ue); free(pe);
    return out;
}

// ---- Server ----
typedef struct {
    char* username;
    int iterations;
    uint8_t* salt;
    size_t saltlen;
    uint8_t* hash;
    size_t hashlen;
} UserRecord;

static void free_user_record(UserRecord* r) {
    if (!r) return;
    free(r->username);
    if (r->salt) { memset(r->salt,0,r->saltlen); free(r->salt); }
    if (r->hash) { memset(r->hash,0,r->hashlen); free(r->hash); }
}

static int lookup_user(const char* dbPath, const char* username, UserRecord* out) {
    FILE* f = fopen(dbPath, "r");
    if (!f) return 0;
    char line[4096];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        // Trim newline
        line[strcspn(line, "\r\n")] = 0;
        // Split by ':'
        char* save=NULL;
        char* p1 = strtok_r(line, ":", &save);
        char* p2 = strtok_r(NULL, ":", &save);
        char* p3 = strtok_r(NULL, ":", &save);
        char* p4 = strtok_r(NULL, ":", &save);
        if (!p1 || !p2 || !p3 || !p4) continue;
        if (strcmp(p1, username) != 0) continue;
        int iters = atoi(p2);
        uint8_t* salt=NULL; size_t saltlen=0;
        uint8_t* hash=NULL; size_t hashlen=0;
        if (!hex_to_bytes(p3, &salt, &saltlen)) continue;
        if (!hex_to_bytes(p4, &hash, &hashlen)) { free(salt); continue; }
        out->username = strdup(username);
        out->iterations = iters;
        out->salt = salt; out->saltlen = saltlen;
        out->hash = hash; out->hashlen = hashlen;
        found = 1;
        break;
    }
    fclose(f);
    return found;
}

char* process_request(const char* request, const char* dbPath) {
    if (!request || !*request) return strdup("ERR: Malformed request");
    // parse u and p
    char* req = strdup(request);
    if (!req) return strdup("ERR: Server error");
    char* uEnc = NULL; char* pEnc = NULL;
    char* save = NULL;
    char* token = strtok_r(req, "&", &save);
    while (token) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            char* key = token;
            char* val = eq + 1;
            if (strcmp(key, "u")==0) uEnc = val;
            else if (strcmp(key, "p")==0) pEnc = val;
        }
        token = strtok_r(NULL, "&", &save);
    }
    if (!uEnc || !pEnc) { free(req); return strdup("ERR: Malformed request"); }
    char* username = url_decode(uEnc);
    char* password = url_decode(pEnc);
    if (!username || !password || !*username || !*password) {
        free(req); free(username); free(password);
        return strdup("ERR: Missing credentials");
    }
    UserRecord rec = {0};
    if (!lookup_user(dbPath, username, &rec)) {
        free(req); free(username); free(password);
        return strdup("ERR: Invalid credentials");
    }
    uint8_t derived[32];
    kdf_iter_sha256(password, rec.salt, rec.saltlen, rec.iterations, derived);
    int ok = (rec.hashlen==32) && ct_equal(derived, rec.hash, 32);
    memset(derived, 0, sizeof(derived));
    free(req); free(username); free(password);
    free_user_record(&rec);
    return strdup(ok ? "OK" : "ERR: Invalid credentials");
}

// ---- DB creation ----
static int secure_random(uint8_t* buf, size_t len) {
#ifdef _WIN32
    FILE* f = fopen("NUL", "rb"); (void)f; // placeholder
    // Fallback: use rand() - not cryptographically secure, but attempting without dependencies.
    for (size_t i=0;i<len;i++) buf[i] = (uint8_t)(rand() & 0xFF);
    return 1;
#else
    FILE* ur = fopen("/dev/urandom", "rb");
    if (!ur) return 0;
    size_t r = fread(buf, 1, len, ur);
    fclose(ur);
    return r == len;
#endif
}

int create_user_db(const char* path, const char* const* users, const char* const* passwords, size_t count, int iterations) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    for (size_t i=0;i<count;i++){
        const char* u = users[i];
        const char* p = passwords[i];
        if (!u || !p || !*u || !*p) continue;
        uint8_t salt[16];
        if (!secure_random(salt, sizeof(salt))) {
            fclose(f); return 0;
        }
        uint8_t hash[32];
        kdf_iter_sha256(p, salt, sizeof(salt), iterations, hash);
        char* saltHex = bytes_to_hex(salt, sizeof(salt));
        char* hashHex = bytes_to_hex(hash, sizeof(hash));
        if (!saltHex || !hashHex) {
            free(saltHex); free(hashHex); fclose(f); return 0;
        }
        fprintf(f, "%s:%d:%s:%s\n", u, iterations, saltHex, hashHex);
        free(saltHex); free(hashHex);
        memset(hash, 0, sizeof(hash));
    }
    fclose(f);
#ifndef _WIN32
    chmod(path, 0600);
#endif
    return 1;
}

// ---- Main with 5 tests ----
int main(void) {
    const char* dbPath = "userdb_c.txt";
    const char* users[] = {"alice","bob","charlie"};
    const char* passes[] = {"Password123!","S3cur3Pa$$","letmein"};
    if (!create_user_db(dbPath, users, passes, 3, 100000)) {
        printf("ERR: Failed to create DB\n");
        return 1;
    }
    const char* tu[] = {"alice","alice","dave","bob","bob"};
    const char* tp[] = {"Password123!","wrong","doesntmatter","","S3cur3Pa$$"};
    for (int i=0;i<5;i++){
        char* req = build_login_request(tu[i], tp[i]);
        char* resp = process_request(req, dbPath);
        printf("Test %d -> %s\n", i+1, resp);
        free(req); free(resp);
    }
    remove(dbPath);
    return 0;
}