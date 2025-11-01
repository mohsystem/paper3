#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <vector>
#include <limits>

static const size_t MAX_PAYLOAD_BYTES = 8192;
static const size_t MAX_NAME_CHARS = 256;

static bool isEscaped(const std::string& s, size_t pos) {
    size_t backslashes = 0;
    if (pos == 0) return false;
    for (size_t i = pos; i-- > 0 && s[i] == '\\'; ) {
        backslashes++;
    }
    return (backslashes % 2) == 1;
}

static std::string jsonError(const std::string& msg) {
    std::ostringstream oss;
    // minimal JSON escape
    oss << "{\"error\":\"";
    for (char c : msg) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned int)(unsigned char)c);
                    oss << buf;
                } else {
                    oss << c;
                }
        }
    }
    oss << "\"}";
    return oss.str();
}

static std::string escapeJson(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned int)c);
                    oss << buf;
                } else {
                    oss << (char)c;
                }
        }
    }
    return oss.str();
}

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string urlDecode(const std::string& s, bool& ok) {
    ok = true;
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%') {
            if (i + 2 >= s.size()) { ok = false; return ""; }
            int hi = hexVal(s[i+1]);
            int lo = hexVal(s[i+2]);
            if (hi < 0 || lo < 0) { ok = false; return ""; }
            out.push_back((char)((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(c);
        }
        if (out.size() > MAX_NAME_CHARS * 4) { ok = false; return ""; }
    }
    return out;
}

static std::string extractFromFormUrlEncoded(const std::string& body) {
    size_t i = 0;
    while (i <= body.size()) {
        size_t amp = body.find('&', i);
        size_t end = (amp == std::string::npos) ? body.size() : amp;
        if (end > i) {
            std::string pair = body.substr(i, end - i);
            size_t eq = pair.find('=');
            std::string key = (eq == std::string::npos) ? pair : pair.substr(0, eq);
            std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
            bool ok1=true, ok2=true;
            key = urlDecode(key, ok1);
            if (!ok1) return "";
            if (key == "name") {
                val = urlDecode(val, ok2);
                if (!ok2) return "";
                return val;
            }
        }
        if (amp == std::string::npos) break;
        i = amp + 1;
    }
    return "";
}

static bool isValidName(const std::string& name) {
    if (name.empty() || name.size() > MAX_NAME_CHARS) return false;
    for (unsigned char c : name) {
        if (c < 0x20) return false;
    }
    return true;
}

static std::string extractFromJson(const std::string& s) {
    const std::string key = "name";
    // Find unescaped "name"
    size_t pos = 0;
    while (true) {
        size_t q = s.find('"', pos);
        if (q == std::string::npos) return "";
        if (!isEscaped(s, q)) {
            // check literal
            size_t after = q + 1 + key.size();
            if (after < s.size() && s.compare(q + 1, key.size(), key) == 0) {
                size_t q2 = after;
                if (q2 < s.size() && s[q2] == '"' && !isEscaped(s, q2)) {
                    // Found "name"
                    size_t i = q2 + 1;
                    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
                    if (i >= s.size() || s[i] != ':') return "";
                    i++;
                    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
                    if (i >= s.size() || s[i] != '"') return "";
                    i++;
                    std::string val;
                    val.reserve(32);
                    while (i < s.size()) {
                        char c = s[i++];
                        if (c == '\\') {
                            if (i >= s.size()) return "";
                            char e = s[i++];
                            switch (e) {
                                case '"': val.push_back('"'); break;
                                case '\\': val.push_back('\\'); break;
                                case '/': val.push_back('/'); break;
                                case 'b': val.push_back('\b'); break;
                                case 'f': val.push_back('\f'); break;
                                case 'n': val.push_back('\n'); break;
                                case 'r': val.push_back('\r'); break;
                                case 't': val.push_back('\t'); break;
                                case 'u': {
                                    if (i + 4 > s.size()) return "";
                                    int code = 0;
                                    for (int k = 0; k < 4; k++) {
                                        int hv = hexVal(s[i + k]);
                                        if (hv < 0) { code = -1; break; }
                                        code = (code << 4) | hv;
                                    }
                                    i += 4;
                                    if (code < 0) return "";
                                    // Append basic BMP char (lossy for >127)
                                    if (code <= 0x7F) {
                                        val.push_back((char)code);
                                    } else {
                                        // simplistic UTF-8 encoding for BMP
                                        if (code <= 0x7FF) {
                                            val.push_back((char)(0xC0 | ((code >> 6) & 0x1F)));
                                            val.push_back((char)(0x80 | (code & 0x3F)));
                                        } else {
                                            val.push_back((char)(0xE0 | ((code >> 12) & 0x0F)));
                                            val.push_back((char)(0x80 | ((code >> 6) & 0x3F)));
                                            val.push_back((char)(0x80 | (code & 0x3F)));
                                        }
                                    }
                                } break;
                                default: return "";
                            }
                        } else if (c == '"') {
                            return val;
                        } else {
                            val.push_back(c);
                        }
                        if (val.size() > MAX_NAME_CHARS * 4) return "";
                    }
                    return "";
                }
            }
        }
        pos = q + 1;
    }
}

std::string processPayload(const std::string& payload, const std::string& contentType) {
    if (payload.size() > MAX_PAYLOAD_BYTES) {
        return jsonError("payload too large");
    }
    std::string ct = contentType;
    for (auto& ch : ct) ch = (char)std::tolower((unsigned char)ch);
    std::string name;

    auto ltrim = [](const std::string& s)->std::string{
        size_t i=0; while (i<s.size() && std::isspace((unsigned char)s[i])) i++; return s.substr(i);
    };
    bool tried = false;
    if (ct.find("application/json") != std::string::npos ||
        (ct.empty() && !payload.empty() && ltrim(payload)[0] == '{')) {
        name = extractFromJson(payload);
        tried = true;
    } else if (ct.find("application/x-www-form-urlencoded") != std::string::npos ||
               (ct.empty() && payload.find('=') != std::string::npos)) {
        name = extractFromFormUrlEncoded(payload);
        tried = true;
    }
    if (!tried) {
        name = extractFromJson(payload);
        if (name.empty()) {
            name = extractFromFormUrlEncoded(payload);
        }
    }
    if (name.empty()) {
        return jsonError("name field missing");
    }
    if (!isValidName(name)) {
        return jsonError("invalid name");
    }
    return std::string("{\"name\":\"") + escapeJson(name) + "\"}";
}

int main() {
    std::vector<std::pair<std::string,std::string>> tests = {
        {std::string("{\"name\":\"Alice\"}"), "application/json"},
        {std::string("{\"age\":30}"), "application/json"},
        {std::string("name=Bob%20Builder&role=dev"), "application/x-www-form-urlencoded"},
        {std::string("{\"name\":\"") + std::string(300, 'x') + "\"}", "application/json"},
        {std::string("invalid_payload"), ""}
    };
    for (size_t i = 0; i < tests.size(); i++) {
        std::cout << "Test " << (i+1) << ": " << processPayload(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}