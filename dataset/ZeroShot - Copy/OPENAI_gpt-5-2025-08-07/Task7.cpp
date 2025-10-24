#include <bits/stdc++.h>
using namespace std;

// -------- Utility: Hex encoding/decoding --------
static string bytes_to_hex(const vector<uint8_t>& v) {
    static const char* hexd = "0123456789abcdef";
    string out; out.resize(v.size()*2);
    for (size_t i=0;i<v.size();++i){ out[2*i]=hexd[v[i]>>4]; out[2*i+1]=hexd[v[i]&0xF]; }
    return out;
}
static string bytes_to_hex_raw(const uint8_t* d, size_t n) {
    static const char* hexd = "0123456789abcdef";
    string out; out.resize(n*2);
    for (size_t i=0;i<n;++i){ out[2*i]=hexd[d[i]>>4]; out[2*i+1]=hexd[d[i]&0xF]; }
    return out;
}
static bool hex_to_bytes(const string& hex, vector<uint8_t>& out) {
    if (hex.size()%2!=0) return false;
    size_t n=hex.size()/2; out.resize(n);
    auto val=[&](char c)->int{
        if (c>='0'&&c<='9') return c-'0';
        if (c>='a'&&c<='f') return c-'a'+10;
        if (c>='A'&&c<='F') return c-'A'+10;
        return -1;
    };
    for (size_t i=0;i<n;++i){
        int hi=val(hex[2*i]), lo=val(hex[2*i+1]);
        if (hi<0||lo<0) return false;
        out[i]=static_cast<uint8_t>((hi<<4)|lo);
    }
    return true;
}

// -------- Utility: URL percent-encoding/decoding --------
static string url_encode(const string& s){
    string out;
    for(unsigned char c: s){
        if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~'){
            out.push_back(c);
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", c);
            out += buf;
        }
    }
    return out;
}
static string url_decode(const string& s){
    string out;
    for (size_t i=0;i<s.size();){
        if (s[i]=='%' && i+2<s.size()){
            int hi, lo;
            auto val=[&](char c)->int{
                if (c>='0'&&c<='9') return c-'0';
                if (c>='a'&&c<='f') return c-'a'+10;
                if (c>='A'&&c<='F') return c-'A'+10;
                return -1;
            };
            hi=val(s[i+1]); lo=val(s[i+2]);
            if (hi>=0 && lo>=0){ out.push_back((char)((hi<<4)|lo)); i+=3; continue; }
        }
        if (s[i]=='+'){ out.push_back(' '); i++; }
        else { out.push_back(s[i]); i++; }
    }
    return out;
}

// -------- SHA-256 implementation --------
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

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

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0,j=0; i<16; ++i, j+=4)
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    for (; i<64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (i=0; i<64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0; i<len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;
    // Pad
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i=0; i<4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xFF;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xFF;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xFF;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xFF;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xFF;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xFF;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xFF;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xFF;
    }
}

// -------- KDF: iterated salted SHA-256 --------
static vector<uint8_t> kdf_iter_sha256(const string& password, const vector<uint8_t>& salt, int iterations) {
    if (iterations < 1) iterations = 1;
    vector<uint8_t> first; first.reserve(salt.size() + password.size());
    first.insert(first.end(), salt.begin(), salt.end());
    first.insert(first.end(), password.begin(), password.end());

    uint8_t digest[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, first.data(), first.size());
    sha256_final(&ctx, digest);

    for (int i=1; i<iterations; ++i) {
        sha256_init(&ctx);
        sha256_update(&ctx, digest, 32);
        sha256_final(&ctx, digest);
    }
    vector<uint8_t> out(32);
    memcpy(out.data(), digest, 32);
    memset(digest, 0, sizeof(digest));
    return out;
}

static bool ct_equal(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i=0;i<a.size();++i) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

// -------- Client --------
static string buildLoginRequest(const string& username, const string& password) {
    string u = username;
    string p = password;
    if (u.empty() || u.size() > 128 || p.empty() || p.size() > 256) return "";
    string ue = url_encode(u);
    string pe = url_encode(p);
    return string("u=") + ue + "&p=" + pe;
}

// -------- Server --------
struct UserRecord {
    string username;
    int iterations;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
};

static bool lookupUser(const string& dbPath, const string& username, UserRecord& out) {
    ifstream in(dbPath);
    if (!in) return false;
    string line;
    while (getline(in, line)) {
        if (line.empty() || line[0]=='#') continue;
        vector<string> parts;
        {
            string tmp; stringstream ss(line);
            while (getline(ss, tmp, ':')) parts.push_back(tmp);
        }
        if (parts.size()!=4) continue;
        if (parts[0] != username) continue;
        int iters = 1;
        try {
            iters = stoi(parts[1]);
        } catch (...) { continue; }
        vector<uint8_t> salt, hash;
        if (!hex_to_bytes(parts[2], salt)) continue;
        if (!hex_to_bytes(parts[3], hash)) continue;
        out.username = username;
        out.iterations = iters;
        out.salt = move(salt);
        out.hash = move(hash);
        return true;
    }
    return false;
}

static string processRequest(const string& request, const string& dbPath) {
    if (request.empty()) return "ERR: Malformed request";
    string uEnc, pEnc;
    {
        // parse query
        string s = request;
        size_t a = s.find("u=");
        size_t amp = s.find("&");
        if (a!=string::npos && amp!=string::npos && a==0) {
            uEnc = s.substr(2, amp-2);
            if (amp+3 <= s.size() && s.substr(amp+1,2)=="p=") pEnc = s.substr(amp+3);
        } else {
            // generic parse
            vector<string> parts; string tmp; stringstream ss(s);
            while (getline(ss, tmp, '&')) parts.push_back(tmp);
            for (auto& kv : parts) {
                auto pos = kv.find('=');
                if (pos==string::npos) continue;
                auto k = kv.substr(0,pos);
                auto v = kv.substr(pos+1);
                if (k=="u") uEnc = v; else if (k=="p") pEnc = v;
            }
        }
    }
    if (uEnc.empty() || pEnc.empty()) return "ERR: Malformed request";
    string username = url_decode(uEnc);
    string password = url_decode(pEnc);
    if (username.empty() || password.empty()) return "ERR: Missing credentials";

    UserRecord rec;
    if (!lookupUser(dbPath, username, rec)) return "ERR: Invalid credentials";
    vector<uint8_t> derived = kdf_iter_sha256(password, rec.salt, rec.iterations);
    bool ok = ct_equal(derived, rec.hash);
    fill(derived.begin(), derived.end(), 0);
    return ok ? "OK" : "ERR: Invalid credentials";
}

// -------- DB creation --------
static void createUserDb(const string& path, const vector<pair<string,string>>& users, int iterations) {
    ofstream out(path, ios::out | ios::trunc);
    if (!out) throw runtime_error("Cannot open db for writing");
    std::random_device rd;
    for (auto& up : users) {
        if (up.first.empty() || up.second.empty()) continue;
        vector<uint8_t> salt(16);
        for (auto& b : salt) b = static_cast<uint8_t>(rd());
        vector<uint8_t> hash = kdf_iter_sha256(up.second, salt, iterations);
        out << up.first << ":" << iterations << ":" << bytes_to_hex(salt) << ":" << bytes_to_hex(hash) << "\n";
    }
    out.close();
    // Best-effort permission tightening (POSIX)
#if defined(__unix__) || defined(__APPLE__)
    ::chmod(path.c_str(), 0600);
#endif
}

// -------- Main with 5 test cases --------
int main() {
    try {
        string dbPath = "userdb_cpp.txt";
        vector<pair<string,string>> users = {
            {"alice","Password123!"},
            {"bob","S3cur3Pa$$"},
            {"charlie","letmein"}
        };
        createUserDb(dbPath, users, 100000);

        vector<pair<string,string>> tests = {
            {"alice","Password123!"},
            {"alice","wrong"},
            {"dave","doesntmatter"},
            {"bob",""},
            {"bob","S3cur3Pa$$"}
        };
        for (size_t i=0;i<tests.size();++i) {
            string req = buildLoginRequest(tests[i].first, tests[i].second);
            string resp = processRequest(req, dbPath);
            cout << "Test " << (i+1) << " -> " << resp << "\n";
        }
        remove(dbPath.c_str());
    } catch (const exception& e) {
        cout << "ERR: " << e.what() << "\n";
    }
    return 0;
}