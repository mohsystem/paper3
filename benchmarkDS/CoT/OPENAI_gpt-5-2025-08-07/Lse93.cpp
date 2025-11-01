#include <bits/stdc++.h>
using namespace std;

/*
Chain-of-Through in comments:
1) Problem understanding: Provide a function to accept username & password, hash the password, and insert into a database (here, an in-memory store).
2) Security requirements: Validate inputs; use PBKDF2-HMAC-SHA256 with a random salt and high iteration count; avoid storing plaintext; ensure uniqueness.
3) Secure coding generation: Implement SHA-256, HMAC, PBKDF2; use std::random_device for salt; sanitize sensitive buffers.
4) Code review: No plaintext persistence; proper bounds; safe encodings; uniqueness enforced.
5) Secure code output: Final implementation below.
*/

// Minimal SHA-256 implementation
namespace sha256 {
    typedef uint32_t u32; typedef uint64_t u64; typedef uint8_t u8;

    struct Ctx {
        u32 state[8];
        u64 bitlen;
        u8 data[64];
        size_t datalen;
    };

    static inline u32 rotr(u32 x, u32 n){ return (x>>n)|(x<<(32-n)); }
    static inline u32 ch(u32 x,u32 y,u32 z){ return (x&y) ^ (~x&z); }
    static inline u32 maj(u32 x,u32 y,u32 z){ return (x&y) ^ (x&z) ^ (y&z); }
    static inline u32 ep0(u32 x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
    static inline u32 ep1(u32 x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
    static inline u32 sig0(u32 x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
    static inline u32 sig1(u32 x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

    static const u32 K[64]={
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    static void transform(Ctx &ctx, const u8 data[64]){
        u32 m[64];
        for(int i=0;i<16;i++){
            m[i] = (u32)data[i*4]<<24 | (u32)data[i*4+1]<<16 | (u32)data[i*4+2]<<8 | (u32)data[i*4+3];
        }
        for(int i=16;i<64;i++){
            m[i]=sig1(m[i-2])+m[i-7]+sig0(m[i-15])+m[i-16];
        }
        u32 a=ctx.state[0],b=ctx.state[1],c=ctx.state[2],d=ctx.state[3],e=ctx.state[4],f=ctx.state[5],g=ctx.state[6],h=ctx.state[7];
        for(int i=0;i<64;i++){
            u32 t1 = h + ep1(e) + ch(e,f,g) + K[i] + m[i];
            u32 t2 = ep0(a) + maj(a,b,c);
            h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        ctx.state[0]+=a; ctx.state[1]+=b; ctx.state[2]+=c; ctx.state[3]+=d;
        ctx.state[4]+=e; ctx.state[5]+=f; ctx.state[6]+=g; ctx.state[7]+=h;
    }

    static void init(Ctx &ctx){
        ctx.datalen=0; ctx.bitlen=0;
        ctx.state[0]=0x6a09e667; ctx.state[1]=0xbb67ae85; ctx.state[2]=0x3c6ef372; ctx.state[3]=0xa54ff53a;
        ctx.state[4]=0x510e527f; ctx.state[5]=0x9b05688c; ctx.state[6]=0x1f83d9ab; ctx.state[7]=0x5be0cd19;
    }

    static void update(Ctx &ctx, const u8* data, size_t len){
        for(size_t i=0;i<len;i++){
            ctx.data[ctx.datalen++] = data[i];
            if(ctx.datalen==64){
                transform(ctx, ctx.data);
                ctx.bitlen += 512;
                ctx.datalen=0;
            }
        }
    }

    static void final(Ctx &ctx, u8 hash[32]){
        u64 bitlen_total = ctx.bitlen + ctx.datalen*8;
        size_t i = ctx.datalen;
        ctx.data[i++] = 0x80;
        if(i>56){
            while(i<64) ctx.data[i++]=0x00;
            transform(ctx, ctx.data);
            i=0;
        }
        while(i<56) ctx.data[i++]=0x00;
        for(int j=7;j>=0;j--){
            ctx.data[i++] = (u8)((bitlen_total >> (j*8)) & 0xff);
        }
        transform(ctx, ctx.data);
        for(i=0;i<8;i++){
            hash[i*4+0]=(u8)((ctx.state[i]>>24)&0xff);
            hash[i*4+1]=(u8)((ctx.state[i]>>16)&0xff);
            hash[i*4+2]=(u8)((ctx.state[i]>>8)&0xff);
            hash[i*4+3]=(u8)(ctx.state[i]&0xff);
        }
    }

    static vector<u8> digest(const vector<u8>& data){
        Ctx c; init(c); update(c, data.data(), data.size()); vector<u8> out(32); final(c, out.data()); return out;
    }
}

static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    const size_t block = 64;
    vector<uint8_t> k(block, 0);
    if(keylen > block){
        vector<uint8_t> kh = sha256::digest(vector<uint8_t>(key, key+keylen));
        memcpy(k.data(), kh.data(), kh.size());
    } else {
        memcpy(k.data(), key, keylen);
    }
    vector<uint8_t> ipad(block), opad(block);
    for(size_t i=0;i<block;i++){ ipad[i]=k[i]^0x36; opad[i]=k[i]^0x5c; }
    sha256::Ctx ctx;
    sha256::init(ctx);
    sha256::update(ctx, ipad.data(), ipad.size());
    sha256::update(ctx, data, datalen);
    uint8_t inner[32];
    sha256::final(ctx, inner);

    sha256::init(ctx);
    sha256::update(ctx, opad.data(), opad.size());
    sha256::update(ctx, inner, 32);
    sha256::final(ctx, out);
    memset(inner, 0, sizeof(inner));
    fill(k.begin(), k.end(), 0);
    fill(ipad.begin(), ipad.end(), 0);
    fill(opad.begin(), opad.end(), 0);
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen, const uint8_t* salt, size_t saltlen, uint32_t iterations, uint8_t* out, size_t dkLen){
    uint32_t blocks = (uint32_t)((dkLen + 31) / 32);
    vector<uint8_t> saltBlock(salt, salt+saltlen);
    saltBlock.resize(saltBlock.size()+4);
    size_t offset=0;
    for(uint32_t i=1;i<=blocks;i++){
        saltBlock[saltlen+0] = (uint8_t)((i>>24)&0xff);
        saltBlock[saltlen+1] = (uint8_t)((i>>16)&0xff);
        saltBlock[saltlen+2] = (uint8_t)((i>>8)&0xff);
        saltBlock[saltlen+3] = (uint8_t)(i&0xff);
        uint8_t u[32];
        uint8_t t[32];
        hmac_sha256(password, passlen, saltBlock.data(), saltBlock.size(), u);
        memcpy(t, u, 32);
        for(uint32_t j=1;j<iterations;j++){
            hmac_sha256(password, passlen, u, 32, u);
            for(int k=0;k<32;k++) t[k]^=u[k];
        }
        size_t clen = min((size_t)32, dkLen - offset);
        memcpy(out+offset, t, clen);
        offset += clen;
        memset(u,0,sizeof(u));
        memset(t,0,sizeof(t));
    }
    fill(saltBlock.begin(), saltBlock.end(), 0);
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
    bool U=false,L=false,D=false,S=false;
    for(char c: p){
        if(isupper((unsigned char)c)) U=true;
        else if(islower((unsigned char)c)) L=true;
        else if(isdigit((unsigned char)c)) D=true;
        else S=true;
    }
    return U&&L&&D&&S;
}

struct User {
    int id;
    string username;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
};
static vector<User> DB;
static int NEXT_ID=1;

static string hexenc(const vector<uint8_t>& v){
    static const char* H="0123456789abcdef";
    string s; s.reserve(v.size()*2);
    for(uint8_t b: v){ s.push_back(H[b>>4]); s.push_back(H[b&0xF]); }
    return s;
}

bool register_user(const string& username, const string& password){
    if(!valid_username(username) || !valid_password(password)) return false;
    // uniqueness
    for(const auto& u: DB) if(u.username == username) return false;

    // secure salt
    vector<uint8_t> salt(16);
    std::random_device rd;
    for(size_t i=0;i<salt.size();i++) salt[i] = (uint8_t)(rd() & 0xFF);

    // derive key
    vector<uint8_t> pwd(password.begin(), password.end());
    vector<uint8_t> dk(32);
    pbkdf2_hmac_sha256(pwd.data(), pwd.size(), salt.data(), salt.size(), 200000, dk.data(), dk.size());
    // wipe password
    fill(pwd.begin(), pwd.end(), 0);

    User rec;
    rec.id = NEXT_ID++;
    rec.username = username;
    rec.salt = move(salt);
    rec.hash = move(dk);
    DB.push_back(move(rec));
    return true;
}

int main(){
    // 5 test cases
    cout << "T1 alice: " << (register_user("alice", "StrongP@ssw0rd!") ? "true":"false") << "\n";
    cout << "T2 bob (weak): " << (register_user("bob", "weak") ? "true":"false") << "\n";
    cout << "T3 alice duplicate: " << (register_user("alice", "AnotherStr0ng!") ? "true":"false") << "\n";
    cout << "T4 charlie: " << (register_user("charlie", "Cmplx#Pass123") ? "true":"false") << "\n";
    cout << "T5 bad username: " << (register_user("bad user", "G00d#Pass!") ? "true":"false") << "\n";

    // Print stored users (no plaintext)
    for(const auto& u: DB){
        cout << "id="<<u.id<<", user="<<u.username<<", salt="<<hexenc(u.salt)<<", hash="<<hexenc(u.hash)<<"\n";
    }
    return 0;
}