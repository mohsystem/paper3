#include <iostream>
#include <string>
#include <cctype>

static const size_t MAX_BODY_SIZE = 10 * 1024;

static size_t skipWs(const std::string& s, size_t i) {
    while (i < s.size()) {
        char c = s[i];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') i++;
        else break;
    }
    return i;
}

static bool parseJsonString(const std::string& s, size_t startIdx, std::string& out, size_t& nextIdx) {
    if (startIdx >= s.size() || s[startIdx] != '"') return false;
    out.clear();
    size_t i = startIdx + 1;
    while (i < s.size()) {
        char c = s[i];
        if (c == '"') {
            nextIdx = i + 1;
            return true;
        } else if (c == '\\') {
            if (i + 1 >= s.size()) return false;
            char esc = s[i + 1];
            switch (esc) {
                case '"': out.push_back('"'); i += 2; break;
                case '\\': out.push_back('\\'); i += 2; break;
                case '/': out.push_back('/'); i += 2; break;
                case 'b': out.push_back('\b'); i += 2; break;
                case 'f': out.push_back('\f'); i += 2; break;
                case 'n': out.push_back('\n'); i += 2; break;
                case 'r': out.push_back('\r'); i += 2; break;
                case 't': out.push_back('\t'); i += 2; break;
                case 'u': {
                    if (i + 6 > s.size()) return false;
                    unsigned int code = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = s[i + 2 + k];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code += (h - '0');
                        else if (h >= 'a' && h <= 'f') code += 10 + (h - 'a');
                        else if (h >= 'A' && h <= 'F') code += 10 + (h - 'A');
                        else return false;
                    }
                    // encode to UTF-8
                    if (code <= 0x7F) out.push_back(static_cast<char>(code));
                    else if (code <= 0x7FF) {
                        out.push_back(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
                        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                    } else {
                        out.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F)));
                        out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                    }
                    i += 6;
                    break;
                }
                default:
                    return false;
            }
        } else {
            out.push_back(c);
            i++;
        }
    }
    return false;
}

static bool skipJsonValue(const std::string& s, size_t i, size_t& nextIdx) {
    i = skipWs(s, i);
    if (i >= s.size()) return false;
    char c = s[i];
    if (c == '"') {
        std::string tmp; size_t nx;
        if (!parseJsonString(s, i, tmp, nx)) return false;
        nextIdx = nx; return true;
    } else if (c == '{') {
        int depth = 1; i++;
        while (i < s.size() && depth > 0) {
            if (s[i] == '"') {
                std::string tmp; size_t nx;
                if (!parseJsonString(s, i, tmp, nx)) return false;
                i = nx;
            } else if (s[i] == '{') { depth++; i++; }
            else if (s[i] == '}') { depth--; i++; }
            else i++;
        }
        if (depth != 0) return false;
        nextIdx = i; return true;
    } else if (c == '[') {
        int depth = 1; i++;
        while (i < s.size() && depth > 0) {
            if (s[i] == '"') {
                std::string tmp; size_t nx;
                if (!parseJsonString(s, i, tmp, nx)) return false;
                i = nx;
            } else if (s[i] == '[') { depth++; i++; }
            else if (s[i] == ']') { depth--; i++; }
            else i++;
        }
        if (depth != 0) return false;
        nextIdx = i; return true;
    } else {
        size_t j = i;
        while (j < s.size()) {
            char ch = s[j];
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '+' || ch == '-' || ch == '.' || ch == 'e' || ch == 'E') j++;
            else break;
        }
        if (j == i) return false;
        nextIdx = j; return true;
    }
}

std::string handleRequest(const std::string& contentType, const std::string& body) {
    std::string ct = contentType;
    // lowercase compare safely
    for (auto& c : ct) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (ct.rfind("application/json", 0) != 0) return "";
    if (body.empty() || body.size() > MAX_BODY_SIZE) return "";

    const std::string& s = body;
    size_t i = 0, n = s.size();
    while (i < n) {
        i = skipWs(s, i);
        if (i >= n) break;
        if (s[i] == '"') {
            std::string key; size_t nx;
            if (!parseJsonString(s, i, key, nx)) return "";
            i = skipWs(s, nx);
            if (i < n && s[i] == ':') {
                i++;
                i = skipWs(s, i);
                if (key == "name") {
                    if (i < n && s[i] == '"') {
                        std::string val; size_t nxv;
                        if (!parseJsonString(s, i, val, nxv)) return "";
                        return val;
                    } else {
                        // Not a string value; ignore
                    }
                }
                size_t skipped;
                if (!skipJsonValue(s, i, skipped)) return "";
                i = skipped;
            }
        } else {
            i++;
        }
    }
    return "";
}

int main() {
    std::string ct = "application/json";
    std::string tests[] = {
        "{\"name\":\"Alice\"}",
        "{\"id\":123,\"name\":\"Bob \\\\ \\\"The Builder\\\"\",\"extra\":true}",
        "{\"id\":7,\"names\":[\"x\",\"y\"]}",
        "{malformed json",
        "{\"nested\":{\"name\":\"Carol\"},\"other\":1}"
    };
    for (int t = 0; t < 5; ++t) {
        std::string res = handleRequest(ct, tests[t]);
        std::cout << "Test " << (t+1) << " => " << res << "\n";
    }
    return 0;
}