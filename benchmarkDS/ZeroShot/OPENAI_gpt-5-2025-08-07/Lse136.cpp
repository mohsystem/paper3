#include <bits/stdc++.h>
using namespace std;

// SHA-256 implementation (public domain style)
struct SHA256_CTX {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
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

static uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static uint32_t ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static uint32_t maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t ep0(uint32_t x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static uint32_t ep1(uint32_t x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static uint32_t sig0(uint32_t x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static uint32_t sig1(uint32_t x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i]  = (uint32_t)data[i*4] << 24;
        m[i] |= (uint32_t)data[i*4+1] << 16;
        m[i] |= (uint32_t)data[i*4+2] << 8;
        m[i] |= (uint32_t)data[i*4+3];
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + k256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
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
    size_t i = ctx->datalen;

    // Pad remaining data
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0xff);
    }
}

static vector<uint8_t> sha256_bytes(const string &s) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, reinterpret_cast<const uint8_t*>(s.data()), s.size());
    vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

static bool ct_eq_vec(const vector<uint8_t> &a, const vector<uint8_t> &b) {
    size_t m = max(a.size(), b.size());
    unsigned int diff = (unsigned int)(a.size() ^ b.size());
    for (size_t i = 0; i < m; ++i) {
        uint8_t av = i < a.size() ? a[i] : 0;
        uint8_t bv = i < b.size() ? b[i] : 0;
        diff |= (unsigned int)(av ^ bv);
    }
    return diff == 0;
}

static bool ct_eq_str(const string &a, const string &b) {
    size_t m = max(a.size(), b.size());
    unsigned int diff = (unsigned int)(a.size() ^ b.size());
    for (size_t i = 0; i < m; ++i) {
        unsigned char av = i < a.size() ? (unsigned char)a[i] : 0;
        unsigned char bv = i < b.size() ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(av ^ bv);
    }
    return diff == 0;
}

static bool hexval(char c, uint8_t &v) {
    if (c >= '0' && c <= '9') { v = (uint8_t)(c - '0'); return true; }
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') { v = (uint8_t)(10 + c - 'a'); return true; }
    return false;
}

static bool url_decode(const string &in, string &out, size_t maxLen) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '%') {
            if (i + 2 >= in.size()) return false;
            uint8_t hi, lo;
            if (!hexval(in[i+1], hi) || !hexval(in[i+2], lo)) return false;
            char decoded = (char)((hi << 4) | lo);
            out.push_back(decoded);
            i += 2;
        } else if (c == '+') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
        if (out.size() > maxLen) return false;
    }
    return true;
}

static void split_url(const string &url, string &path, string &query) {
    size_t qpos = url.find('?');
    if (qpos == string::npos) {
        path = url;
        query.clear();
    } else {
        path = url.substr(0, qpos);
        query = url.substr(qpos + 1);
    }
}

static unordered_map<string,string> parse_query(const string &query) {
    unordered_map<string,string> params;
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        string pair = (amp == string::npos) ? query.substr(start) : query.substr(start, amp - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            string k = (eq == string::npos) ? pair : pair.substr(0, eq);
            string v = (eq == string::npos) ? "" : pair.substr(eq + 1);
            string dk, dv;
            if (url_decode(k, dk, 64) && url_decode(v, dv, 512)) {
                if (!params.count(dk)) {
                    params[dk] = dv;
                }
            }
        }
        if (amp == string::npos) break;
        start = amp + 1;
    }
    return params;
}

static bool is_valid_username(const string &u) {
    if (u.empty() || u.size() > 64) return false;
    for (char c : u) {
        if (!(c == '_' || c == '-' || c == '.' ||
              (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9'))) return false;
    }
    return true;
}

string check_mod(const string &url) {
    static const string SALT = "u$er-salt::";
    static const string EXPECTED_USERNAME = "admin";
    static const string EXPECTED_PASSWORD = "s3cureP@ss";

    string path, query;
    split_url(url, path, query);
    if (path != "/check_mod") return "false";
    auto params = parse_query(query);
    auto itU = params.find("username");
    auto itP = params.find("password");
    if (itU == params.end() || itP == params.end()) return "false";
    const string &username = itU->second;
    const string &password = itP->second;

    if (!is_valid_username(username)) return "false";
    if (password.empty() || password.size() > 256) return "false";

    if (!ct_eq_str(username, EXPECTED_USERNAME)) return "false";

    vector<uint8_t> expectedHash = sha256_bytes(SALT + EXPECTED_PASSWORD);
    vector<uint8_t> providedHash = sha256_bytes(SALT + password);
    if (!ct_eq_vec(expectedHash, providedHash)) return "false";

    return "true";
}

int main() {
    vector<string> tests = {
        "/check_mod?username=admin&password=s3cureP%40ss",
        "/check_mod?username=admin&password=wrong",
        "/check_mod?username=Admin&password=s3cureP%40ss",
        "/check_mod?username=admin",
        "/other?username=admin&password=s3cureP%40ss"
    };
    for (auto &t : tests) {
        cout << check_mod(t) << "\n";
    }
    return 0;
}