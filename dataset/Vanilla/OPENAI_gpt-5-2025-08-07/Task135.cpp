#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>

static std::unordered_map<std::string, std::string> TOKENS;

static std::string base64UrlEncode(const std::vector<unsigned char>& data) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t i = 0;
    unsigned int val = 0;
    int valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    // Convert to URL-safe and strip padding
    for (char& ch : out) {
        if (ch == '+') ch = '-';
        else if (ch == '/') ch = '_';
    }
    while (!out.empty() && out.back() == '=') out.pop_back();
    return out;
}

static std::string randomToken(size_t numBytes) {
    std::random_device rd;
    std::vector<unsigned char> buf(numBytes);
    for (size_t i = 0; i < numBytes; ++i) {
        buf[i] = static_cast<unsigned char>(rd());
    }
    return base64UrlEncode(buf);
}

static bool constantTimeEquals(const std::string& a, const std::string& b) {
    size_t lenA = a.size();
    size_t lenB = b.size();
    size_t len = (lenA < lenB) ? lenA : lenB;
    unsigned char diff = static_cast<unsigned char>(lenA ^ lenB);
    for (size_t i = 0; i < len; ++i) {
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    }
    return diff == 0;
}

std::string issueToken(const std::string& sessionId) {
    std::string token = randomToken(32);
    TOKENS[sessionId] = token;
    return token;
}

bool validateToken(const std::string& sessionId, const std::string& token) {
    auto it = TOKENS.find(sessionId);
    if (it == TOKENS.end() || token.empty()) return false;
    return constantTimeEquals(it->second, token);
}

std::string rotateToken(const std::string& sessionId) {
    return issueToken(sessionId);
}

void invalidateSession(const std::string& sessionId) {
    TOKENS.erase(sessionId);
}

int main() {
    // 5 test cases
    std::string s1 = "sessionA";
    std::string s2 = "sessionB";

    std::string t1 = issueToken(s1);
    std::cout << "Test1 (valid token): " << (validateToken(s1, t1) ? "true" : "false") << "\n";

    std::cout << "Test2 (invalid token): " << (validateToken(s1, "invalidtoken") ? "true" : "false") << "\n";

    std::cout << "Test3 (missing session): " << (validateToken("noSession", t1) ? "true" : "false") << "\n";

    std::string t2 = issueToken(s2);
    std::cout << "Test4 (cross-session misuse): " << (validateToken(s2, t1) ? "true" : "false") << "\n";

    std::string old = t1;
    std::string rotated = rotateToken(s1);
    bool oldValid = validateToken(s1, old);
    bool newValid = validateToken(s1, rotated);
    std::cout << "Test5 (rotate old invalid/new valid): " << ((!oldValid && newValid) ? "true" : "false") << "\n";

    return 0;
}