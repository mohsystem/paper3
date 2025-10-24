#include <bits/stdc++.h>
using namespace std;

// Simple file-based "database"
static const string DB_FILE = "users_cpp.db";
static const size_t SALT_LEN = 16;
static const int HASH_ITERATIONS = 100000;
static const size_t MAX_NAME_LEN = 100;

// -------------------- SHA-256 implementation (public domain style) --------------------
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

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

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]){
    uint32_t a,b,c,d,e,f,g,h,t1,t2,m[64];
    for (int i=0,j=0;i<16;i++,j+=4)
        m[i] = (uint32_t)data[j]<<24 | (uint32_t)data[j+1]<<16 | (uint32_t)data[j+2]<<8 | (uint32_t)data[j+3];
    for (int i=16;i<64;i++)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (int i=0;i<64;i++){
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }

    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX *ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]){
    uint32_t i = ctx->datalen;
    // Pad
    if (ctx->datalen < 56){
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;
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
        hash[i]    = (ctx->state[0] >> (24 - i * 8)) & 0xff;
        hash[i+4]  = (ctx->state[1] >> (24 - i * 8)) & 0xff;
        hash[i+8]  = (ctx->state[2] >> (24 - i * 8)) & 0xff;
        hash[i+12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
        hash[i+16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
        hash[i+20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
        hash[i+24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
        hash[i+28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
    }
}

// -------------------- Utilities --------------------
static string toHex(const vector<uint8_t>& v){
    static const char* hexd="0123456789abcdef";
    string s; s.reserve(v.size()*2);
    for (auto b : v){ s.push_back(hexd[b>>4]); s.push_back(hexd[b&0xF]); }
    return s;
}
static string toHex(const uint8_t* d, size_t n){
    static const char* hexd="0123456789abcdef";
    string s; s.reserve(n*2);
    for (size_t i=0;i<n;i++){ uint8_t b=d[i]; s.push_back(hexd[b>>4]); s.push_back(hexd[b&0xF]); }
    return s;
}

static bool isValidEmail(const string& email){
    if (email.empty() || email.size() > 254) return false;
    auto at = email.find('@');
    if (at == string::npos || at == 0 || at == email.size()-1) return false;
    if (email.find('@', at+1) != string::npos) return false;
    auto dot = email.find('.', at);
    if (dot == string::npos || dot == at+1 || dot == email.size()-1) return false;
    for (char c: email){
        if (!(isalnum((unsigned char)c) || c=='.' || c=='-' || c=='_' || c=='+' || c=='%' || c=='@')) return false;
    }
    return true;
}

static bool validatePassword(const string& password, string& err){
    if (password.size() < 8){ err = "Error: Password too short."; return false; }
    bool lo=false, up=false, di=false, sp=false;
    for (char c: password){
        if (islower((unsigned char)c)) lo=true;
        else if (isupper((unsigned char)c)) up=true;
        else if (isdigit((unsigned char)c)) di=true;
        else sp=true;
    }
    if (!(lo && up && di && sp)){ err = "Error: Password must include upper, lower, digit, and special."; return false; }
    return true;
}

static bool isValidName(const string& name){
    if (name.empty() || name.size() > MAX_NAME_LEN) return false;
    if (!isalpha((unsigned char)name[0])) return false;
    for (char c: name){
        if (!(isalpha((unsigned char)c) || c==' ' || c=='\'' || c=='-')) return false;
    }
    return true;
}

static string trim(const string& s){
    size_t a=0, b=s.size();
    while (a<b && isspace((unsigned char)s[a])) a++;
    while (b>a && isspace((unsigned char)s[b-1])) b--;
    return s.substr(a,b-a);
}

static bool emailExists(const string& email){
    ifstream in(DB_FILE);
    if (!in) return false;
    string line;
    while (getline(in, line)){
        vector<string> parts;
        size_t start=0;
        for (int i=0;i<3;i++){
            size_t p = line.find('|', start);
            parts.push_back(line.substr(start, p == string::npos ? string::npos : p - start));
            if (p == string::npos) break;
            start = p+1;
        }
        if (parts.size() >= 2){
            string existing = parts[1];
            string a=email, b=existing;
            transform(a.begin(), a.end(), a.begin(), ::tolower);
            transform(b.begin(), b.end(), b.begin(), ::tolower);
            if (a == b) return true;
        }
    }
    return false;
}

static string escapeField(const string& s){
    string r;
    r.reserve(s.size());
    for (char c: s){
        if (c=='\n' || c=='\r') continue;
        r.push_back(c);
    }
    return r;
}

static void secureRandomBytes(uint8_t* out, size_t n){
    // Try /dev/urandom
    ifstream ur("/dev/urandom", ios::in | ios::binary);
    if (ur){
        ur.read(reinterpret_cast<char*>(out), n);
        if ((size_t)ur.gcount() == n) return;
    }
    // Fallback to random_device
    std::random_device rd;
    for (size_t i=0;i<n;i++){
        out[i] = static_cast<uint8_t>(rd());
    }
}

static vector<uint8_t> sha256(const vector<uint8_t>& data){
    SHA256_CTX ctx; sha256_init(&ctx);
    sha256_update(&ctx, data.data(), data.size());
    vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

// Hash password with salt and iterations (simple iterative SHA-256)
static vector<uint8_t> hashPassword(const string& password, const vector<uint8_t>& salt, int iterations){
    vector<uint8_t> buf; buf.reserve(salt.size() + password.size());
    buf.insert(buf.end(), salt.begin(), salt.end());
    buf.insert(buf.end(), password.begin(), password.end());
    vector<uint8_t> h = sha256(buf);
    for (int i=1;i<iterations;i++){
        h = sha256(h);
    }
    return h;
}

// Public API
string registerUser(const string& name, const string& email, const string& password){
    string n = trim(name);
    string e = trim(email);
    if (n.empty() || e.empty() || password.empty()) return "Error: Null or empty input.";
    if (!isValidName(n)) return "Error: Name contains invalid characters or length.";
    if (!isValidEmail(e)) return "Error: Invalid email format.";
    if (n.find('|') != string::npos || e.find('|') != string::npos) return "Error: Invalid characters in input.";

    string perr;
    if (!validatePassword(password, perr)) return perr;

    string normEmail = e;
    transform(normEmail.begin(), normEmail.end(), normEmail.begin(), ::tolower);

    if (emailExists(normEmail)) return "Error: Email already registered.";

    vector<uint8_t> salt(SALT_LEN);
    secureRandomBytes(salt.data(), salt.size());
    vector<uint8_t> hash = hashPassword(password, salt, HASH_ITERATIONS);

    ofstream out(DB_FILE, ios::app);
    if (!out) return "Error: Cannot open database file.";
    out << escapeField(n) << "|" << escapeField(normEmail) << "|" << toHex(salt) << "|" << toHex(hash) << "\n";
    out.close();
    return "Success: User registered.";
}

int main(){
    // Clean previous DB for demonstration
    remove(DB_FILE.c_str());

    vector<tuple<string,string,string>> tests = {
        {"Alice Johnson", "alice@example.com", "Str0ng!Pass"},
        {"Bob O'Neil", "bob.oneil@example.org", "An0ther$Good"},
        {"Invalid|Name", "bademail@@example.com", "weakpass"},
        {"Charly", "alice@example.com", "Str0ng!Pass"},
        {"Dana Smith", "dana.smith@example.co", "Y3t@NotherGood"}
    };
    for (size_t i=0;i<tests.size();i++){
        cout << "Test " << (i+1) << ": " << registerUser(get<0>(tests[i]), get<1>(tests[i]), get<2>(tests[i])) << "\n";
    }
    return 0;
}