#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <cctype>

class CsrfTokenStore {
public:
    explicit CsrfTokenStore(long ttlSeconds = 600) : ttl(ttlSeconds) {
        if (ttlSeconds <= 0 || ttlSeconds > 86400) {
            throw std::invalid_argument("Invalid TTL");
        }
    }

    std::string generateToken(const std::string& sessionId) {
        if (!validSessionId(sessionId)) {
            throw std::invalid_argument("Invalid sessionId");
        }
        std::vector<unsigned char> bytes(32);
        secureRandom(bytes);
        std::string token = base64UrlEncode(bytes);
        long long exp = nowSeconds() + ttl;
        store[sessionId][token] = exp;
        purgeExpired();
        return token;
    }

    bool validateToken(const std::string& sessionId, const std::string& token) {
        if (!validSessionId(sessionId)) return false;
        if (token.size() < 16 || token.size() > 256) return false;
        auto sit = store.find(sessionId);
        if (sit == store.end()) {
            purgeExpired();
            return false;
        }
        auto& tokens = sit->second;
        auto tit = tokens.find(token);
        if (tit == tokens.end()) {
            purgeExpired();
            return false;
        }
        long long exp = tit->second;
        long long now = nowSeconds();
        if (exp <= now) {
            tokens.erase(tit);
            if (tokens.empty()) {
                store.erase(sit);
            }
            purgeExpired();
            return false;
        }
        // One-time use
        tokens.erase(tit);
        if (tokens.empty()) {
            store.erase(sit);
        }
        purgeExpired();
        return true;
    }

    bool expireTokenForTest(const std::string& sessionId, const std::string& token) {
        auto sit = store.find(sessionId);
        if (sit == store.end()) return false;
        auto& tokens = sit->second;
        auto tit = tokens.find(token);
        if (tit == tokens.end()) return false;
        tit->second = nowSeconds() - 1;
        return true;
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, long long>> store;
    long ttl;

    static bool validSessionId(const std::string& s) {
        if (s.empty() || s.size() > 64) return false;
        for (char c : s) {
            if (!(std::isalpha(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '_')) {
                return false;
            }
        }
        return true;
    }

    static long long nowSeconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    void purgeExpired() {
        long long now = nowSeconds();
        std::vector<std::string> emptySessions;
        for (auto& pair : store) {
            auto& tokens = pair.second;
            std::vector<std::string> toErase;
            for (auto& tp : tokens) {
                if (tp.second <= now) {
                    toErase.push_back(tp.first);
                }
            }
            for (const auto& k : toErase) {
                tokens.erase(k);
            }
            if (tokens.empty()) {
                emptySessions.push_back(pair.first);
            }
        }
        for (const auto& s : emptySessions) {
            store.erase(s);
        }
    }

    static void secureRandom(std::vector<unsigned char>& out) {
        std::random_device rd;
        size_t i = 0;
        while (i < out.size()) {
            unsigned int r = rd();
            for (int b = 0; b < 4 && i < out.size(); ++b) {
                out[i++] = static_cast<unsigned char>((r >> (b * 8)) & 0xFFu);
            }
        }
    }

    static std::string base64UrlEncode(const std::vector<unsigned char>& data) {
        static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        size_t len = data.size();
        size_t outLen = (len / 3) * 4 + ((len % 3) == 0 ? 0 : ((len % 3) == 1 ? 2 : 3));
        std::string out;
        out.reserve(outLen);
        size_t i = 0;
        while (i + 3 <= len) {
            uint32_t triple = (static_cast<uint32_t>(data[i]) << 16) |
                              (static_cast<uint32_t>(data[i + 1]) << 8) |
                              (static_cast<uint32_t>(data[i + 2]));
            out.push_back(table[(triple >> 18) & 0x3F]);
            out.push_back(table[(triple >> 12) & 0x3F]);
            out.push_back(table[(triple >> 6) & 0x3F]);
            out.push_back(table[triple & 0x3F]);
            i += 3;
        }
        size_t rem = len - i;
        if (rem == 1) {
            uint32_t triple = (static_cast<uint32_t>(data[i]) << 16);
            out.push_back(table[(triple >> 18) & 0x3F]);
            out.push_back(table[(triple >> 12) & 0x3F]);
        } else if (rem == 2) {
            uint32_t triple = (static_cast<uint32_t>(data[i]) << 16) |
                              (static_cast<uint32_t>(data[i + 1]) << 8);
            out.push_back(table[(triple >> 18) & 0x3F]);
            out.push_back(table[(triple >> 12) & 0x3F]);
            out.push_back(table[(triple >> 6) & 0x3F]);
        }
        return out;
    }
};

int main() {
    CsrfTokenStore store(600);

    // Test 1: Valid token
    std::string s1 = "sessA";
    std::string tok1 = store.generateToken(s1);
    bool t1 = store.validateToken(s1, tok1);
    std::cout << (t1 ? "true" : "false") << "\n";

    // Test 2: Wrong session
    std::string s2 = "sessB";
    bool t2 = store.validateToken(s2, tok1);
    std::cout << (t2 ? "true" : "false") << "\n";

    // Test 3: Tampered token
    std::string tok3 = store.generateToken(s1);
    std::string tampered = tok3;
    tampered[0] = (tampered[0] == 'A') ? 'B' : 'A';
    bool t3 = store.validateToken(s1, tampered);
    std::cout << (t3 ? "true" : "false") << "\n";

    // Test 4: Expired token
    std::string tok4 = store.generateToken("sessX");
    store.expireTokenForTest("sessX", tok4);
    bool t4 = store.validateToken("sessX", tok4);
    std::cout << (t4 ? "true" : "false") << "\n";

    // Test 5: Replay detection (second use fails)
    std::string tok5 = store.generateToken("sessY");
    bool first = store.validateToken("sessY", tok5);
    bool second = store.validateToken("sessY", tok5);
    bool t5 = first && !second;
    std::cout << (t5 ? "true" : "false") << "\n";

    return 0;
}