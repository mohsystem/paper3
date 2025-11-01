#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>

// Step 1-5 summarized:
// - Provide processLogin that checks credentials against an in-memory "database" using PBKDF2-HMAC-SHA256
// - Use per-user salts, iterations, and constant-time comparison
// - No external dependencies

// Minimal SHA-256 implementation
struct SHA256_CTX {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
};

static const uint32_t k256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t ep1(uint32_t x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sig0(uint32_t x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]){
    uint32_t m[64];
    for(int i=0;i<16;++i){
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for(int i=16;i<64;++i){
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for(int i=0;i<64;++i){
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + k256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len){
    for(size_t i=0;i<len;++i){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen==64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]){
    uint32_t i = ctx->datalen;
    // Pad
    ctx->data[i++] = 0x80;
    if(i > 56){
        while(i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while(i < 56) ctx->data[i++] = 0x00;
    ctx->bitlen += (uint64_t)ctx->datalen * 8;
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8 );
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for(i=0;i<4;++i){
        for(int j=0;j<8;++j){
            hash[j*4 + i] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xffu);
        }
    }
}

// HMAC-SHA256
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    uint8_t k_ipad[64], k_opad[64], tk[32];
    if(keylen > 64){
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        key = tk;
        keylen = 32;
    }
    std::memset(k_ipad, 0x36, 64);
    std::memset(k_opad, 0x5c, 64);
    for(size_t i=0;i<keylen;++i){
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }
    uint8_t inner[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner, 32);
    sha256_final(&ctx, out);
}

// PBKDF2-HMAC-SHA256
static void pbkdf2_hmac_sha256(const uint8_t* password, size_t plen, const uint8_t* salt, size_t slen, uint32_t iters, uint8_t* out, size_t dkLen){
    uint32_t blockCount = (uint32_t)((dkLen + 31) / 32);
    uint8_t u[32], t[32];
    std::vector<uint8_t> asalt(slen + 4);
    std::memcpy(asalt.data(), salt, slen);
    for(uint32_t i=1;i<=blockCount;++i){
        asalt[slen+0] = (uint8_t)(i >> 24);
        asalt[slen+1] = (uint8_t)(i >> 16);
        asalt[slen+2] = (uint8_t)(i >> 8);
        asalt[slen+3] = (uint8_t)(i);
        hmac_sha256(password, plen, asalt.data(), asalt.size(), u);
        std::memcpy(t, u, 32);
        for(uint32_t j=1;j<iters;++j){
            hmac_sha256(password, plen, u, 32, u);
            for(int k=0;k<32;++k) t[k] ^= u[k];
        }
        size_t offset = (size_t)(i - 1) * 32;
        size_t toCopy = std::min((size_t)32, dkLen - offset);
        std::memcpy(out + offset, t, toCopy);
    }
}

static bool constantTimeEq(const uint8_t* a, const uint8_t* b, size_t len){
    uint8_t res = 0;
    for(size_t i=0;i<len;++i) res |= (uint8_t)(a[i] ^ b[i]);
    return res == 0;
}

struct UserRecord {
    std::vector<uint8_t> salt;
    uint32_t iterations;
    std::vector<uint8_t> hash; // 32 bytes
};

static std::unordered_map<std::string, UserRecord> USER_DB;

static void initUserDb(){
    const std::string alicePass = "alicePass123!";
    const std::string bobPass = "bobSecure#456";
    const std::vector<uint8_t> aliceSalt = {'A','L','I','C','E','_','S','A','L','T'};
    const std::vector<uint8_t> bobSalt = {'B','O','B','_','S','A','L','T'};
    const uint32_t iterations = 100000;
    uint8_t dk[32];

    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(alicePass.data()), alicePass.size(),
                       aliceSalt.data(), aliceSalt.size(), iterations, dk, 32);
    USER_DB["alice"] = UserRecord{aliceSalt, iterations, std::vector<uint8_t>(dk, dk+32)};

    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(bobPass.data()), bobPass.size(),
                       bobSalt.data(), bobSalt.size(), iterations, dk, 32);
    USER_DB["bob"] = UserRecord{bobSalt, iterations, std::vector<uint8_t>(dk, dk+32)};
}

std::string processLogin(const std::string& username, const std::string& password){
    std::string uname = username;
    // trim
    while(!uname.empty() && (uname.back()==' ' || uname.back()=='\t' || uname.back()=='\n' || uname.back()=='\r')) uname.pop_back();
    size_t start = 0; while(start<uname.size() && (uname[start]==' ' || uname[start]=='\t' || uname[start]=='\n' || uname[start]=='\r')) ++start;
    if(start>0) uname = uname.substr(start);

    if(uname.empty()) return "/login";
    auto it = USER_DB.find(uname);
    if(it == USER_DB.end()) return "/login";
    const UserRecord& rec = it->second;
    uint8_t candidate[32];
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       rec.salt.data(), rec.salt.size(), rec.iterations, candidate, 32);
    if(constantTimeEq(candidate, rec.hash.data(), 32)){
        return "/dashboard";
    }
    return "/login";
}

int main(){
    initUserDb();
    std::vector<std::pair<std::string,std::string>> tests = {
        {"alice", "alicePass123!"},
        {"alice", "wrongPassword"},
        {"unknown", "somepass"},
        {"bob", "bobSecure#456"},
        {"", "any"}
    };
    for(const auto& t : tests){
        std::cout << "Login(" << t.first << ", ****) -> " << processLogin(t.first, t.second) << "\n";
    }
    return 0;
}