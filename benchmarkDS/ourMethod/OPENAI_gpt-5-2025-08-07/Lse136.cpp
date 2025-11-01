#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <limits>
#include <sstream>
#include <algorithm>

extern "C" {
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
}

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

static void secure_zero(void* v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(v, n, 0, n);
#else
    volatile unsigned char* p = reinterpret_cast<volatile unsigned char*>(v);
    while (n--) { *p++ = 0; }
#endif
}

static bool is_username_valid(const std::string& u) {
    if (u.size() < 3 || u.size() > 64) return false;
    for (unsigned char c : u) {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool is_password_valid_policy(const std::string& p) {
    if (p.size() < 12 || p.size() > 128) return false;
    bool U=false,L=false,D=false,S=false;
    for (unsigned char c : p) {
        if (std::isupper(c)) U = true;
        else if (std::islower(c)) L = true;
        else if (std::isdigit(c)) D = true;
        else S = true;
    }
    return U && L && D && S;
}

static bool pbkdf2_sha256(const std::string& password, const std::vector<unsigned char>& salt, int iterations, std::vector<unsigned char>& out) {
    out.assign(32, 0);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), static_cast<int>(out.size()), out.data()) != 1) {
        return false;
    }
    return true;
}

static UserRecord create_user_record(const std::string& password, int iterations) {
    UserRecord rec;
    rec.salt.assign(16, 0);
    RAND_bytes(rec.salt.data(), static_cast<int>(rec.salt.size()));
    rec.iterations = iterations;
    rec.hash.assign(32, 0);
    pbkdf2_sha256(password, rec.salt, rec.iterations, rec.hash);
    return rec;
}

static bool verify_password(const std::string& password, const UserRecord& rec) {
    std::vector<unsigned char> derived(32, 0);
    if (!pbkdf2_sha256(password, rec.salt, rec.iterations, derived)) return false;
    bool ok = (CRYPTO_memcmp(derived.data(), rec.hash.data(), derived.size()) == 0);
    secure_zero(derived.data(), derived.size());
    return ok;
}

static bool url_percent_decode(const std::string& in, std::string& out, size_t max_len) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (out.size() >= max_len) return false;
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size() &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 1])) &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 2]))) {
            auto hex = [](char ch)->int {
                if (ch >= '0' && ch <= '9') return ch - '0';
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
                return -1;
            };
            int hi = hex(in[i + 1]);
            int lo = hex(in[i + 2]);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return out.size() <= max_len;
}

static void parse_query(const std::string& query, std::unordered_map<std::string, std::string>& params, size_t max_pairs, size_t max_key_val) {
    params.clear();
    if (query.empty()) return;
    size_t start = 0;
    size_t count = 0;
    while (start <= query.size() && count < max_pairs) {
        size_t end = query.find('&', start);
        if (end == std::string::npos) end = query.size();
        std::string pair = query.substr(start, end - start);
        size_t eq = pair.find('=');
        std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string v = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        std::string dk, dv;
        if (!url_percent_decode(k, dk, max_key_val) || !url_percent_decode(v, dv, max_key_val)) { params.clear(); return; }
        if (!dk.empty() && params.find(dk) == params.end()) {
            params.emplace(dk, dv);
            ++count;
        }
        if (end == query.size()) break;
        start = end + 1;
    }
}

static std::string extract_path(const std::string& url) {
    if (url.size() > 2048) return std::string();
    size_t start = 0;
    size_t scheme = url.find("://");
    if (scheme != std::string::npos) {
        size_t slash = url.find('/', scheme + 3);
        start = (slash == std::string::npos) ? url.size() : slash;
    } else {
        start = 0;
    }
    size_t q = url.find('?', start);
    size_t h = url.find('#', start);
    size_t end = std::min(q == std::string::npos ? url.size() : q, h == std::string::npos ? url.size() : h);
    std::string path = url.substr(start, end - start);
    if (path.empty()) path = "/";
    if (path.size() > 256) return std::string();
    return path;
}

static std::string extract_query(const std::string& url) {
    size_t q = url.find('?');
    if (q == std::string::npos) return std::string();
    size_t h = url.find('#', q + 1);
    if (h == std::string::npos) return url.substr(q + 1);
    return url.substr(q + 1, h - (q + 1));
}

// The requested function
static std::string check_mod(const std::string& url, const std::unordered_map<std::string, UserRecord>& store) {
    std::string path = extract_path(url);
    if (path != "/check_mod") return "false";
    std::unordered_map<std::string, std::string> params;
    parse_query(extract_query(url), params, 10, 256);
    if (params.empty()) return "false";
    auto itU = params.find("username");
    auto itP = params.find("password");
    if (itU == params.end() || itP == params.end()) return "false";
    const std::string& username = itU->second;
    const std::string& password = itP->second;
    if (!is_username_valid(username)) return "false";
    if (!is_password_valid_policy(password)) return "false";
    auto itRec = store.find(username);
    if (itRec == store.end()) return "false";
    return verify_password(password, itRec->second) ? "true" : "false";
}

int main() {
    std::unordered_map<std::string, UserRecord> store;
    store.emplace("alice", create_user_record("Str0ng-P@ssw0rd!", 210000));

    std::string t1 = "http://localhost/check_mod?username=alice&password=Str0ng-P%40ssw0rd%21"; // true
    std::string t2 = "/check_mod?username=alice&password=Wrong-P%40ssw0rd%21"; // false
    std::string t3 = "/other?username=alice&password=Str0ng-P%40ssw0rd%21"; // false
    std::string t4 = "/check_mod?username=alice"; // false
    std::string t5 = "/check_mod?username=alice&password=short1!A"; // false

    std::cout << check_mod(t1, store) << "\n";
    std::cout << check_mod(t2, store) << "\n";
    std::cout << check_mod(t3, store) << "\n";
    std::cout << check_mod(t4, store) << "\n";
    std::cout << check_mod(t5, store) << "\n";
    return 0;
}