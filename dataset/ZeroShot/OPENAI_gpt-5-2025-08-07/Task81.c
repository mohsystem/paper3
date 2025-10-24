#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* SHA-256 implementation (public domain style) */
typedef struct {
    uint32_t h[8];
    uint64_t bits;
    uint8_t buf[64];
    size_t buf_len;
} sha256_ctx;

static uint32_t rotr32(uint32_t x, uint32_t n){ return (x>>n)|(x<<(32-n)); }
static uint32_t Ch(uint32_t x,uint32_t y,uint32_t z){return (x&y)^(~x&z);}
static uint32_t Maj(uint32_t x,uint32_t y,uint32_t z){return (x&y)^(x&z)^(y&z);}
static uint32_t BSIG0(uint32_t x){return rotr32(x,2)^rotr32(x,13)^rotr32(x,22);}
static uint32_t BSIG1(uint32_t x){return rotr32(x,6)^rotr32(x,11)^rotr32(x,25);}
static uint32_t SSIG0(uint32_t x){return rotr32(x,7)^rotr32(x,18)^(x>>3);}
static uint32_t SSIG1(uint32_t x){return rotr32(x,17)^rotr32(x,19)^(x>>10);}

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

static void sha256_init(sha256_ctx* c){
    c->h[0]=0x6a09e667; c->h[1]=0xbb67ae85; c->h[2]=0x3c6ef372; c->h[3]=0xa54ff53a;
    c->h[4]=0x510e527f; c->h[5]=0x9b05688c; c->h[6]=0x1f83d9ab; c->h[7]=0x5be0cd19;
    c->bits=0; c->buf_len=0;
}

static void sha256_transform(sha256_ctx* c, const uint8_t block[64]){
    uint32_t w[64];
    for(int i=0;i<16;i++){
        w[i] = ((uint32_t)block[i*4+0]<<24) | ((uint32_t)block[i*4+1]<<16) |
               ((uint32_t)block[i*4+2]<<8) | ((uint32_t)block[i*4+3]);
    }
    for(int i=16;i<64;i++){
        w[i] = SSIG1(w[i-2]) + w[i-7] + SSIG0(w[i-15]) + w[i-16];
    }
    uint32_t a=c->h[0],b=c->h[1],cc=c->h[2],d=c->h[3],e=c->h[4],f=c->h[5],g=c->h[6],h=c->h[7];
    for(int i=0;i<64;i++){
        uint32_t t1 = h + BSIG1(e) + Ch(e,f,g) + K256[i] + w[i];
        uint32_t t2 = BSIG0(a) + Maj(a,b,cc);
        h=g; g=f; f=e; e=d + t1;
        d=cc; cc=b; b=a; a=t1 + t2;
    }
    c->h[0]+=a; c->h[1]+=b; c->h[2]+=cc; c->h[3]+=d;
    c->h[4]+=e; c->h[5]+=f; c->h[6]+=g; c->h[7]+=h;
}

static void sha256_update(sha256_ctx* c, const uint8_t* data, size_t len){
    c->bits += (uint64_t)len * 8;
    size_t i=0;
    if (c->buf_len>0){
        size_t toFill = 64 - c->buf_len;
        if (toFill > len) toFill = len;
        memcpy(c->buf + c->buf_len, data, toFill);
        c->buf_len += toFill; i += toFill;
        if (c->buf_len == 64){
            sha256_transform(c, c->buf);
            c->buf_len = 0;
        }
    }
    for(; i+64<=len; i+=64){
        sha256_transform(c, data + i);
    }
    if (i<len){
        memcpy(c->buf, data+i, len-i);
        c->buf_len = len - i;
    }
}

static void sha256_final(sha256_ctx* c, uint8_t out[32]){
    uint8_t pad[64]; memset(pad, 0, sizeof(pad)); pad[0]=0x80;
    size_t padLen = (c->buf_len < 56) ? (56 - c->buf_len) : (64 + 56 - c->buf_len);
    sha256_update(c, pad, padLen);
    uint8_t lenbe[8];
    for(int i=0;i<8;i++){
        lenbe[7-i] = (uint8_t)((c->bits >> (i*8)) & 0xFF);
    }
    sha256_update(c, lenbe, 8);
    for(int i=0;i<8;i++){
        out[i*4+0] = (uint8_t)((c->h[i]>>24)&0xFF);
        out[i*4+1] = (uint8_t)((c->h[i]>>16)&0xFF);
        out[i*4+2] = (uint8_t)((c->h[i]>>8)&0xFF);
        out[i*4+3] = (uint8_t)((c->h[i])&0xFF);
    }
}

/* Hex helpers */
static void sanitize_hex(const char* in, char* out, size_t out_sz){
    size_t j=0;
    for(size_t i=0; in[i]!=0 && j+1<out_sz; ++i){
        char c = in[i];
        if (isxdigit((unsigned char)c)){
            out[j++] = c;
        }
    }
    out[j]=0;
}

static int hex_to_bytes(const char* hex, uint8_t* out, size_t out_sz){
    size_t n = strlen(hex);
    if (n%2!=0) return 0;
    if (out_sz < n/2) return 0;
    auto val = [](char c)->int{
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return 10 + (c-'a');
        if(c>='A'&&c<='F') return 10 + (c-'A');
        return -1;
    };
    for(size_t i=0,j=0;i<n;i+=2,++j){
        int hi=val(hex[i]), lo=val(hex[i+1]);
        if(hi<0||lo<0) return 0;
        out[j] = (uint8_t)((hi<<4)|lo);
    }
    return (int)(n/2);
}

/* Constant-time compare */
static int constant_time_eq(const uint8_t* a, const uint8_t* b, size_t n){
    uint8_t acc=0;
    for(size_t i=0;i<n;i++) acc |= (uint8_t)(a[i]^b[i]);
    return acc==0;
}

/* Base64 decode (strict, ignores whitespace) */
static int b64_val(int c){
    if (c>='A'&&c<='Z') return c-'A';
    if (c>='a'&&c<='z') return 26 + (c-'a');
    if (c>='0'&&c<='9') return 52 + (c-'0');
    if (c=='+') return 62;
    if (c=='/') return 63;
    if (c=='=') return -2; // padding
    if (c=='\r'||c=='\n'||c=='\t'||c==' ') return -3; // whitespace
    return -1; // invalid
}

static int base64_decode_strict(const char* in, uint8_t** out, size_t* out_len){
    size_t in_len = strlen(in);
    int *vals = (int*)malloc(sizeof(int)*in_len);
    if (!vals) return 0;
    size_t vlen=0;
    for(size_t i=0;i<in_len;i++){
        int v = b64_val((unsigned char)in[i]);
        if (v==-3) continue; // skip whitespace
        if (v==-1){ free(vals); return 0; }
        vals[vlen++] = v;
    }
    if (vlen==0){ *out=NULL; *out_len=0; free(vals); return 1; }
    if (vlen%4!=0){ free(vals); return 0; }
    size_t quad = vlen/4;
    size_t cap = quad*3;
    uint8_t *buf = (uint8_t*)malloc(cap?cap:1);
    if (!buf){ free(vals); return 0; }
    size_t pos=0, idx=0;
    for(size_t i=0;i<quad;i++){
        int a=vals[idx++], b=vals[idx++], c=vals[idx++], d=vals[idx++];
        if (a<0||b<0){ free(vals); free(buf); return 0; }
        uint32_t triple = ((uint32_t)a<<18)|((uint32_t)b<<12);
        buf[pos++] = (uint8_t)((triple>>16)&0xFF);
        if (c>=0){
            triple |= ((uint32_t)c<<6);
            buf[pos++] = (uint8_t)((triple>>8)&0xFF);
            if (d>=0){
                triple |= (uint32_t)d;
                buf[pos++] = (uint8_t)(triple&0xFF);
            } else {
                // d is padding -> ok, handled after loop
            }
        } else {
            free(vals); free(buf); return 0;
        }
    }
    // Adjust for padding of last quartet
    int c = vals[vlen-2], d = vals[vlen-1];
    if (d==-2){ // one padding
        if (pos==0){ free(vals); free(buf); return 0; }
        pos--; // remove last
        if (c==-2){ // two padding
            if (pos==0){ free(vals); free(buf); return 0; }
            pos--; // remove one more
        }
    }
    *out = buf; *out_len = pos;
    free(vals);
    return 1;
}

/* PEM to DER */
static int pem_to_der(const char* pem, uint8_t** out, size_t* out_len){
    const char* header = "-----BEGIN CERTIFICATE-----";
    const char* footer = "-----END CERTIFICATE-----";
    const char* h = strstr(pem, header);
    const char* f = strstr(pem, footer);
    if (!h || !f || f<=h) return 0;
    h += strlen(header);
    size_t body_len = (size_t)(f - h);
    char* body = (char*)malloc(body_len + 1);
    if (!body) return 0;
    memcpy(body, h, body_len);
    body[body_len] = 0;
    int ok = base64_decode_strict(body, out, out_len);
    free(body);
    return ok;
}

/* Core function */
int certificate_matches_hash(const char* cert_input, const char* expected_sha256_hex){
    if (!cert_input || !expected_sha256_hex) return 0;

    char sanitized[256+1];
    sanitize_hex(expected_sha256_hex, sanitized, sizeof(sanitized));
    uint8_t expected[32];
    int elen = hex_to_bytes(sanitized, expected, sizeof(expected));
    if (elen != 32) return 0;

    uint8_t* data = NULL;
    size_t data_len = 0;
    if (strstr(cert_input, "-----BEGIN CERTIFICATE-----")){
        if (!pem_to_der(cert_input, &data, &data_len)) return 0;
    } else {
        data_len = strlen(cert_input);
        data = (uint8_t*)malloc(data_len?data_len:1);
        if (!data) return 0;
        memcpy(data, cert_input, data_len);
    }

    uint8_t digest[32];
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, digest);

    int match = constant_time_eq(digest, expected, 32);
    free(data);
    return match;
}

int main(void){
    const char* hello = "hello";
    const char* sha_hello = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
    char wrong[65]; memset(wrong, '0', 64); wrong[64]=0;

    const char* pem_hello =
        "-----BEGIN CERTIFICATE-----\n"
        "aGVsbG8=\n"
        "-----END CERTIFICATE-----\n";

    const char* pem_hello_messy =
        "-----BEGIN CERTIFICATE-----\n"
        "  aG Vs\n"
        "  bG8=\n"
        "-----END CERTIFICATE-----";

    const char* invalid_pem =
        "-----BEGIN CERTIFICATE-----\n"
        "???not_base64@@@\n"
        "-----END CERTIFICATE-----";

    printf("Test1 (DER correct): %s\n", certificate_matches_hash(hello, sha_hello) ? "true" : "false");
    printf("Test2 (DER wrong):   %s\n", certificate_matches_hash(hello, wrong) ? "true" : "false");
    printf("Test3 (PEM correct): %s\n", certificate_matches_hash(pem_hello, sha_hello) ? "true" : "false");
    printf("Test4 (PEM messy):   %s\n", certificate_matches_hash(pem_hello_messy, sha_hello) ? "true" : "false");
    printf("Test5 (PEM invalid): %s\n", certificate_matches_hash(invalid_pem, sha_hello) ? "true" : "false");
    return 0;
}