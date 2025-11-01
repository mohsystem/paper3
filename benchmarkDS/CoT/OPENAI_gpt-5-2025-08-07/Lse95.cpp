// Chain-of-Through:
// 1) Understand: in-memory registration/login with salted PBKDF2-HMAC-SHA256.
// 2) Security: secure random salt, PBKDF2 iterations, constant-time compare, input validation.
// 3) Implement securely without external libs: include SHA-256, HMAC, PBKDF2 implementations.
// 4) Review: safe memory handling, no sensitive prints, bounds checks.
// 5) Output final secure code.

#include <bits/stdc++.h>
using namespace std;

// ---- Minimal SHA-256 implementation (public domain style) ----
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t ep1(uint32_t x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sig0(uint32_t x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static const uint32_t K256[64] = {
  0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
  0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
  0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
  0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
  0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
  0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
  0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
  0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
    uint32_t m[64];
    for(int i=0;i<16;i++){
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for(int i=16;i<64;i++){
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3];
    uint32_t e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for(int i=0;i<64;i++){
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + K256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len){
    for(size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    ctx->bitlen += ctx->datalen * 8ull;
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if(i > 56){
        while(i<64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while(i<56) ctx->data[i++] = 0x00;
    // Append length big-endian
    uint64_t bl = ctx->bitlen;
    for(int j=7;j>=0;j--) ctx->data[i++] = (uint8_t)((bl >> (j*8)) & 0xff);
    sha256_transform(ctx, ctx->data);
    for(int j=0;j<8;j++){
        hash[j*4  ] = (uint8_t)((ctx->state[j] >> 24) & 0xff);
        hash[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xff);
        hash[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xff);
        hash[j*4+3] = (uint8_t)(ctx->state[j] & 0xff);
    }
}

// ---- HMAC-SHA256 ----
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];
    if(keylen > 64){
        SHA256_CTX t;
        sha256_init(&t);
        sha256_update(&t, key, keylen);
        sha256_final(&t, tk);
        key = tk; keylen = 32;
    }
    memset(k_ipad, 0, 64);
    memset(k_opad, 0, 64);
    memcpy(k_ipad, key, keylen);
    memcpy(k_opad, key, keylen);
    for(int i=0;i<64;i++){ k_ipad[i] ^= 0x36; k_opad[i] ^= 0x5c; }
    // inner
    SHA256_CTX c;
    sha256_init(&c);
    sha256_update(&c, k_ipad, 64);
    sha256_update(&c, data, datalen);
    uint8_t inner[32];
    sha256_final(&c, inner);
    // outer
    sha256_init(&c);
    sha256_update(&c, k_opad, 64);
    sha256_update(&c, inner, 32);
    sha256_final(&c, out);
    // clear
    memset(tk, 0, sizeof(tk));
    memset(inner, 0, sizeof(inner));
}

// ---- PBKDF2-HMAC-SHA256 ----
static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen, const uint8_t* salt, size_t saltlen, uint32_t iterations, uint8_t* dk, size_t dkLen){
    uint32_t blocks = (uint32_t)((dkLen + 31) / 32);
    vector<uint8_t> asalt(salt, salt + saltlen);
    asalt.resize(saltlen + 4);
    uint8_t U[32], T[32];
    for(uint32_t i=1;i<=blocks;i++){
        asalt[saltlen+0] = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen+3] = (uint8_t)(i & 0xff);
        hmac_sha256(password, passlen, asalt.data(), asalt.size(), U);
        memcpy(T, U, 32);
        for(uint32_t j=1;j<iterations;j++){
            hmac_sha256(password, passlen, U, 32, U);
            for(int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i-1) * 32;
        size_t tocpy = min((size_t)32, dkLen - offset);
        memcpy(dk + offset, T, tocpy);
    }
    // clear sensitive buffers
    memset(U, 0, sizeof(U));
    memset(T, 0, sizeof(T));
    fill(asalt.begin(), asalt.end(), 0);
}

static bool const_time_eq(const vector<uint8_t>& a, const vector<uint8_t>& b){
    if(a.size() != b.size()) return false;
    uint8_t diff = 0;
    for(size_t i=0;i<a.size();i++) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

static string to_hex(const vector<uint8_t>& data){
    static const char* hex="0123456789abcdef";
    string s; s.resize(data.size()*2);
    for(size_t i=0;i<data.size();i++){
        s[2*i] = hex[(data[i]>>4)&0xF];
        s[2*i+1] = hex[data[i]&0xF];
    }
    return s;
}

static vector<uint8_t> from_hex(const string& s){
    auto val = [](char c)->int{
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return c-'a'+10;
        if(c>='A'&&c<='F') return c-'A'+10;
        return -1;
    };
    vector<uint8_t> out;
    if(s.size()%2) return out;
    out.resize(s.size()/2);
    for(size_t i=0;i<out.size();i++){
        int hi=val(s[2*i]), lo=val(s[2*i+1]);
        if(hi<0||lo<0){ out.clear(); return out; }
        out[i] = (uint8_t)((hi<<4)|lo);
    }
    return out;
}

static bool valid_username(const string& u){
    if(u.size()<3 || u.size()>64) return false;
    for(char c: u){
        if(!(isalnum((unsigned char)c) || c=='_' || c=='.' || c=='-')) return false;
    }
    return true;
}
static bool valid_password(const string& p){
    if(p.size()<8 || p.size()>1024) return false;
    for(unsigned char c: p){ if(c < 32) return false; }
    return true;
}

class Lse95 {
    unordered_map<string,string> db;
    const uint32_t iterations = 120000;
    const size_t saltLen = 16;
    const size_t keyLen = 32;
public:
    bool registerUser(const string& username, const string& password){
        if(!valid_username(username) || !valid_password(password)) return false;
        if(db.find(username) != db.end()) return false;
        vector<uint8_t> salt(saltLen);
        std::random_device rd;
        for(size_t i=0;i<saltLen;i++) salt[i] = static_cast<uint8_t>(rd());
        vector<uint8_t> dk(keyLen);
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           salt.data(), salt.size(), iterations, dk.data(), dk.size());
        string record = to_hex(dk) + ":" + to_hex(salt);
        db[username] = record;
        // clear dk
        fill(dk.begin(), dk.end(), 0);
        return true;
    }
    bool verifyLogin(const string& username, const string& password){
        auto it = db.find(username);
        if(it == db.end()) return false;
        const string& rec = it->second;
        size_t pos = rec.find(':');
        if(pos == string::npos) return false;
        string hhex = rec.substr(0,pos);
        string shex = rec.substr(pos+1);
        vector<uint8_t> stored = from_hex(hhex);
        vector<uint8_t> salt = from_hex(shex);
        if(stored.empty() || salt.empty()) return false;
        vector<uint8_t> dk(stored.size());
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           salt.data(), salt.size(), iterations, dk.data(), dk.size());
        bool ok = const_time_eq(stored, dk);
        fill(dk.begin(), dk.end(), 0);
        return ok;
    }
};

int main(){
    Lse95 store;
    cout << boolalpha;

    // Test case 1: Register new user
    cout << "TC1 register alice: " << store.registerUser("alice", "S3curePwd!") << "\n";

    // Test case 2: Duplicate registration
    cout << "TC2 register alice again: " << store.registerUser("alice", "AnotherPwd!") << "\n";

    // Test case 3: Correct login
    cout << "TC3 login alice correct: " << store.verifyLogin("alice", "S3curePwd!") << "\n";

    // Test case 4: Wrong password
    cout << "TC4 login alice wrong: " << store.verifyLogin("alice", "WrongPass!") << "\n";

    // Test case 5: Another user register and login
    cout << "TC5 register bob: " << store.registerUser("bob-01", "P@ssw0rd123") << "\n";
    cout << "TC5 login bob correct: " << store.verifyLogin("bob-01", "P@ssw0rd123") << "\n";

    return 0;
}