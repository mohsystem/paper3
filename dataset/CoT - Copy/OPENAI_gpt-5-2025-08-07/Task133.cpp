#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through Steps integrated: secure design, implementation, review.

// Minimal SHA-256 implementation (public domain style)
namespace sha256_ns {
    typedef uint32_t u32;
    typedef uint64_t u64;
    static inline u32 R(u32 x, int n){ return (x>>n) | (x<<(32-n)); }
    static inline u32 Ch(u32 x,u32 y,u32 z){ return (x & y) ^ (~x & z); }
    static inline u32 Maj(u32 x,u32 y,u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
    static inline u32 S0(u32 x){ return R(x,2) ^ R(x,13) ^ R(x,22); }
    static inline u32 S1(u32 x){ return R(x,6) ^ R(x,11) ^ R(x,25); }
    static inline u32 s0(u32 x){ return R(x,7) ^ R(x,18) ^ (x>>3); }
    static inline u32 s1(u32 x){ return R(x,17) ^ R(x,19) ^ (x>>10); }

    static const u32 K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    vector<uint8_t> sha256(const vector<uint8_t>& data){
        u32 H[8] = {
            0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
            0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
        };
        vector<uint8_t> msg = data;
        size_t bitlen = (u64)msg.size() * 8;
        msg.push_back(0x80);
        while ((msg.size() % 64) != 56) msg.push_back(0x00);
        for (int i=7;i>=0;i--) msg.push_back((uint8_t)((bitlen >> (i*8)) & 0xFF));
        for (size_t off=0; off<msg.size(); off+=64){
            u32 w[64];
            for (int i=0;i<16;i++){
                w[i] = ((u32)msg[off + i*4] << 24) | ((u32)msg[off + i*4 + 1] << 16) | ((u32)msg[off + i*4 + 2] << 8) | (u32)msg[off + i*4 + 3];
            }
            for (int i=16;i<64;i++) w[i] = s1(w[i-2]) + w[i-7] + s0(w[i-15]) + w[i-16];
            u32 a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
            for (int i=0;i<64;i++){
                u32 T1 = h + S1(e) + Ch(e,f,g) + K[i] + w[i];
                u32 T2 = S0(a) + Maj(a,b,c);
                h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a = T1 + T2;
            }
            H[0]+=a; H[1]+=b; H[2]+=c; H[3]+=d; H[4]+=e; H[5]+=f; H[6]+=g; H[7]+=h;
        }
        vector<uint8_t> out(32);
        for (int i=0;i<8;i++){
            out[i*4]   = (H[i] >> 24) & 0xFF;
            out[i*4+1] = (H[i] >> 16) & 0xFF;
            out[i*4+2] = (H[i] >> 8) & 0xFF;
            out[i*4+3] = H[i] & 0xFF;
        }
        return out;
    }
}

static vector<uint8_t> hmac_sha256(const vector<uint8_t>& key, const vector<uint8_t>& data){
    const size_t block = 64;
    vector<uint8_t> k = key;
    if (k.size() > block) k = sha256_ns::sha256(k);
    if (k.size() < block) k.resize(block, 0x00);
    vector<uint8_t> o(block), i(block);
    for (size_t j=0;j<block;j++){ o[j] = k[j] ^ 0x5c; i[j] = k[j] ^ 0x36; }
    vector<uint8_t> inner(i);
    inner.insert(inner.end(), data.begin(), data.end());
    vector<uint8_t> innerHash = sha256_ns::sha256(inner);
    vector<uint8_t> outer(o);
    outer.insert(outer.end(), innerHash.begin(), innerHash.end());
    return sha256_ns::sha256(outer);
}

static vector<uint8_t> pbkdf2_hmac_sha256(const string& password, const vector<uint8_t>& salt, int iterations, size_t dkLen){
    vector<uint8_t> dk(dkLen);
    size_t hLen = 32;
    size_t l = (dkLen + hLen - 1) / hLen;
    size_t r = dkLen - (l - 1) * hLen;
    vector<uint8_t> P(password.begin(), password.end());
    for (size_t i=1;i<=l;i++){
        // U1 = PRF(P, S || INT(i))
        vector<uint8_t> Si(salt);
        uint8_t be[4] = { (uint8_t)((i>>24)&0xFF), (uint8_t)((i>>16)&0xFF), (uint8_t)((i>>8)&0xFF), (uint8_t)(i&0xFF) };
        Si.insert(Si.end(), be, be+4);
        vector<uint8_t> Ui = hmac_sha256(P, Si);
        vector<uint8_t> T = Ui;
        for (int j=2;j<=iterations;j++){
            Ui = hmac_sha256(P, Ui);
            for (size_t k=0;k<T.size();k++) T[k] ^= Ui[k];
        }
        size_t offset = (i-1)*hLen;
        size_t clen = (i==l)? r : hLen;
        memcpy(&dk[offset], &T[0], clen);
    }
    return dk;
}

static bool constTimeEq(const vector<uint8_t>& a, const vector<uint8_t>& b){
    size_t len = max(a.size(), b.size());
    uint8_t r = 0;
    for (size_t i=0;i<len;i++){
        uint8_t x = i<a.size()?a[i]:0;
        uint8_t y = i<b.size()?b[i]:0;
        r |= (x ^ y);
    }
    return r == 0 && a.size() == b.size();
}

static string base64url(const vector<uint8_t>& data){
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    string out;
    int val=0; int valb=-6;
    for (uint8_t c : data){
        val = (val<<8) + c;
        valb += 8;
        while (valb >= 0){
            out.push_back(tbl[(val>>valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    // no padding
    return out;
}

static vector<uint8_t> sha256_bytes(const string& s){
    vector<uint8_t> d(s.begin(), s.end());
    return sha256_ns::sha256(d);
}

static vector<uint8_t> random_bytes(size_t n){
    vector<uint8_t> out(n);
    std::random_device rd;
    for (size_t i=0;i<n;i++){
        out[i] = static_cast<uint8_t>(rd());
    }
    return out;
}

struct User {
    string email;
    vector<uint8_t> salt;
    vector<uint8_t> passhash;
    int iterations;
    vector<uint8_t> tokenHash;
    long long tokenExpiryMs;
};

static unordered_map<string, User> USERS;
static unordered_map<string, string> TOKEN_TO_EMAIL; // hex token hash -> email

static string toHex(const vector<uint8_t>& v){
    static const char* hex="0123456789abcdef";
    string s; s.reserve(v.size()*2);
    for (auto b: v){ s.push_back(hex[(b>>4)&0xF]); s.push_back(hex[b&0xF]); }
    return s;
}

static bool isPasswordStrong(const string& p){
    if (p.size() < 10 || p.size() > 100) return false;
    bool up=false,lo=false,di=false,sp=false;
    for (char c: p){
        if (isupper((unsigned char)c)) up=true;
        else if (islower((unsigned char)c)) lo=true;
        else if (isdigit((unsigned char)c)) di=true;
        else sp=true;
    }
    if (!(up && lo && di && sp)) return false;
    string lower=p; for (auto& c: lower) c=tolower((unsigned char)c);
    vector<string> bad={"password","12345","qwerty","letmein","admin"};
    for (auto& b: bad) if (lower.find(b)!=string::npos) return false;
    return true;
}

static bool createUser(const string& email, const string& password){
    if (email.empty() || password.empty()) return false;
    if (USERS.find(email) != USERS.end()) return false;
    if (!isPasswordStrong(password)) return false;
    vector<uint8_t> salt = random_bytes(16);
    vector<uint8_t> hash = pbkdf2_hmac_sha256(password, salt, 100000, 32);
    USERS[email] = User{email, salt, hash, 100000, {}, 0};
    return true;
}

static string requestReset(const string& email){
    auto it = USERS.find(email);
    if (it == USERS.end()) return string();
    // generate token
    vector<uint8_t> tok = random_bytes(32);
    string token = base64url(tok);
    vector<uint8_t> th = sha256_bytes(token);
    string hex = toHex(th);
    // map
    it->second.tokenHash = th;
    it->second.tokenExpiryMs = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()).count() + 10*60*1000;
    TOKEN_TO_EMAIL[hex] = email;
    return token;
}

static bool resetPassword(const string& token, const string& newPassword){
    if (token.empty() || newPassword.empty()) return false;
    vector<uint8_t> th = sha256_bytes(token);
    string hex = toHex(th);
    auto it = TOKEN_TO_EMAIL.find(hex);
    if (it == TOKEN_TO_EMAIL.end()) return false;
    string email = it->second;
    auto uit = USERS.find(email);
    if (uit == USERS.end()) return false;
    long long nowms = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
    if (uit->second.tokenExpiryMs < nowms){
        // expire
        TOKEN_TO_EMAIL.erase(hex);
        uit->second.tokenHash.clear();
        uit->second.tokenExpiryMs = 0;
        return false;
    }
    if (!isPasswordStrong(newPassword)) return false;
    vector<uint8_t> salt = random_bytes(16);
    vector<uint8_t> hash = pbkdf2_hmac_sha256(newPassword, salt, 100000, 32);
    uit->second.salt = move(salt);
    uit->second.passhash = move(hash);
    uit->second.iterations = 100000;
    // invalidate token
    TOKEN_TO_EMAIL.erase(hex);
    uit->second.tokenHash.clear();
    uit->second.tokenExpiryMs = 0;
    return true;
}

int main(){
    cout << "C++ tests:" << endl;
    // 1) Unknown user
    string t1 = requestReset("unknown@example.com");
    cout << "Test1 token for unknown user should be empty: " << (t1.empty() ? "empty" : t1) << endl;

    // 2) Create and wrong token
    bool created = createUser("alice@example.com", "OldPassw0rd!");
    cout << "Test2 created user: " << (created ? "true" : "false") << endl;
    string token = requestReset("alice@example.com");
    cout << "Test2 got token: " << (!token.empty() ? "true" : "false") << endl;
    bool wrong = resetPassword("badtoken", "NewPassw0rd!");
    cout << "Test2 reset with wrong token: " << (wrong ? "true" : "false") << endl;

    // 3) Weak password
    bool weak = resetPassword(token, "weak");
    cout << "Test3 reset with weak password: " << (weak ? "true" : "false") << endl;

    // 4) Successful reset
    bool ok = resetPassword(token, "N3w_Str0ngPass!");
    cout << "Test4 reset with correct token and strong password: " << (ok ? "true" : "false") << endl;

    // 5) Reuse token; then new token and reset
    bool reuse = resetPassword(token, "AnotherStr0ng!");
    cout << "Test5 reuse token: " << (reuse ? "true" : "false") << endl;
    string token2 = requestReset("alice@example.com");
    bool ok2 = resetPassword(token2, "UltraStr0ng#2025");
    cout << "Test5 new token reset: " << (ok2 ? "true" : "false") << endl;
    return 0;
}