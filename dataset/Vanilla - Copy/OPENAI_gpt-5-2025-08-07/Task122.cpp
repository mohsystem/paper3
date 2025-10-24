#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation (public domain style)
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

struct SHA256 {
    u32 state[8];
    u64 bitlen;
    u8 data[64];
    size_t datalen;

    static inline u32 rotr(u32 x, u32 n){ return (x >> n) | (x << (32 - n)); }
    static inline u32 ch(u32 x, u32 y, u32 z){ return (x & y) ^ (~x & z); }
    static inline u32 maj(u32 x, u32 y, u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
    static inline u32 ep0(u32 x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
    static inline u32 ep1(u32 x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
    static inline u32 sig0(u32 x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
    static inline u32 sig1(u32 x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

    static constexpr u32 K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    void init(){
        state[0]=0x6a09e667; state[1]=0xbb67ae85; state[2]=0x3c6ef372; state[3]=0xa54ff53a;
        state[4]=0x510e527f; state[5]=0x9b05688c; state[6]=0x1f83d9ab; state[7]=0x5be0cd19;
        bitlen = 0;
        datalen = 0;
    }

    void transform(const u8 data[]){
        u32 m[64];
        for (int i=0;i<16;i++){
            m[i] = (u32)data[i*4]<<24 | (u32)data[i*4+1]<<16 | (u32)data[i*4+2]<<8 | (u32)data[i*4+3];
        }
        for (int i=16;i<64;i++){
            m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
        }
        u32 a=state[0], b=state[1], c=state[2], d=state[3], e=state[4], f=state[5], g=state[6], h=state[7];
        for (int i=0;i<64;i++){
            u32 t1 = h + ep1(e) + ch(e,f,g) + K[i] + m[i];
            u32 t2 = ep0(a) + maj(a,b,c);
            h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
        }
        state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
        state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
    }

    void update(const u8* in, size_t len){
        for (size_t i=0;i<len;i++){
            data[datalen++] = in[i];
            if (datalen == 64){
                transform(data);
                bitlen += 512;
                datalen = 0;
            }
        }
    }

    void final(u8 hash[32]){
        size_t i = datalen;
        // Pad
        data[i++] = 0x80;
        if (i > 56){
            while (i < 64) data[i++] = 0x00;
            transform(data);
            i = 0;
        }
        while (i < 56) data[i++] = 0x00;
        bitlen += datalen * 8ULL;
        // Append length
        for (int j=7;j>=0;j--) {
            data[i++] = (u8)((bitlen >> (j*8)) & 0xff);
        }
        transform(data);
        for (int j=0;j<8;j++){
            hash[j*4+0] = (u8)((state[j] >> 24) & 0xff);
            hash[j*4+1] = (u8)((state[j] >> 16) & 0xff);
            hash[j*4+2] = (u8)((state[j] >> 8) & 0xff);
            hash[j*4+3] = (u8)((state[j] >> 0) & 0xff);
        }
    }
};

static string toHex(const vector<u8>& data){
    static const char* hex = "0123456789abcdef";
    string out; out.reserve(data.size()*2);
    for (u8 b : data){
        out.push_back(hex[(b>>4)&0xF]);
        out.push_back(hex[b&0xF]);
    }
    return out;
}
static string toHex(const u8* data, size_t n){
    static const char* hex = "0123456789abcdef";
    string out; out.reserve(n*2);
    for (size_t i=0;i<n;i++){
        u8 b=data[i];
        out.push_back(hex[(b>>4)&0xF]);
        out.push_back(hex[b&0xF]);
    }
    return out;
}
static vector<u8> fromHex(const string& s){
    auto hexval=[](char c)->int{
        if ('0'<=c && c<='9') return c-'0';
        if ('a'<=c && c<='f') return c-'a'+10;
        if ('A'<=c && c<='F') return c-'A'+10;
        return -1;
    };
    vector<u8> out;
    if (s.size()%2) return out;
    out.reserve(s.size()/2);
    for (size_t i=0;i<s.size();i+=2){
        int hi=hexval(s[i]), lo=hexval(s[i+1]);
        if (hi<0||lo<0){ out.clear(); return out; }
        out.push_back((u8)((hi<<4)|lo));
    }
    return out;
}

static string sha256_hex_with_salt(const string& password, const vector<u8>& salt){
    SHA256 ctx; ctx.init();
    ctx.update(salt.data(), salt.size());
    ctx.update(reinterpret_cast<const u8*>(password.data()), password.size());
    u8 out[32]; ctx.final(out);
    return toHex(out, 32);
}

class UserStore {
    unordered_map<string, string> store;
public:
    bool signup(const string& username, const string& password){
        if (username.empty()) return false;
        if (store.find(username) != store.end()) return false;
        vector<u8> salt(16);
        std::random_device rd;
        for (auto& b : salt) b = (u8)(rd() & 0xFF);
        string saltHex = toHex(salt);
        string hashHex = sha256_hex_with_salt(password, salt);
        store[username] = saltHex + ":" + hashHex;
        return true;
    }

    bool verify(const string& username, const string& password){
        auto it = store.find(username);
        if (it == store.end()) return false;
        const string& stored = it->second;
        auto pos = stored.find(':');
        if (pos == string::npos) return false;
        string saltHex = stored.substr(0, pos);
        string expected = stored.substr(pos+1);
        vector<u8> salt = fromHex(saltHex);
        if (salt.empty()) return false;
        string actual = sha256_hex_with_salt(password, salt);
        if (actual.size() != expected.size()) return false;
        unsigned diff = 0;
        for (size_t i=0;i<actual.size();i++) diff |= (unsigned)(unsigned char)actual[i] ^ (unsigned)(unsigned char)expected[i];
        return diff == 0;
    }

    string getStored(const string& username){
        auto it = store.find(username);
        if (it == store.end()) return "";
        return it->second;
    }
};

int main(){
    UserStore us;
    cout << "Test1 signup alice: " << (us.signup("alice", "Password123!") ? "true" : "false") << "\n";
    cout << "Test2 signup bob: " << (us.signup("bob", "Secr3t!") ? "true" : "false") << "\n";
    cout << "Test3 duplicate alice: " << (us.signup("alice", "another") ? "true" : "false") << "\n";
    cout << "Test4 verify bob correct: " << (us.verify("bob", "Secr3t!") ? "true" : "false") << "\n";
    cout << "Test5 verify bob wrong: " << (us.verify("bob", "wrong") ? "true" : "false") << "\n";
    cout << "Stored alice: " << us.getStored("alice") << "\n";
    return 0;
}