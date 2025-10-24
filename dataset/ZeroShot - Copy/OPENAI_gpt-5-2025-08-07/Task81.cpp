#include <bits/stdc++.h>
using namespace std;

// SHA-256 implementation (public domain style, compact)
namespace sha256_impl {
    typedef uint32_t u32;
    typedef uint64_t u64;

    static inline u32 rotr(u32 x, u32 n){ return (x>>n)|(x<<(32-n)); }

    struct SHA256 {
        u32 h[8];
        u8vector<uint8_t> buf;
        u64 bits;

        SHA256(){ init(); }

        void init(){
            h[0]=0x6a09e667; h[1]=0xbb67ae85; h[2]=0x3c6ef372; h[3]=0xa54ff53a;
            h[4]=0x510e527f; h[5]=0x9b05688c; h[6]=0x1f83d9ab; h[7]=0x5be0cd19;
            buf.clear(); buf.reserve(64);
            bits = 0;
        }

        static inline u32 Ch(u32 x,u32 y,u32 z){return (x&y)^(~x&z);}
        static inline u32 Maj(u32 x,u32 y,u32 z){return (x&y)^(x&z)^(y&z);}
        static inline u32 BSIG0(u32 x){return rotr(x,2)^rotr(x,13)^rotr(x,22);}
        static inline u32 BSIG1(u32 x){return rotr(x,6)^rotr(x,11)^rotr(x,25);}
        static inline u32 SSIG0(u32 x){return rotr(x,7)^rotr(x,18)^(x>>3);}
        static inline u32 SSIG1(u32 x){return rotr(x,17)^rotr(x,19)^(x>>10);}

        static const u32 K[64];

        void update(const uint8_t* data, size_t len){
            bits += (u64)len * 8;
            size_t i=0;
            if(!buf.empty()){
                size_t toFill = min(len, (size_t)64 - buf.size());
                buf.insert(buf.end(), data, data+toFill);
                i += toFill;
                if(buf.size()==64){
                    transform(buf.data());
                    buf.clear();
                }
            }
            for(; i+64<=len; i+=64){
                transform(data+i);
            }
            if(i<len){
                buf.insert(buf.end(), data+i, data+len);
            }
        }

        void transform(const uint8_t block[64]){
            u32 w[64];
            for(int i=0;i<16;i++){
                w[i] = ((u32)block[i*4+0]<<8*3) | ((u32)block[i*4+1]<<8*2) | ((u32)block[i*4+2]<<8*1) | ((u32)block[i*4+3]);
            }
            for(int i=16;i<64;i++){
                w[i] = SSIG1(w[i-2]) + w[i-7] + SSIG0(w[i-15]) + w[i-16];
            }
            u32 a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
            for(int i=0;i<64;i++){
                u32 t1 = hh + BSIG1(e) + Ch(e,f,g) + K[i] + w[i];
                u32 t2 = BSIG0(a) + Maj(a,b,c);
                hh=g; g=f; f=e; e=d + t1;
                d=c; c=b; b=a; a=t1 + t2;
            }
            h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
        }

        void finalize(uint8_t out[32]){
            uint8_t pad[64] = {0x80};
            size_t padLen = (buf.size() < 56) ? (56 - buf.size()) : (64 + 56 - buf.size());
            update(pad, padLen);
            uint8_t lenbe[8];
            for(int i=0;i<8;i++){
                lenbe[7-i] = (uint8_t)((bits >> (i*8)) & 0xFF);
            }
            update(lenbe, 8);
            for(int i=0;i<8;i++){
                out[i*4+0] = (uint8_t)((h[i]>>24)&0xFF);
                out[i*4+1] = (uint8_t)((h[i]>>16)&0xFF);
                out[i*4+2] = (uint8_t)((h[i]>>8)&0xFF);
                out[i*4+3] = (uint8_t)((h[i])&0xFF);
            }
        }

        static vector<uint8_t> hash(const vector<uint8_t>& data){
            SHA256 s; s.update(data.data(), data.size());
            vector<uint8_t> out(32);
            s.finalize(out.data());
            return out;
        }
    };

    const uint32_t SHA256::K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
}
using sha256_impl::SHA256;

// Helpers
static string sanitizeHex(const string& s){
    string out; out.reserve(s.size());
    for(char c: s){
        if ((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f')) out.push_back(c);
    }
    return out;
}
static bool hexToBytes(const string& hex, vector<uint8_t>& out){
    if (hex.size()%2!=0) return false;
    out.resize(hex.size()/2);
    auto hexval = [](char c)->int{
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return 10 + (c-'a');
        if(c>='A'&&c<='F') return 10 + (c-'A');
        return -1;
    };
    for(size_t i=0,j=0;i<hex.size();i+=2,++j){
        int hi=hexval(hex[i]), lo=hexval(hex[i+1]);
        if(hi<0||lo<0) return false;
        out[j] = (uint8_t)((hi<<4)|lo);
    }
    return true;
}

static bool constantTimeEq(const vector<uint8_t>& a, const vector<uint8_t>& b){
    if(a.size()!=b.size()) return false;
    uint8_t acc=0;
    for(size_t i=0;i<a.size();++i) acc |= (uint8_t)(a[i]^b[i]);
    return acc==0;
}

static bool base64DecodeStrict(const string& in, vector<uint8_t>& out){
    static const int8_t T[256] = []{
        int8_t t[256];
        for(int i=0;i<256;i++) t[i] = -1;
        for(char c='A';c<='Z';++c) t[(unsigned char)c] = c - 'A';
        for(char c='a';c<='z';++c) t[(unsigned char)c] = 26 + (c - 'a');
        for(char c='0';c<='9';++c) t[(unsigned char)c] = 52 + (c - '0');
        t[(unsigned char)'+']=62; t[(unsigned char)'/']=63; t[(unsigned char)'=']= -2;
        return t;
    }();
    vector<int> vals;
    vals.reserve(in.size());
    for(unsigned char c: in){
        if (c=='\r'||c=='\n'||c=='\t'||c==' ') continue; // skip whitespace
        int8_t v = T[c];
        if (v==-1) return false; // invalid char
        vals.push_back(v);
    }
    if (vals.size()==0){ out.clear(); return true; }
    if (vals.size()%4!=0) return false;
    size_t quad = vals.size()/4;
    out.clear();
    out.reserve(quad*3);
    size_t idx=0;
    for(size_t i=0;i<quad;i++){
        int a=vals[idx++], b=vals[idx++], c=vals[idx++], d=vals[idx++];
        if (a<0||b<0) return false;
        uint32_t triple = ((uint32_t)a<<18)|((uint32_t)b<<12);
        uint8_t x = (triple>>16)&0xFF;
        out.push_back(x);
        if (c>=0){
            triple |= ((uint32_t)c<<6);
            uint8_t y = (triple>>8)&0xFF;
            out.push_back(y);
            if (d>=0){
                triple |= (uint32_t)d;
                uint8_t z = triple & 0xFF;
                out.push_back(z);
            } else { // d is padding
                // must be '='
                // c must not be padding
            }
        } else { // c is padding -> invalid because we can't pad at third char unless exactly one byte present
            return false;
        }
        if (c==-2){ // '=' can't happen; handled above as c>=0 check
            return false;
        }
        if (d==-2){
            // valid two-byte padding scenario: we ensured d<0 above means padding
        }
    }
    // Validate padding rules precisely
    // We need to re-process last quartet for proper padding lengths
    // Simpler: decode using standard rules:
    // Instead, we re-decode last block and adjust
    // Re-implement for last block:
    {
        size_t n = vals.size();
        int a=vals[n-4], b=vals[n-3], c=vals[n-2], d=vals[n-1];
        // Remove extra bytes if padded
        if (d==-2){ // '='
            out.pop_back(); // remove last byte
            if (c==-2){
                out.pop_back(); // remove one more byte
            }
        }
    }
    return true;
}

static bool pemToDer(const string& pem, vector<uint8_t>& der){
    const string header="-----BEGIN CERTIFICATE-----";
    const string footer="-----END CERTIFICATE-----";
    size_t h = pem.find(header);
    size_t f = pem.find(footer);
    if (h==string::npos || f==string::npos || f<=h) return false;
    string body = pem.substr(h+header.size(), f-(h+header.size()));
    return base64DecodeStrict(body, der);
}

static vector<uint8_t> sha256Bytes(const vector<uint8_t>& data){
    return SHA256::hash(data);
}

static bool certificateMatchesHash(const string& certInput, const string& expectedSha256Hex){
    if (expectedSha256Hex.empty()) return false;
    string sHex = sanitizeHex(expectedSha256Hex);
    vector<uint8_t> expected;
    if (!hexToBytes(sHex, expected) || expected.size()!=32) return false;

    vector<uint8_t> data;
    if (certInput.find("-----BEGIN CERTIFICATE-----") != string::npos){
        if (!pemToDer(certInput, data)) return false;
    } else {
        data.assign(certInput.begin(), certInput.end());
    }
    vector<uint8_t> actual = sha256Bytes(data);
    return constantTimeEq(actual, expected);
}

int main(){
    string hello = "hello";
    string shaHello = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
    string wrong(64, '0');

    string pemHello = "-----BEGIN CERTIFICATE-----\n"
                      "aGVsbG8=\n"
                      "-----END CERTIFICATE-----\n";

    string pemHelloMessy = "-----BEGIN CERTIFICATE-----\n"
                           "  aG Vs\n"
                           "  bG8=\n"
                           "-----END CERTIFICATE-----";

    string invalidPem = "-----BEGIN CERTIFICATE-----\n"
                        "???not_base64@@@\n"
                        "-----END CERTIFICATE-----";

    cout << boolalpha;
    cout << "Test1 (DER correct): " << certificateMatchesHash(hello, shaHello) << "\n";
    cout << "Test2 (DER wrong):   " << certificateMatchesHash(hello, wrong) << "\n";
    cout << "Test3 (PEM correct): " << certificateMatchesHash(pemHello, shaHello) << "\n";
    cout << "Test4 (PEM messy):   " << certificateMatchesHash(pemHelloMessy, shaHello) << "\n";
    cout << "Test5 (PEM invalid): " << certificateMatchesHash(invalidPem, shaHello) << "\n";
    return 0;
}