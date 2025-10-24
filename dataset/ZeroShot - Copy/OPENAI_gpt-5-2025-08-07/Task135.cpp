#include <bits/stdc++.h>
using namespace std;

class CsrfTokenService {
public:
    explicit CsrfTokenService(chrono::milliseconds ttl) : ttl_(ttl) {
        if (ttl_.count() <= 0) throw invalid_argument("ttl must be positive");
    }

    string issueToken(const string& sessionId) {
        if (sessionId.empty()) throw invalid_argument("sessionId required");
        string token = generateToken(32);
        auto expiry = chrono::steady_clock::now() + ttl_;
        store_[sessionId] = {token, expiry};
        return token;
    }

    bool validateToken(const string& sessionId, const string& providedToken) {
        auto it = store_.find(sessionId);
        if (it == store_.end() || providedToken.empty()) return false;
        auto now = chrono::steady_clock::now();
        if (now > it->second.expiry) {
            store_.erase(it);
            return false;
        }
        bool ok = constantTimeEquals(it->second.token, providedToken);
        if (ok) {
            // rotate
            it->second.token = generateToken(32);
            it->second.expiry = now + ttl_;
        }
        return ok;
    }

    size_t purgeExpired() {
        auto now = chrono::steady_clock::now();
        vector<string> toDel;
        for (auto& kv : store_) {
            if (now > kv.second.expiry) toDel.push_back(kv.first);
        }
        for (auto& k : toDel) store_.erase(k);
        return toDel.size();
    }

private:
    struct TokenRecord {
        string token;
        chrono::steady_clock::time_point expiry;
    };
    unordered_map<string, TokenRecord> store_;
    chrono::milliseconds ttl_;

    static bool constantTimeEquals(const string& a, const string& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            diff |= static_cast<unsigned char>(a[i] ^ b[i]);
        }
        return diff == 0;
    }

    static string generateToken(size_t numBytes) {
        vector<unsigned char> buf(numBytes);
        std::random_device rd;
        for (size_t i = 0; i < numBytes; ++i) {
            buf[i] = static_cast<unsigned char>(rd());
        }
        // hex encode
        static const char* hex = "0123456789abcdef";
        string out;
        out.reserve(numBytes * 2);
        for (unsigned char c : buf) {
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
        return out;
    }
};

int main() {
    CsrfTokenService service(std::chrono::milliseconds(200));

    // Test 1: Valid token
    string s1 = "sessA";
    string tok1 = service.issueToken(s1);
    bool r1 = service.validateToken(s1, tok1);

    // Test 2: Invalid token
    bool r2 = service.validateToken(s1, string("invalid"));

    // Test 3: Expired token
    CsrfTokenService shortT(std::chrono::milliseconds(10));
    string s2 = "sessB";
    string tok2 = shortT.issueToken(s2);
    this_thread::sleep_for(std::chrono::milliseconds(30));
    bool r3 = shortT.validateToken(s2, tok2);

    // Test 4: Wrong session
    string s3 = "sessC";
    string tok3 = service.issueToken(s3);
    bool r4 = service.validateToken(string("other"), tok3);

    // Test 5: Replay failure after rotation
    string s4 = "sessD";
    string tok4 = service.issueToken(s4);
    bool first = service.validateToken(s4, tok4);
    bool second = service.validateToken(s4, tok4);
    bool r5 = first && !second;

    cout << boolalpha;
    cout << r1 << "\n";
    cout << r2 << "\n";
    cout << r3 << "\n";
    cout << r4 << "\n";
    cout << r5 << "\n";
    return 0;
}