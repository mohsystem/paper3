#include <bits/stdc++.h>
using namespace std;

class Task62 {
public:
    static string identifyRoot(const string& json) {
        if (json.size() > MAX_LEN) return "invalid";
        Parser p(json);
        Type t;
        if (!p.parseRoot(t)) return "invalid";
        return typeToString(t);
    }

private:
    enum class Type { OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NUL };
    static constexpr size_t MAX_LEN = 1000000;
    static constexpr int MAX_DEPTH = 512;

    static string typeToString(Type t) {
        switch (t) {
            case Type::OBJECT: return "object";
            case Type::ARRAY: return "array";
            case Type::STRING: return "string";
            case Type::NUMBER: return "number";
            case Type::BOOLEAN: return "boolean";
            case Type::NUL: return "null";
        }
        return "invalid";
    }

    class Parser {
    public:
        Parser(const string& in) : s(in), len(in.size()) {}

        bool parseRoot(Type& out) {
            skipWS();
            if (pos >= len) return false;
            if (!parseValue(out)) return false;
            skipWS();
            return pos == len;
        }
    private:
        const string& s;
        size_t pos = 0;
        size_t len;
        int depth = 0;

        void skipWS() {
            while (pos < len && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r' || s[pos] == '\n')) pos++;
        }

        bool parseValue(Type& out) {
            skipWS();
            if (pos >= len) return false;
            char c = s[pos];
            if (c == '{') {
                if (!parseObject()) return false;
                out = Type::OBJECT; return true;
            }
            if (c == '[') {
                if (!parseArray()) return false;
                out = Type::ARRAY; return true;
            }
            if (c == '"') {
                if (!parseString()) return false;
                out = Type::STRING; return true;
            }
            if (c == '-' || (c >= '0' && c <= '9')) {
                if (!parseNumber()) return false;
                out = Type::NUMBER; return true;
            }
            if (matchLiteral("true")) { out = Type::BOOLEAN; return true; }
            if (matchLiteral("false")) { out = Type::BOOLEAN; return true; }
            if (matchLiteral("null")) { out = Type::NUL; return true; }
            return false;
        }

        bool matchLiteral(const char* lit) {
            size_t L = strlen(lit);
            if (pos + L > len) return false;
            for (size_t i = 0; i < L; ++i) if (s[pos + i] != lit[i]) return false;
            pos += L;
            return true;
        }

        bool parseObject() {
            if (depth >= MAX_DEPTH) return false;
            if (pos >= len || s[pos] != '{') return false;
            depth++;
            pos++;
            skipWS();
            if (pos < len && s[pos] == '}') { pos++; depth--; return true; }
            while (true) {
                if (!parseString()) { depth--; return false; }
                skipWS();
                if (pos >= len || s[pos] != ':') { depth--; return false; }
                pos++;
                Type dummy;
                if (!parseValue(dummy)) { depth--; return false; }
                skipWS();
                if (pos < len && s[pos] == ',') {
                    pos++;
                    continue;
                } else if (pos < len && s[pos] == '}') {
                    pos++;
                    depth--;
                    return true;
                } else {
                    depth--;
                    return false;
                }
            }
        }

        bool parseArray() {
            if (depth >= MAX_DEPTH) return false;
            if (pos >= len || s[pos] != '[') return false;
            depth++;
            pos++;
            skipWS();
            if (pos < len && s[pos] == ']') { pos++; depth--; return true; }
            while (true) {
                Type dummy;
                if (!parseValue(dummy)) { depth--; return false; }
                skipWS();
                if (pos < len && s[pos] == ',') {
                    pos++;
                    continue;
                } else if (pos < len && s[pos] == ']') {
                    pos++;
                    depth--;
                    return true;
                } else {
                    depth--;
                    return false;
                }
            }
        }

        bool parseString() {
            if (pos >= len || s[pos] != '"') return false;
            pos++;
            while (pos < len) {
                char c = s[pos++];
                if (c == '"') return true;
                if (c == '\\') {
                    if (pos >= len) return false;
                    char e = s[pos++];
                    switch (e) {
                        case '"': case '\\': case '/': case 'b':
                        case 'f': case 'n': case 'r': case 't':
                            break;
                        case 'u':
                            for (int i = 0; i < 4; ++i) {
                                if (pos >= len) return false;
                                char h = s[pos++];
                                if (!isHex(h)) return false;
                            }
                            break;
                        default:
                            return false;
                    }
                } else {
                    if ((unsigned char)c < 0x20) return false;
                }
            }
            return false;
        }

        bool parseNumber() {
            size_t start = pos;
            if (pos < len && s[pos] == '-') pos++;
            if (pos >= len) return false;
            if (s[pos] == '0') {
                pos++;
            } else if (s[pos] >= '1' && s[pos] <= '9') {
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            } else {
                return false;
            }
            if (pos < len && s[pos] == '.') {
                pos++;
                if (pos >= len || !(s[pos] >= '0' && s[pos] <= '9')) return false;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            }
            if (pos < len && (s[pos] == 'e' || s[pos] == 'E')) {
                pos++;
                if (pos < len && (s[pos] == '+' || s[pos] == '-')) pos++;
                if (pos >= len || !(s[pos] >= '0' && s[pos] <= '9')) return false;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            }
            return pos > start;
        }

        static bool isHex(char c) {
            return (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F');
        }
    };
};

int main() {
    vector<string> tests = {
        "{\"a\":1,\"b\":[true,false,null]}",
        "[1,2,3]",
        "\"hello\"",
        "123.45e-6",
        "{unquoted: 1}"
    };
    for (const auto& t : tests) {
        cout << Task62::identifyRoot(t) << "\n";
    }
    return 0;
}