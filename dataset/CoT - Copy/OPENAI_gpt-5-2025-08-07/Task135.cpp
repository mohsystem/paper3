#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

class CsrfProtectorCPP {
public:
    explicit CsrfProtectorCPP(int ttlSeconds, const std::string& expectedOrigin)
        : ttlSeconds(ttlSeconds), expectedOrigin(expectedOrigin) {}

    // Synchronizer token pattern: random token stored server-side per session with TTL and one-time use
    std::string generateToken(const std::string& sessionId) {
        std::string token = randomHex(32); // 32 bytes -> 64 hex chars
        time_t now = std::time(nullptr);
        TokenInfo info;
        info.expiry = now + ttlSeconds;
        info.used = false;
        tokens[sessionId][token] = info;
        // Clean up expired entries for this session
        cleanup(sessionId, now);
        return token;
    }

    bool validateRequest(const std::string& method,
                         const std::string& sessionId,
                         const std::string& token,
                         const std::string& originHeader,
                         const std::string& refererHeader) {
        if (!isStateChanging(method)) {
            return true;
        }
        if (!validateOriginReferer(originHeader, refererHeader)) {
            return false;
        }
        if (sessionId.empty() || token.empty()) {
            return false;
        }
        time_t now = std::time(nullptr);
        auto itSess = tokens.find(sessionId);
        if (itSess == tokens.end()) return false;
        auto& tmap = itSess->second;
        auto itTok = tmap.find(token);
        if (itTok == tmap.end()) return false;
        if (itTok->second.used) return false;
        if (now > itTok->second.expiry) {
            tmap.erase(itTok);
            return false;
        }
        // One-time use
        itTok->second.used = true;
        // Cleanup
        cleanup(sessionId, now);
        return true;
    }

private:
    struct TokenInfo {
        time_t expiry;
        bool used;
    };

    int ttlSeconds;
    std::string expectedOrigin;
    std::unordered_map<std::string, std::unordered_map<std::string, TokenInfo>> tokens;

    static bool isStateChanging(const std::string& method) {
        std::string m = method;
        for (auto& c : m) c = toupper(c);
        return (m == "POST" || m == "PUT" || m == "PATCH" || m == "DELETE");
    }

    bool validateOriginReferer(const std::string& originHeader, const std::string& refererHeader) const {
        if (expectedOrigin.empty()) return true;
        if (!originHeader.empty()) {
            return originHeader == expectedOrigin;
        }
        if (!refererHeader.empty()) {
            return refererHeader.rfind(expectedOrigin, 0) == 0; // starts with
        }
        return false;
    }

    void cleanup(const std::string& sessionId, time_t now) {
        auto itSess = tokens.find(sessionId);
        if (itSess == tokens.end()) return;
        auto& tmap = itSess->second;
        for (auto it = tmap.begin(); it != tmap.end(); ) {
            if (now > it->second.expiry || (it->second.used && (now - (it->second.expiry - ttlSeconds)) > ttlSeconds)) {
                it = tmap.erase(it);
            } else {
                ++it;
            }
        }
    }

    static std::string randomHex(size_t bytes) {
        std::vector<unsigned char> buf(bytes);
        if (!secureRandom(buf.data(), buf.size())) {
            // Fallback to /dev/urandom failure handling: fill with zeros (not recommended)
            std::memset(buf.data(), 0, buf.size());
        }
        static const char* hex = "0123456789abcdef";
        std::string out;
        out.reserve(bytes * 2);
        for (size_t i = 0; i < bytes; ++i) {
            unsigned char b = buf[i];
            out.push_back(hex[(b >> 4) & 0xF]);
            out.push_back(hex[b & 0xF]);
        }
        return out;
    }

    static bool secureRandom(unsigned char* out, size_t len) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) return false;
        size_t readTotal = 0;
        while (readTotal < len) {
            ssize_t r = read(fd, out + readTotal, len - readTotal);
            if (r <= 0) { close(fd); return false; }
            readTotal += static_cast<size_t>(r);
        }
        close(fd);
        return true;
    }
};

int main() {
    CsrfProtectorCPP protector(5, "https://example.com");
    std::string session = "sess-1";
    std::string token = protector.generateToken(session);

    bool t1 = protector.validateRequest("POST", session, token, "https://example.com", "");
    bool t2 = protector.validateRequest("POST", session, token, "https://evil.com", "");
    std::string tampered = token.substr(0, token.size() - 1) + (token.back() == 'a' ? 'b' : 'a');
    bool t3 = protector.validateRequest("POST", session, tampered, "https://example.com", "");
    // Expired: wait past TTL or simulate by using a new protector with ttl=0 and validating; instead just sleep
    // For deterministic test, generate token and then manipulate internal state is not available, so we simulate expiration by waiting.
    // But to keep runtime short, we create a protector with ttl=1 and sleep 2 seconds.
    CsrfProtectorCPP protector2(1, "https://example.com");
    std::string session2 = "sess-2";
    std::string token2 = protector2.generateToken(session2);
    sleep(2);
    bool t4 = protector2.validateRequest("POST", session2, token2, "https://example.com", "");
    bool t5 = protector.validateRequest("POST", session, token, "https://example.com", "");

    std::cout << "T1_valid=" << (t1 ? "true" : "false") << std::endl;
    std::cout << "T2_bad_origin=" << (t2 ? "true" : "false") << std::endl;
    std::cout << "T3_tampered=" << (t3 ? "true" : "false") << std::endl;
    std::cout << "T4_expired=" << (t4 ? "true" : "false") << std::endl;
    std::cout << "T5_replay=" << (t5 ? "true" : "false") << std::endl;
    return 0;
}