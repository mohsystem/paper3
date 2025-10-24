#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

// Minimal SHA-256 implementation (public domain)
typedef uint32_t u32;
typedef uint64_t u64;

static u32 rotr(u32 x, u32 n){ return (x >> n) | (x << (32 - n)); }
static u32 Ch(u32 x, u32 y, u32 z){ return (x & y) ^ (~x & z); }
static u32 Maj(u32 x, u32 y, u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
static u32 Sigma0(u32 x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static u32 Sigma1(u32 x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static u32 sigma0(u32 x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static u32 sigma1(u32 x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static const u32 K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

typedef struct {
    u32 h[8];
    unsigned char buf[64];
    size_t buf_len;
    u64 bits;
} SHA256_CTX;

static void sha256_init(SHA256_CTX* ctx){
    ctx->h[0]=0x6a09e667ul; ctx->h[1]=0xbb67ae85ul; ctx->h[2]=0x3c6ef372ul; ctx->h[3]=0xa54ff53aul;
    ctx->h[4]=0x510e527ful; ctx->h[5]=0x9b05688cul; ctx->h[6]=0x1f83d9abul; ctx->h[7]=0x5be0cd19ul;
    ctx->buf_len = 0;
    ctx->bits = 0;
}

static void sha256_process_block(SHA256_CTX* ctx, const unsigned char block[64]){
    u32 w[64];
    for (int i=0;i<16;i++){
        w[i] = ((u32)block[4*i]<<24)|((u32)block[4*i+1]<<16)|((u32)block[4*i+2]<<8)|((u32)block[4*i+3]);
    }
    for (int i=16;i<64;i++){
        w[i] = sigma1(w[i-2]) + w[i-7] + sigma0(w[i-15]) + w[i-16];
    }
    u32 a=ctx->h[0], b=ctx->h[1], c=ctx->h[2], d=ctx->h[3], e=ctx->h[4], f=ctx->h[5], g=ctx->h[6], h=ctx->h[7];
    for (int i=0;i<64;i++){
        u32 t1 = h + Sigma1(e) + Ch(e,f,g) + K256[i] + w[i];
        u32 t2 = Sigma0(a) + Maj(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a = t1 + t2;
    }
    ctx->h[0]+=a; ctx->h[1]+=b; ctx->h[2]+=c; ctx->h[3]+=d; ctx->h[4]+=e; ctx->h[5]+=f; ctx->h[6]+=g; ctx->h[7]+=h;
}

static void sha256_update(SHA256_CTX* ctx, const unsigned char* data, size_t len){
    ctx->bits += (u64)len * 8;
    size_t i = 0;
    if (ctx->buf_len){
        while (i < len && ctx->buf_len < 64){
            ctx->buf[ctx->buf_len++] = data[i++];
        }
        if (ctx->buf_len == 64){
            sha256_process_block(ctx, ctx->buf);
            ctx->buf_len = 0;
        }
    }
    while (i + 64 <= len){
        sha256_process_block(ctx, data + i);
        i += 64;
    }
    while (i < len){
        ctx->buf[ctx->buf_len++] = data[i++];
    }
}

static void sha256_final(SHA256_CTX* ctx, unsigned char out[32]){
    unsigned char pad = 0x80;
    sha256_update(ctx, &pad, 1);
    unsigned char zero = 0x00;
    while ((ctx->buf_len % 64) != 56){
        sha256_update(ctx, &zero, 1);
    }
    unsigned char lenb[8];
    for (int i=0;i<8;i++) lenb[7-i] = (unsigned char)((ctx->bits >> (i*8)) & 0xFF);
    sha256_update(ctx, lenb, 8);
    for (int i=0;i<8;i++){
        out[4*i]   = (unsigned char)((ctx->h[i] >> 24) & 0xFF);
        out[4*i+1] = (unsigned char)((ctx->h[i] >> 16) & 0xFF);
        out[4*i+2] = (unsigned char)((ctx->h[i] >> 8) & 0xFF);
        out[4*i+3] = (unsigned char)(ctx->h[i] & 0xFF);
    }
}

static void sha256(const unsigned char* data, size_t len, unsigned char out[32]){
    SHA256_CTX ctx; sha256_init(&ctx); sha256_update(&ctx, data, len); sha256_final(&ctx, out);
}

// Utils
static int is_valid_username(const char* u){
    size_t n = strlen(u);
    if (n == 0 || n > 64) return 0;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

static char* escape_str(const char* s){
    size_t n = strlen(s);
    // worst case every char escaped
    char* out = (char*)malloc(n*2 + 1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<n;i++){
        char c = s[i];
        if (c == '\\' || c == ';' || c == '=') out[j++]='\\';
        out[j++]=c;
    }
    out[j]='\0';
    return out;
}

static char* unescape_str(const char* s){
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    size_t j=0; int esc=0;
    for (size_t i=0;i<n;i++){
        char c = s[i];
        if (esc){ out[j++]=c; esc=0; }
        else if (c=='\\') esc=1;
        else out[j++]=c;
    }
    out[j]='\0';
    return out;
}

typedef struct {
    char* op;
    char* user;
    char* pass;
} KVReq;

static void free_kv(KVReq* kv){
    if (!kv) return;
    free(kv->op);
    free(kv->user);
    free(kv->pass);
}

static KVReq parse_kv(const char* s){
    KVReq kv = {0};
    size_t n = strlen(s);
    char* key = (char*)malloc(n+1);
    char* val = (char*)malloc(n+1);
    if (!key || !val) { free(key); free(val); return kv; }
    size_t ki=0, vi=0;
    int inKey=1, esc=0;
    for (size_t i=0;i<=n;i++){
        char c = s[i];
        if (i == n) c = ';'; // force flush last
        if (esc){
            if (inKey) key[ki++]=c; else val[vi++]=c;
            esc=0;
        } else {
            if (c=='\\') esc=1;
            else if (c=='=' && inKey) inKey=0;
            else if (c==';'){
                key[ki]='\0'; val[vi]='\0';
                if (ki>0){
                    if (strcmp(key,"op")==0) { kv.op = strdup(val); }
                    else if (strcmp(key,"user")==0) { kv.user = strdup(val); }
                    else if (strcmp(key,"pass")==0) { kv.pass = strdup(val); }
                }
                ki=vi=0; inKey=1;
            } else {
                if (inKey) key[ki++]=c; else val[vi++]=c;
            }
        }
    }
    free(key); free(val);
    return kv;
}

static void to_hex(const unsigned char* in, size_t len, char* out){
    static const char* hex="0123456789abcdef";
    for (size_t i=0;i<len;i++){
        unsigned v = in[i];
        out[2*i] = hex[v>>4];
        out[2*i+1] = hex[v & 0xF];
    }
    out[2*len]='\0';
}

static unsigned char hexval(char c){
    if (c>='0'&&c<='9') return (unsigned char)(c-'0');
    if (c>='a'&&c<='f') return (unsigned char)(c-'a'+10);
    if (c>='A'&&c<='F') return (unsigned char)(c-'A'+10);
    return 255;
}

static int from_hex(const char* s, unsigned char* out, size_t outlen){
    size_t n = strlen(s);
    if (n%2) return 0;
    if (outlen < n/2) return 0;
    for (size_t i=0;i<n/2;i++){
        unsigned char hi = hexval(s[2*i]);
        unsigned char lo = hexval(s[2*i+1]);
        if (hi==255||lo==255) return 0;
        out[i] = (unsigned char)((hi<<4)|lo);
    }
    return 1;
}

static void concat2(const unsigned char* a, size_t la, const unsigned char* b, size_t lb, unsigned char* out){
    memcpy(out, a, la);
    memcpy(out+la, b, lb);
}

static char* hash_password(const char* password, const unsigned char* salt, size_t saltlen, int iterations){
    size_t plen = strlen(password);
    unsigned char* buf = (unsigned char*)malloc(saltlen + plen);
    if (!buf) return NULL;
    concat2(salt, saltlen, (const unsigned char*)password, plen, buf);
    unsigned char h[32];
    sha256(buf, saltlen + plen, h);
    free(buf);
    unsigned char* tmp = (unsigned char*)malloc(32 + saltlen);
    if (!tmp) return NULL;
    for (int i=0;i<iterations;i++){
        concat2(h, 32, salt, saltlen, tmp);
        sha256(tmp, 32 + saltlen, h);
    }
    free(tmp);
    char* hex = (char*)malloc(65);
    if (!hex) return NULL;
    to_hex(h, 32, hex);
    return hex;
}

static int constant_time_eq(const char* a, const char* b){
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i=0;i<la;i++) diff |= (unsigned char)(a[i]^b[i]);
    return diff == 0;
}

static int secure_random_bytes(unsigned char* out, size_t len){
#if defined(_WIN32)
    // Fallback weak RNG for demo; in production use BCryptGenRandom
    srand((unsigned)time(NULL));
    for (size_t i=0;i<len;i++) out[i] = (unsigned char)(rand() & 0xFF);
    return 1;
#else
    FILE* f = fopen("/dev/urandom","rb");
    if (!f) return 0;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    return r == len;
#endif
}

// Client: build request
static char* build_login_request(const char* username, const char* password){
    if (!username || !password) return strdup("");
    if (strlen(username) > 128 || strlen(password) > 256) return strdup("");
    char* ue = escape_str(username);
    char* pe = escape_str(password);
    size_t len = strlen("op=login;user=") + strlen(ue) + strlen(";pass=") + strlen(pe) + 1;
    char* req = (char*)malloc(len);
    if (!req){ free(ue); free(pe); return strdup(""); }
    snprintf(req, len, "op=login;user=%s;pass=%s", ue, pe);
    free(ue); free(pe);
    return req;
}

// Server: authenticate against store
static int authenticate(const char* user_store_path, const char* username, const char* password){
    FILE* f = fopen(user_store_path, "r");
    if (!f) return 0;
    char line[1024];
    int ok = 0;
    while (fgets(line, sizeof(line), f)){
        size_t l = strlen(line);
        while (l>0 && (line[l-1]=='\n' || line[l-1]=='\r')) line[--l]='\0';
        if (l==0) continue;
        char* u = strtok(line, ":");
        char* saltHex = strtok(NULL, ":");
        char* iterStr = strtok(NULL, ":");
        char* stored = strtok(NULL, ":");
        if (!u || !saltHex || !iterStr || !stored) continue;
        if (strcmp(u, username) != 0) continue;
        int iters = atoi(iterStr);
        if (iters < 1000 || iters > 200000) { ok = 0; break; }
        unsigned char salt[64];
        size_t saltlen = strlen(saltHex)/2;
        if (!from_hex(saltHex, salt, sizeof(salt))) { ok = 0; break; }
        char* h = hash_password(password, salt, saltlen, iters);
        if (!h){ ok = 0; break; }
        ok = constant_time_eq(stored, h);
        free(h);
        break;
    }
    fclose(f);
    return ok;
}

// Server: process request
static char* process_request(const char* user_store_path, const char* request){
    if (!request || strlen(request) > 4096) return strdup("ERR:invalid request size");
    KVReq kv = parse_kv(request);
    if (!kv.op || strcmp(kv.op, "login") != 0){
        free_kv(&kv);
        return strdup("ERR:unsupported op");
    }
    if (!kv.user || !kv.pass){
        free_kv(&kv);
        return strdup("ERR:missing fields");
    }
    char* user = unescape_str(kv.user);
    char* pass = unescape_str(kv.pass);
    free_kv(&kv);
    if (!user || !pass){
        free(user); free(pass);
        return strdup("ERR:server io error");
    }
    if (!is_valid_username(user)){
        free(user); free(pass);
        return strdup("ERR:invalid username");
    }
    int ok = authenticate(user_store_path, user, pass);
    size_t outlen = 16 + strlen(user) + 1;
    char* out = (char*)malloc(outlen);
    if (!out){ free(user); free(pass); return strdup("ERR:server io error"); }
    snprintf(out, outlen, "%s%s", ok ? "OK:Welcome " : "ERR:authentication failed", ok ? user : "");
    free(user); free(pass);
    return out;
}

// Admin: create or update user
static int create_or_update_user(const char* user_store_path, const char* username, const char* password){
    if (!is_valid_username(username)) return 0;
    int iterations = 10000;
    unsigned char salt[16];
    if (!secure_random_bytes(salt, sizeof(salt))) return 0;
    char* hash = hash_password(password, salt, sizeof(salt), iterations);
    if (!hash) return 0;
    // Load existing into memory
    FILE* f = fopen(user_store_path, "r");
    typedef struct { char* line; char* user; } Rec;
    Rec* recs = NULL; size_t rc=0;
    if (f){
        char buf[1024];
        while (fgets(buf, sizeof(buf), f)){
            size_t l = strlen(buf);
            while (l>0 && (buf[l-1]=='\n'||buf[l-1]=='\r')) buf[--l]='\0';
            if (l==0) continue;
            char* line = strdup(buf);
            if (!line) { fclose(f); free(hash); return 0; }
            char* cp = strdup(buf);
            char* u = strtok(cp, ":");
            recs = (Rec*)realloc(recs, sizeof(Rec)*(rc+1));
            recs[rc].line = line;
            recs[rc].user = u ? strdup(u) : strdup("");
            free(cp);
            rc++;
        }
        fclose(f);
    }
    // Update or add
    int found = 0;
    char saltHex[33];
    to_hex(salt, sizeof(salt), saltHex);
    char newline[256];
    snprintf(newline, sizeof(newline), "%s:%s:%d:%s", username, saltHex, iterations, hash);
    for (size_t i=0;i<rc;i++){
        if (strcmp(recs[i].user, username)==0){
            free(recs[i].line);
            recs[i].line = strdup(newline);
            found = 1;
            break;
        }
    }
    if (!found){
        recs = (Rec*)realloc(recs, sizeof(Rec)*(rc+1));
        recs[rc].line = strdup(newline);
        recs[rc].user = strdup(username);
        rc++;
    }
    // Write tmp
    char tmppath[512];
    snprintf(tmppath, sizeof(tmppath), "%s.tmp", user_store_path);
    FILE* out = fopen(tmppath, "w");
    if (!out){
        for (size_t i=0;i<rc;i++){ free(recs[i].line); free(recs[i].user); }
        free(recs); free(hash);
        return 0;
    }
    for (size_t i=0;i<rc;i++){
        fprintf(out, "%s\n", recs[i].line);
    }
    fclose(out);
    // Replace
    remove(user_store_path);
    rename(tmppath, user_store_path);
    for (size_t i=0;i<rc;i++){ free(recs[i].line); free(recs[i].user); }
    free(recs);
    free(hash);
    return 1;
}

int main(){
    const char* store = "users_c.db";
    create_or_update_user(store, "alice", "Wonder@123");
    create_or_update_user(store, "bob", "Builder#456");

    char* r1 = build_login_request("alice", "Wonder@123");
    char* r2 = build_login_request("alice", "wrongpass");
    char* r3 = build_login_request("charlie", "Anything1!");
    char* r4 = build_login_request("bob", "Builder#456");
    char* r5 = build_login_request("bad;user", "str@nge=pass");

    char* resp1 = process_request(store, r1);
    char* resp2 = process_request(store, r2);
    char* resp3 = process_request(store, r3);
    char* resp4 = process_request(store, r4);
    char* resp5 = process_request(store, r5);

    printf("Case 1: %s\n", resp1);
    printf("Case 2: %s\n", resp2);
    printf("Case 3: %s\n", resp3);
    printf("Case 4: %s\n", resp4);
    printf("Case 5: %s\n", resp5);

    free(r1); free(r2); free(r3); free(r4); free(r5);
    free(resp1); free(resp2); free(resp3); free(resp4); free(resp5);
    return 0;
}