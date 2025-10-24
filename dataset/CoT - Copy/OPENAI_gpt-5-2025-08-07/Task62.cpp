#include <bits/stdc++.h>
using namespace std;

/*
Chain-of-Through Secure Implementation
1) Understand: Parse JSON to structure, identify root element.
2) Security: No external libraries, careful index checks, robust string/number parsing.
3) Implement: Defensive recursive-descent parser, limit recursion depth.
4) Review: Ensure bounds checks and correct token expectations.
5) Output: "Root=<type>; Structure=<structure>" or invalid.
*/

class JsonParser {
public:
    explicit JsonParser(const string& input) : s(input), i(0), n(input.size()) {}
    string parse() {
        skipWS();
        string v;
        if (!parseValue(0, v)) return string();
        skipWS();
        if (i != n) return string();
        return v;
    }
private:
    const string s;
    size_t i;
    size_t n;
    static constexpr int MAX_DEPTH = 1000;

    void skipWS() {
        while (i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
    }

    bool parseValue(int depth, string& out) {
        if (depth > MAX_DEPTH) return false;
        skipWS();
        if (i >= n) return false;
        char c = s[i];
        if (c == '{') return parseObject(depth + 1, out);
        if (c == '[') return parseArray(depth + 1, out);
        if (c == '"') {
            string tmp;
            if (!parseString(tmp)) return false;
            out = "string";
            return true;
        }
        if (c == '-' || (c >= '0' && c <= '9')) {
            if (!parseNumber()) return false;
            out = "number";
            return true;
        }
        if (matchLiteral("true")) { out = "boolean"; return true; }
        if (matchLiteral("false")) { out = "boolean"; return true; }
        if (matchLiteral("null")) { out = "null"; return true; }
        return false;
    }

    bool matchLiteral(const char* lit) {
        size_t len = strlen(lit);
        if (i + len > n) return false;
        if (s.compare(i, len, lit) == 0) {
            i += len;
            return true;
        }
        return false;
    }

    bool parseObject(int depth, string& out) {
        if (i >= n || s[i] != '{') return false;
        i++; // consume '{'
        skipWS();
        if (i < n && s[i] == '}') {
            i++;
            out = "{}";
            return true;
        }
        vector<pair<string,string>> kv;
        while (true) {
            skipWS();
            if (i >= n || s[i] != '"') return false;
            string key;
            if (!parseString(key)) return false;
            skipWS();
            if (i >= n || s[i] != ':') return false;
            i++; // ':'
            skipWS();
            string v;
            if (!parseValue(depth, v)) return false;
            kv.emplace_back(key, v);
            skipWS();
            if (i >= n) return false;
            if (s[i] == ',') {
                i++;
                continue;
            } else if (s[i] == '}') {
                i++;
                break;
            } else {
                return false;
            }
        }
        // Build structure
        string res;
        res.push_back('{');
        for (size_t k = 0; k < kv.size(); ++k) {
            if (k > 0) res += ", ";
            res.push_back('"');
            res += escapeForOutput(kv[k].first);
            res += "\": ";
            res += kv[k].second;
        }
        res.push_back('}');
        out = res;
        return true;
    }

    bool parseArray(int depth, string& out) {
        if (i >= n || s[i] != '[') return false;
        i++; // '['
        skipWS();
        if (i < n && s[i] == ']') {
            i++;
            out = "[]";
            return true;
        }
        vector<string> elems;
        while (true) {
            string v;
            if (!parseValue(depth, v)) return false;
            elems.push_back(v);
            skipWS();
            if (i >= n) return false;
            if (s[i] == ',') {
                i++;
                skipWS();
                continue;
            } else if (s[i] == ']') {
                i++;
                break;
            } else {
                return false;
            }
        }
        // unique preserve order
        vector<string> uniq;
        unordered_set<string> seen;
        for (auto& e : elems) {
            if (seen.insert(e).second) uniq.push_back(e);
        }
        string res;
        res.push_back('[');
        for (size_t idx = 0; idx < uniq.size(); ++idx) {
            if (idx > 0) res += " | ";
            res += uniq[idx];
        }
        res.push_back(']');
        out = res;
        return true;
    }

    bool parseString(string& out) {
        if (i >= n || s[i] != '"') return false;
        i++; // opening quote
        string tmp;
        tmp.reserve(16);
        while (i < n) {
            char c = s[i++];
            if (c == '"') {
                out = tmp;
                return true;
            } else if (c == '\\') {
                if (i >= n) return false;
                char esc = s[i++];
                switch (esc) {
                    case '"': tmp.push_back('"'); break;
                    case '\\': tmp.push_back('\\'); break;
                    case '/': tmp.push_back('/'); break;
                    case 'b': tmp.push_back('\b'); break;
                    case 'f': tmp.push_back('\f'); break;
                    case 'n': tmp.push_back('\n'); break;
                    case 'r': tmp.push_back('\r'); break;
                    case 't': tmp.push_back('\t'); break;
                    case 'u': {
                        if (i + 4 > n) return false;
                        for (int k = 0; k < 4; ++k) {
                            char h = s[i + k];
                            if (!isHex(h)) return false;
                        }
                        i += 4;
                        tmp.push_back('?'); // unicode placeholder
                        break;
                    }
                    default: return false;
                }
            } else {
                if ((unsigned char)c <= 0x1F) return false;
                tmp.push_back(c);
            }
        }
        return false; // unterminated
    }

    bool parseNumber() {
        size_t start = i;
        if (i < n && s[i] == '-') i++;
        if (i >= n) return false;
        if (s[i] == '0') {
            i++;
        } else if (isdigit((unsigned char)s[i])) {
            i++;
            while (i < n && isdigit((unsigned char)s[i])) i++;
        } else {
            return false;
        }
        if (i < n && s[i] == '.') {
            i++;
            if (i >= n || !isdigit((unsigned char)s[i])) return false;
            while (i < n && isdigit((unsigned char)s[i])) i++;
        }
        if (i < n && (s[i] == 'e' || s[i] == 'E')) {
            i++;
            if (i < n && (s[i] == '+' || s[i] == '-')) i++;
            if (i >= n || !isdigit((unsigned char)s[i])) return false;
            while (i < n && isdigit((unsigned char)s[i])) i++;
        }
        return i > start;
    }

    static bool isHex(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    static string escapeForOutput(const string& in) {
        string out;
        out.reserve(in.size());
        for (char c : in) {
            if (c == '\\' || c == '"') {
                out.push_back('\\');
                out.push_back(c);
            } else if ((unsigned char)c <= 0x1F) {
                out.push_back('?');
            } else {
                out.push_back(c);
            }
        }
        return out;
    }
};

static string extract_root_and_structure(const string& json) {
    JsonParser p(json);
    string s = p.parse();
    if (s.empty()) return "Root=invalid; Structure=invalid";
    string root;
    if (!s.empty() && s[0] == '{') root = "object";
    else if (!s.empty() && s[0] == '[') root = "array";
    else if (s == "string" || s == "number" || s == "boolean" || s == "null") root = s;
    else root = "invalid";
    return "Root=" + root + "; Structure=" + s;
}

int main() {
    vector<string> tests = {
        "{\"a\":1,\"b\":{\"c\":[1,2,3]},\"d\":[{\"e\":\"x\"},{\"e\":\"y\",\"f\":true}]}",
        "[1,2,{\"a\":[null,false]}]",
        "\"hello\"",
        "123.45e-6",
        "{unclosed"
    };
    for (auto& t : tests) {
        cout << extract_root_and_structure(t) << "\n";
    }
    return 0;
}