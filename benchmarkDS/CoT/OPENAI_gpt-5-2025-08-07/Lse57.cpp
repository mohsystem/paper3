// Chain-of-Through (comments only):
// 1) Implement login + email change requiring login, old email, new email, and confirm password.
// 2) Security: salted KDF using HMAC-SHA256 with many iterations; constant-time compare; input validation.
// 3) Secure coding: simple in-memory store, avoid printing secrets.
// 4) Review: constant-time compare, email format validation, state checks.
// 5) Output: final with 5 tests.

#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation (public domain style)
// Source adapted to be concise for demo purposes.
typedef uint8_t u8; typedef uint32_t u32; typedef uint64_t u64;

struct SHA256Ctx {
    u32 state[8];
    u64 bitlen;
    u8 data[64];
    size_t datalen;
};

static inline u32 rotr(u32 x, u32 n){ return (x>>n)|(x<<(32-n)); }
static inline u32 ch(u32 x,u32 y,u32 z){ return (x&y)^(~x&z); }
static inline u32 maj(u32 x,u32 y,u32 z){ return (x&y)^(x&z)^(y&z); }
static inline u32 e0(u32 x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
static inline u32 e1(u32 x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
static inline u32 s0(u32 x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
static inline u32 s1(u32 x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

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

static void sha256_init(SHA256Ctx &ctx){
    ctx.datalen=0; ctx.bitlen=0;
    ctx.state[0]=0x6a09e667; ctx.state[1]=0xbb67ae85; ctx.state[2]=0x3c6ef372; ctx.state[3]=0xa54ff53a;
    ctx.state[4]=0x510e527f; ctx.state[5]=0x9b05688c; ctx.state[6]=0x1f83d9ab; ctx.state[7]=0x5be0cd19;
}

static void sha256_transform(SHA256Ctx &ctx, const u8 data[]){
    u32 m[64]; for(int i=0;i<16;i++){ m[i]= (u32)data[i*4]<<24 | (u32)data[i*4+1]<<16 | (u32)data[i*4+2]<<8 | (u32)data[i*4+3]; }
    for(int i=16;i<64;i++){ m[i]= s1(m[i-2]) + m[i-7] + s0(m[i-15]) + m[i-16]; }
    u32 a=ctx.state[0],b=ctx.state[1],c=ctx.state[2],d=ctx.state[3],e=ctx.state[4],f=ctx.state[5],g=ctx.state[6],h=ctx.state[7];
    for(int i=0;i<64;i++){
        u32 t1 = h + e1(e) + ch(e,f,g) + K256[i] + m[i];
        u32 t2 = e0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx.state[0]+=a; ctx.state[1]+=b; ctx.state[2]+=c; ctx.state[3]+=d; ctx.state[4]+=e; ctx.state[5]+=f; ctx.state[6]+=g; ctx.state[7]+=h;
}

static void sha256_update(SHA256Ctx &ctx, const u8 data[], size_t len){
    for(size_t i=0;i<len;i++){
        ctx.data[ctx.datalen++] = data[i];
        if(ctx.datalen == 64){
            sha256_transform(ctx, ctx.data);
            ctx.bitlen += 512;
            ctx.datalen=0;
        }
    }
}

static void sha256_final(SHA256Ctx &ctx, u8 hash[32]){
    u32 i = ctx.datalen;
    if(ctx.datalen < 56){
        ctx.data[i++] = 0x80;
        while(i<56) ctx.data[i++] = 0x00;
    } else {
        ctx.data[i++] = 0x80;
        while(i<64) ctx.data[i++] = 0x00;
        sha256_transform(ctx, ctx.data);
        memset(ctx.data, 0, 56);
    }
    ctx.bitlen += (u64)ctx.datalen * 8;
    ctx.data[63] = ctx.bitlen; ctx.data[62] = ctx.bitlen>>8; ctx.data[61] = ctx.bitlen>>16; ctx.data[60] = ctx.bitlen>>24;
    ctx.data[59] = ctx.bitlen>>32; ctx.data[58] = ctx.bitlen>>40; ctx.data[57] = ctx.bitlen>>48; ctx.data[56] = ctx.bitlen>>56;
    sha256_transform(ctx, ctx.data);
    for(int j=0;j<8;j++){
        hash[j*4]   = (ctx.state[j]>>24) & 0xff;
        hash[j*4+1] = (ctx.state[j]>>16) & 0xff;
        hash[j*4+2] = (ctx.state[j]>>8) & 0xff;
        hash[j*4+3] = (ctx.state[j]) & 0xff;
    }
}

// HMAC-SHA256
static void hmac_sha256(const vector<u8> &key, const u8 *msg, size_t msglen, u8 out[32]){
    const size_t blockSize = 64;
    vector<u8> k = key;
    if(k.size() > blockSize){
        u8 tmp[32]; SHA256Ctx c; sha256_init(c); sha256_update(c, k.data(), k.size()); sha256_final(c, tmp);
        k.assign(tmp, tmp+32);
    }
    if(k.size() < blockSize) k.resize(blockSize, 0x00);
    vector<u8> o_key(blockSize), i_key(blockSize);
    for(size_t i=0;i<blockSize;i++){ o_key[i] = k[i] ^ 0x5c; i_key[i] = k[i] ^ 0x36; }
    u8 inner[32];
    SHA256Ctx ci; sha256_init(ci); sha256_update(ci, i_key.data(), i_key.size()); sha256_update(ci, msg, msglen); sha256_final(ci, inner);
    SHA256Ctx co; sha256_init(co); sha256_update(co, o_key.data(), o_key.size()); sha256_update(co, inner, 32); sha256_final(co, out);
}

// Simple KDF: iterate HMAC-SHA256 many times (not PBKDF2 but key-stretching)
static void kdf_hmac_sha256(const string &password, const vector<u8> &salt, int iterations, u8 out[32]){
    vector<u8> key(password.begin(), password.end());
    u8 tmp[32];
    hmac_sha256(salt, (const u8*)key.data(), key.size(), tmp);
    for(int i=1;i<iterations;i++){
        hmac_sha256(salt, tmp, 32, tmp);
    }
    memcpy(out, tmp, 32);
    // zeroize tmp
    volatile u8 *vt = tmp;
    for(size_t i=0;i<32;i++) vt[i]=0;
}

static bool ct_equal(const vector<u8>& a, const vector<u8>& b){
    size_t maxlen = max(a.size(), b.size());
    unsigned char res = 0;
    for(size_t i=0;i<maxlen;i++){
        unsigned char x = i<a.size()?a[i]:0;
        unsigned char y = i<b.size()?b[i]:0;
        res |= (x ^ y);
    }
    return res==0 && a.size()==b.size();
}

struct User {
    string username;
    string email;
    vector<u8> salt;
    vector<u8> pwdHash; // 32 bytes
};

struct UserStore {
    unordered_map<string, User> byUsername;
    unordered_map<string, string> emailToUsername;

    static vector<u8> randomBytes(size_t n){
        vector<u8> v(n);
        std::random_device rd;
        for(size_t i=0;i<n;i++) v[i] = static_cast<u8>(rd());
        return v;
    }

    bool registerUser(const string& username, const string& email, const string& password){
        if(username.empty() || email.empty()) return false;
        if(byUsername.count(username)) return false;
        string emailKey = email; transform(emailKey.begin(), emailKey.end(), emailKey.begin(), ::tolower);
        if(emailToUsername.count(emailKey)) return false;
        vector<u8> salt = randomBytes(16);
        u8 out[32]; kdf_hmac_sha256(password, salt, 120000, out);
        User u{username, email, salt, vector<u8>(out, out+32)};
        byUsername[username] = u;
        emailToUsername[emailKey] = username;
        return true;
    }

    User* findByUsername(const string& username){
        auto it = byUsername.find(username);
        if(it==byUsername.end()) return nullptr;
        return &it->second;
    }

    User* findByEmail(const string& email){
        string key = email; transform(key.begin(), key.end(), key.begin(), ::tolower);
        auto it = emailToUsername.find(key);
        if(it==emailToUsername.end()) return nullptr;
        return findByUsername(it->second);
    }

    bool updateEmail(User* user, const string& newEmail){
        if(!user) return false;
        string oldKey = user->email; transform(oldKey.begin(), oldKey.end(), oldKey.begin(), ::tolower);
        string newKey = newEmail; transform(newKey.begin(), newKey.end(), newKey.begin(), ::tolower);
        if(emailToUsername.count(newKey)) return false;
        emailToUsername.erase(oldKey);
        user->email = newEmail;
        emailToUsername[newKey] = user->username;
        return true;
    }
};

struct AuthService {
    UserStore& store;
    User* currentUser{nullptr};

    static bool isValidEmail(const string& email){
        if(email.empty() || email.size()>254) return false;
        // Simple regex-like check
        const regex re("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$", std::regex_constants::icase);
        return regex_match(email, re);
    }

    bool login(const string& username, const string& password){
        User* u = store.findByUsername(username);
        if(!u) return false;
        u8 out[32]; kdf_hmac_sha256(password, u->salt, 120000, out);
        vector<u8> cand(out, out+32);
        if(ct_equal(cand, u->pwdHash)){
            currentUser = u;
            return true;
        }
        return false;
    }

    void logout(){ currentUser = nullptr; }

    bool isLoggedIn() const { return currentUser != nullptr; }

    bool changeEmail(const string& oldEmail, const string& newEmail, const string& confirmPassword){
        if(!isLoggedIn()) return false;
        if(!isValidEmail(oldEmail) || !isValidEmail(newEmail)) return false;
        string curEmail = currentUser->email;
        string a=oldEmail, b=curEmail;
        transform(a.begin(), a.end(), a.begin(), ::tolower);
        transform(b.begin(), b.end(), b.begin(), ::tolower);
        if(a!=b) return false;
        if(strcasecmp(oldEmail.c_str(), newEmail.c_str())==0) return false; // disallow no-op
        // verify password
        u8 out[32]; kdf_hmac_sha256(confirmPassword, currentUser->salt, 120000, out);
        vector<u8> cand(out, out+32);
        if(!ct_equal(cand, currentUser->pwdHash)) return false;
        // ensure emails in store
        if(store.findByEmail(oldEmail)==nullptr) return false;
        if(store.findByEmail(newEmail)!=nullptr) return false;
        return store.updateEmail(currentUser, newEmail);
    }
};

// Helper: case-insensitive compare for portability
static int strcasecmp_portable(const char* a, const char* b){
    for(;; a++, b++){
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if(ca != cb || ca == 0 || cb == 0) return ca - cb;
    }
}
#define strcasecmp strcasecmp_portable

int main(){
    UserStore store;
    AuthService auth{store};

    store.registerUser("alice", "alice@example.com", "StrongPass!1");
    store.registerUser("bob", "bob@example.com", "CorrectHorseBatteryStaple");

    // Test 1: success
    bool t1login = auth.login("alice", "StrongPass!1");
    bool t1change = auth.changeEmail("alice@example.com", "alice.new@example.com", "StrongPass!1");
    cout << "Test1 success=" << (t1login && t1change ? "true" : "false") << "\n";
    auth.logout();

    // Test 2: change without login
    bool t2 = auth.changeEmail("bob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple");
    cout << "Test2 success=" << (!t2 ? "true" : "false") << "\n";

    // Test 3: wrong old email
    bool t3login = auth.login("bob", "CorrectHorseBatteryStaple");
    bool t3change = auth.changeEmail("wrongbob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple");
    cout << "Test3 success=" << (t3login && !t3change ? "true" : "false") << "\n";
    auth.logout();

    // Test 4: wrong confirm password
    bool t4login = auth.login("bob", "CorrectHorseBatteryStaple");
    bool t4change = auth.changeEmail("bob@example.com", "bob.new@example.com", "wrongpassword");
    cout << "Test4 success=" << (t4login && !t4change ? "true" : "false") << "\n";
    auth.logout();

    // Test 5: new email taken and invalid
    bool t5login = auth.login("bob", "CorrectHorseBatteryStaple");
    bool t5taken = auth.changeEmail("bob@example.com", "alice.new@example.com", "CorrectHorseBatteryStaple");
    bool t5invalid = auth.changeEmail("bob@example.com", "not-an-email", "CorrectHorseBatteryStaple");
    cout << "Test5 success=" << (t5login && !t5taken && !t5invalid ? "true" : "false") << "\n";
    auth.logout();

    return 0;
}