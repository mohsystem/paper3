#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <limits>

static const size_t MAX_VALUE_LEN = 256;

struct Account {
    const char* username;
    const char* password;
    const char* session;
};

static const std::vector<Account> ACCOUNTS = {
    {"mod_alice", "Str0ngP@ss!", "sess-AL1CE-123"},
    {"mod_bob",   "B0b$Secure",  "sess-BOB-456"}
};

static bool ct_equals(const std::string& a, const std::string& b) {
    size_t la = a.size(), lb = b.size();
    size_t max = la > lb ? la : lb;
    unsigned char diff = static_cast<unsigned char>(la ^ lb);
    for (size_t i = 0; i < max; ++i) {
        unsigned char ac = i < la ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char bc = i < lb ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned char>(ac ^ bc);
    }
    return diff == 0;
}

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static bool url_decode(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            if (out.size() >= MAX_VALUE_LEN) return false;
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size()) {
            int hi = hexVal(in[i+1]);
            int lo = hexVal(in[i+2]);
            if (hi < 0 || lo < 0) return false;
            char decoded = static_cast<char>((hi << 4) | lo);
            if (out.size() >= MAX_VALUE_LEN) return false;
            out.push_back(decoded);
            i += 2;
        } else {
            if (out.size() >= MAX_VALUE_LEN) return false;
            out.push_back(c);
        }
    }
    return true;
}

static bool parse_query(const std::string& url, std::string& username, std::string& password, std::string& session) {
    username.clear(); password.clear(); session.clear();
    size_t qpos = url.find('?');
    if (qpos == std::string::npos) return false;
    std::string path = url.substr(0, qpos);
    if (path != "/check_mod") return false;
    std::string qs = url.substr(qpos + 1);
    size_t start = 0;
    while (start <= qs.size()) {
        size_t amp = qs.find('&', start);
        std::string pair = qs.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            std::string key = eq == std::string::npos ? pair : pair.substr(0, eq);
            std::string val = eq == std::string::npos ? "" : pair.substr(eq + 1);
            std::string dkey, dval;
            if (!url_decode(key, dkey)) { /* skip invalid key */ }
            else if (!url_decode(val, dval)) { /* skip invalid value */ }
            else {
                if (dkey == "username") username = dval;
                else if (dkey == "password") password = dval;
                else if (dkey == "session") session = dval;
            }
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    if (username.empty() || password.empty() || session.empty()) return false;
    if (username.size() > MAX_VALUE_LEN || password.size() > MAX_VALUE_LEN || session.size() > MAX_VALUE_LEN) return false;
    return true;
}

bool auth_mod(const std::string& username, const std::string& password, const std::string& session) {
    int match = 0;
    for (const auto& acc : ACCOUNTS) {
        int ueq = ct_equals(username, acc.username) ? 1 : 0;
        int peq = ct_equals(password, acc.password) ? 1 : 0;
        int seq = ct_equals(session, acc.session) ? 1 : 0;
        match |= (ueq & peq & seq);
    }
    return match == 1;
}

bool check_mod(const std::string& url) {
    std::string u, p, s;
    if (!parse_query(url, u, p, s)) return false;
    return auth_mod(u, p, s);
}

int main() {
    std::vector<std::string> tests = {
        "/check_mod?username=mod_alice&password=Str0ngP%40ss%21&session=sess-AL1CE-123",
        "/check_mod?username=mod_alice&password=wrong&session=sess-AL1CE-123",
        "/check_mod?username=mod_bob&password=B0b%24Secure&session=bad-session",
        "/check_mod?username=joe&password=anything&session=whatever",
        "/check_mod?username=mod_alice&password=Str0ngP%40ss%21"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        bool res = check_mod(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << (res ? "true" : "false") << "\n";
    }
    return 0;
}