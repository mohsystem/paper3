#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation (public domain)
namespace sha256_impl {
    typedef uint32_t u32;
    typedef uint64_t u64;

    static inline u32 rotr(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }
    static inline u32 Ch(u32 x, u32 y, u32 z){ return (x & y) ^ (~x & z); }
    static inline u32 Maj(u32 x, u32 y, u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
    static inline u32 Sigma0(u32 x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
    static inline u32 Sigma1(u32 x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
    static inline u32 sigma0(u32 x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
    static inline u32 sigma1(u32 x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

    static const u32 K[64] = {
        0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
        0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
        0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
        0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
        0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
        0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
        0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
        0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
    };

    struct SHA256 {
        u32 h[8];
        vector<unsigned char> buf;
        u64 bits;

        SHA256(){ init(); }

        void init() {
            h[0]=0x6a09e667ul; h[1]=0xbb67ae85ul; h[2]=0x3c6ef372ul; h[3]=0xa54ff53aul;
            h[4]=0x510e527ful; h[5]=0x9b05688cul; h[6]=0x1f83d9abul; h[7]=0x5be0cd19ul;
            buf.clear();
            bits = 0;
        }

        void process_block(const unsigned char block[64]) {
            u32 w[64];
            for (int i=0;i<16;i++) {
                w[i] = (u32)block[i*4]<<24 | (u32)block[i*4+1]<<16 | (u32)block[i*4+2]<<8 | (u32)block[i*4+3];
            }
            for (int i=16;i<64;i++) {
                w[i] = sigma1(w[i-2]) + w[i-7] + sigma0(w[i-15]) + w[i-16];
            }
            u32 a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
            for (int i=0;i<64;i++) {
                u32 t1 = hh + Sigma1(e) + Ch(e,f,g) + K[i] + w[i];
                u32 t2 = Sigma0(a) + Maj(a,b,c);
                hh = g; g = f; f = e; e = d + t1;
                d = c; c = b; b = a; a = t1 + t2;
            }
            h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
        }

        void update(const unsigned char* data, size_t len) {
            bits += (u64)len * 8;
            size_t i = 0;
            if (!buf.empty()) {
                while (i < len && buf.size() < 64) {
                    buf.push_back(data[i++]);
                }
                if (buf.size() == 64) {
                    process_block(buf.data());
                    buf.clear();
                }
            }
            while (i + 64 <= len) {
                process_block(data + i);
                i += 64;
            }
            while (i < len) {
                buf.push_back(data[i++]);
            }
        }

        void final(unsigned char out[32]) {
            vector<unsigned char> tmp = buf;
            tmp.push_back(0x80);
            while ((tmp.size() % 64) != 56) tmp.push_back(0x00);
            unsigned char lenbytes[8];
            for (int i=0;i<8;i++) lenbytes[7-i] = (unsigned char)((bits >> (i*8)) & 0xFF);
            tmp.insert(tmp.end(), lenbytes, lenbytes+8);
            for (size_t i=0;i<tmp.size(); i+=64) {
                process_block(&tmp[i]);
            }
            for (int i=0;i<8;i++) {
                out[i*4]   = (unsigned char)((h[i] >> 24) & 0xFF);
                out[i*4+1] = (unsigned char)((h[i] >> 16) & 0xFF);
                out[i*4+2] = (unsigned char)((h[i] >> 8) & 0xFF);
                out[i*4+3] = (unsigned char)(h[i] & 0xFF);
            }
        }
    };

    vector<unsigned char> sha256(const vector<unsigned char>& data) {
        SHA256 s; s.update(data.data(), data.size());
        vector<unsigned char> out(32);
        s.final(out.data());
        return out;
    }
}

struct Task7 {
    // Client: build request
    static string buildLoginRequest(const string& username, const string& password) {
        if (username.size() > 128 || password.size() > 256) return "";
        return string("op=login;user=") + escape(username) + ";pass=" + escape(password);
    }

    // Server: process request
    static string processRequest(const string& userStorePath, const string& request) {
        if (request.empty() || request.size() > 4096) return "ERR:invalid request size";
        unordered_map<string,string> kv = parseKV(request);
        auto itop = kv.find("op");
        if (itop == kv.end() || itop->second != "login") return "ERR:unsupported op";
        auto itu = kv.find("user");
        auto itp = kv.find("pass");
        if (itu == kv.end() || itp == kv.end()) return "ERR:missing fields";
        string user = unescape(itu->second);
        string pass = unescape(itp->second);
        if (!isValidUsername(user)) return "ERR:invalid username";
        try {
            bool ok = authenticate(userStorePath, user, pass);
            return ok ? string("OK:Welcome ") + user : "ERR:authentication failed";
        } catch (...) {
            return "ERR:server io error";
        }
    }

    // Admin: create or update user
    static bool createOrUpdateUser(const string& userStorePath, const string& username, const string& password) {
        if (!isValidUsername(username)) return false;
        int iterations = 10000;
        vector<unsigned char> salt = secureRandomBytes(16);
        string saltHex = toHex(salt);
        string hash = hashPassword(password, salt, iterations);

        // Load existing
        map<string,string> records;
        {
            ifstream in(userStorePath);
            if (in) {
                string line;
                while (getline(in, line)) {
                    if (line.empty()) continue;
                    size_t pos = line.find(':');
                    if (pos == string::npos) continue;
                    string u = line.substr(0, pos);
                    records[u] = line;
                }
            }
        }
        records[username] = username + ":" + saltHex + ":" + to_string(iterations) + ":" + hash;
        string tmp = userStorePath + ".tmp";
        {
            ofstream out(tmp, ios::trunc);
            if (!out) return false;
            for (auto& kv : records) {
                out << kv.second << "\n";
            }
        }
        // Replace
        remove(userStorePath.c_str());
        if (rename(tmp.c_str(), userStorePath.c_str()) != 0) {
            remove(tmp.c_str());
            return false;
        }
        return true;
    }

    // Authentication
    static bool authenticate(const string& userStorePath, const string& username, const string& password) {
        ifstream in(userStorePath);
        if (!in) return false;
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            vector<string> parts = split(line, ':');
            if (parts.size() != 4) continue;
            if (parts[0] != username) continue;
            string saltHex = parts[1];
            string iterStr = parts[2];
            string storedHash = parts[3];
            int iterations = 0;
            try { iterations = stoi(iterStr); } catch (...) { return false; }
            if (iterations < 1000 || iterations > 200000) return false;
            vector<unsigned char> salt = fromHex(saltHex);
            if (salt.empty()) return false;
            string computed = hashPassword(password, salt, iterations);
            return constantTimeEq(storedHash, computed);
        }
        return false;
    }

    // Utilities
    static bool isValidUsername(const string& u) {
        if (u.empty() || u.size() > 64) return false;
        for (unsigned char c : u) {
            if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
        }
        return true;
    }

    static string escape(const string& s) {
        string out;
        out.reserve(s.size() + 8);
        for (char c : s) {
            if (c == '\\' || c == ';' || c == '=') out.push_back('\\');
            out.push_back(c);
        }
        return out;
    }

    static string unescape(const string& s) {
        string out;
        out.reserve(s.size());
        bool esc = false;
        for (char c : s) {
            if (esc) { out.push_back(c); esc = false; }
            else if (c == '\\') esc = true;
            else out.push_back(c);
        }
        return out;
    }

    static unordered_map<string,string> parseKV(const string& s) {
        unordered_map<string,string> m;
        string key, val;
        bool inKey = true, esc = false;
        for (char c : s) {
            if (esc) {
                if (inKey) key.push_back(c); else val.push_back(c);
                esc = false;
            } else {
                if (c == '\\') esc = true;
                else if (c == '=' && inKey) inKey = false;
                else if (c == ';') {
                    if (!key.empty()) m[key] = val;
                    key.clear(); val.clear(); inKey = true;
                } else {
                    if (inKey) key.push_back(c); else val.push_back(c);
                }
            }
        }
        if (!key.empty()) m[key] = val;
        return m;
    }

    static vector<unsigned char> sha256(const vector<unsigned char>& data) {
        return sha256_impl::sha256(data);
    }

    static string hashPassword(const string& password, const vector<unsigned char>& salt, int iterations) {
        vector<unsigned char> init; init.reserve(salt.size() + password.size());
        init.insert(init.end(), salt.begin(), salt.end());
        init.insert(init.end(), password.begin(), password.end());
        vector<unsigned char> h = sha256(init);
        for (int i = 0; i < iterations; ++i) {
            vector<unsigned char> tmp; tmp.reserve(h.size() + salt.size());
            tmp.insert(tmp.end(), h.begin(), h.end());
            tmp.insert(tmp.end(), salt.begin(), salt.end());
            h = sha256(tmp);
        }
        return toHex(h);
    }

    static vector<unsigned char> secureRandomBytes(size_t len) {
        vector<unsigned char> out(len);
        std::random_device rd;
        for (size_t i = 0; i < len; ++i) out[i] = static_cast<unsigned char>(rd());
        return out;
    }

    static string toHex(const vector<unsigned char>& data) {
        static const char* hex = "0123456789abcdef";
        string out; out.resize(data.size()*2);
        for (size_t i=0;i<data.size();++i) {
            unsigned v = data[i];
            out[2*i] = hex[v >> 4];
            out[2*i+1] = hex[v & 0xF];
        }
        return out;
    }

    static vector<unsigned char> fromHex(const string& s) {
        if (s.size() % 2) return {};
        vector<unsigned char> out(s.size()/2);
        for (size_t i=0;i<out.size();++i) {
            auto hexval = [](char c)->int{
                if (c>='0'&&c<='9') return c-'0';
                if (c>='a'&&c<='f') return c-'a'+10;
                if (c>='A'&&c<='F') return c-'A'+10;
                return -1;
            };
            int hi = hexval(s[2*i]);
            int lo = hexval(s[2*i+1]);
            if (hi<0||lo<0) return {};
            out[i] = (unsigned char)((hi<<4)|lo);
        }
        return out;
    }

    static vector<string> split(const string& s, char delim) {
        vector<string> out;
        string cur;
        for (char c : s) {
            if (c == delim) { out.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        out.push_back(cur);
        return out;
    }

    static bool constantTimeEq(const string& a, const string& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i=0;i<a.size();++i) diff |= (unsigned char)(a[i] ^ b[i]);
        return diff == 0;
    }
};

int main() {
    string store = "users_cpp.db";
    Task7::createOrUpdateUser(store, "alice", "Wonder@123");
    Task7::createOrUpdateUser(store, "bob", "Builder#456");

    vector<string> reqs;
    reqs.push_back(Task7::buildLoginRequest("alice", "Wonder@123"));
    reqs.push_back(Task7::buildLoginRequest("alice", "wrongpass"));
    reqs.push_back(Task7::buildLoginRequest("charlie", "Anything1!"));
    reqs.push_back(Task7::buildLoginRequest("bob", "Builder#456"));
    reqs.push_back(Task7::buildLoginRequest("bad;user", "str@nge=pass"));

    for (size_t i=0;i<reqs.size();++i) {
        cout << "Case " << (i+1) << ": " << Task7::processRequest(store, reqs[i]) << "\n";
    }
    return 0;
}