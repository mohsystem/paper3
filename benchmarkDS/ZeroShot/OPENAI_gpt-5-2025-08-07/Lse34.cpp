#include <bits/stdc++.h>
using namespace std;

/* Minimal SHA-256 + HMAC + PBKDF2 implementation */

struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

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

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 | (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(m[i-15],7) ^ rotr(m[i-15],18) ^ (m[i-15] >> 3);
        uint32_t s1 = rotr(m[i-2],17) ^ rotr(m[i-2],19) ^ (m[i-2] >> 10);
        m[i] = m[i-16] + s0 + m[i-7] + s1;
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + K256[i] + m[i];
        uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul; ctx->state[1] = 0xbb67ae85ul; ctx->state[2] = 0x3c6ef372ul; ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful; ctx->state[5] = 0x9b05688cul; ctx->state[6] = 0x1f83d9abul; ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
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

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    size_t i = ctx->datalen;
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
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >>  8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i*8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i*8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i*8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i*8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i*8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i*8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i*8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i*8)) & 0xff);
    }
}

static vector<uint8_t> sha256(const vector<uint8_t>& data) {
    SHA256_CTX ctx; sha256_init(&ctx);
    sha256_update(&ctx, data.data(), data.size());
    vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

static vector<uint8_t> hmac_sha256(const vector<uint8_t>& key, const vector<uint8_t>& data) {
    const size_t blockSize = 64;
    vector<uint8_t> k(blockSize, 0);
    if (key.size() > blockSize) {
        auto hashed = sha256(key);
        copy(hashed.begin(), hashed.end(), k.begin());
    } else {
        copy(key.begin(), key.end(), k.begin());
    }
    vector<uint8_t> o_key_pad(blockSize), i_key_pad(blockSize);
    for (size_t i = 0; i < blockSize; ++i) {
        o_key_pad[i] = (uint8_t)(k[i] ^ 0x5c);
        i_key_pad[i] = (uint8_t)(k[i] ^ 0x36);
    }
    vector<uint8_t> inner(i_key_pad);
    inner.insert(inner.end(), data.begin(), data.end());
    auto inner_hash = sha256(inner);
    vector<uint8_t> outer(o_key_pad);
    outer.insert(outer.end(), inner_hash.begin(), inner_hash.end());
    return sha256(outer);
}

static vector<uint8_t> pbkdf2_hmac_sha256(const string& password, const vector<uint8_t>& salt, uint32_t iterations, size_t dkLen) {
    vector<uint8_t> keyBytes(password.begin(), password.end());
    size_t hLen = 32;
    size_t l = (dkLen + hLen - 1) / hLen;
    vector<uint8_t> dk(dkLen);
    vector<uint8_t> saltBlock(salt);
    for (size_t i = 1; i <= l; ++i) {
        vector<uint8_t> sb = saltBlock;
        sb.push_back((uint8_t)(i >> 24));
        sb.push_back((uint8_t)(i >> 16));
        sb.push_back((uint8_t)(i >> 8));
        sb.push_back((uint8_t)(i));
        auto u = hmac_sha256(keyBytes, sb);
        vector<uint8_t> t = u;
        for (uint32_t j = 2; j <= iterations; ++j) {
            u = hmac_sha256(keyBytes, u);
            for (size_t k = 0; k < hLen; ++k) t[k] ^= u[k];
        }
        size_t offset = (i - 1) * hLen;
        size_t clen = min(hLen, dkLen - offset);
        memcpy(dk.data() + offset, t.data(), clen);
    }
    // best effort wipe
    fill(keyBytes.begin(), keyBytes.end(), 0);
    return dk;
}

static bool secure_eq(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

struct UserRecord {
    string username;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
};

using Database = unordered_map<string, UserRecord>;

static vector<uint8_t> random_bytes(size_t n) {
    vector<uint8_t> out(n);
    std::random_device rd;
    for (size_t i = 0; i < n; ++i) out[i] = static_cast<uint8_t>(rd());
    return out;
}

static UserRecord create_user_record(const string& username, const string& password) {
    vector<uint8_t> salt = random_bytes(16);
    vector<uint8_t> hash = pbkdf2_hmac_sha256(password, salt, 100000, 32);
    return UserRecord{username, salt, hash};
}

string authenticate_and_redirect(Database& db, const string& username, const string& password) {
    if (username.empty() || password.empty()) return "redirect:/login";
    auto it = db.find(username);
    if (it == db.end()) return "redirect:/login";
    auto computed = pbkdf2_hmac_sha256(password, it->second.salt, 100000, 32);
    bool ok = secure_eq(computed, it->second.hash);
    // best-effort wipe of computed
    fill(computed.begin(), computed.end(), 0);
    return ok ? "redirect:/dashboard" : "redirect:/login";
}

int main() {
    Database db;
    db["alice"] = create_user_record("alice", "S3curePass!");
    db["bob"]   = create_user_record("bob", "hunter2?");

    cout << authenticate_and_redirect(db, "alice", "S3curePass!") << "\n"; // dashboard
    cout << authenticate_and_redirect(db, "alice", "wrongpass") << "\n";   // login
    cout << authenticate_and_redirect(db, "unknown", "anything") << "\n";  // login
    cout << authenticate_and_redirect(db, "bob", "hunter2?") << "\n";      // dashboard
    cout << authenticate_and_redirect(db, "", "") << "\n";                 // login
    return 0;
}