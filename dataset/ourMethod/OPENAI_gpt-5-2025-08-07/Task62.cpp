#include <bits/stdc++.h>
using namespace std;

enum class JSONType { OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NUL, INVALID };

static const char* jsonTypeName(JSONType t) {
    switch (t) {
        case JSONType::OBJECT: return "object";
        case JSONType::ARRAY: return "array";
        case JSONType::STRING: return "string";
        case JSONType::NUMBER: return "number";
        case JSONType::BOOLEAN: return "boolean";
        case JSONType::NUL: return "null";
        default: return "invalid";
    }
}

struct ParseResult {
    bool ok;
    JSONType rootType;
    string rootTypeName;
    string structure;
    string error;
};

struct Parser {
    const string& s;
    size_t i = 0;
    size_t n = 0;
    int depth = 0;
    const int MAX_DEPTH = 256;
    const size_t MAX_INPUT = 1'000'000;
    const size_t MAX_OUTPUT = 200'000;
    const size_t MAX_KEY_PREVIEW = 64;
    const size_t MAX_ARRAY_PREVIEW = 20;
    string out;
    string err;

    Parser(const string& in) : s(in) {
        n = s.size();
    }

    void setError(const string& msg) {
        if (err.empty()) err = msg;
    }

    bool ensureOutputSpace(size_t add) {
        if (out.size() + add > MAX_OUTPUT) {
            setError("Output structure too large");
            return false;
        }
        return true;
    }

    void skipWS() {
        while (i < n) {
            char c = s[i];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') ++i;
            else break;
        }
    }

    bool match(const char* lit) {
        size_t L = strlen(lit);
        if (i + L <= n && s.compare(i, L, lit) == 0) {
            i += L;
            return true;
        }
        return false;
    }

    bool parseValue(JSONType& typeOut) {
        if (++depth > MAX_DEPTH) {
            setError("Maximum nesting depth exceeded");
            return false;
        }
        skipWS();
        if (i >= n) {
            setError("Unexpected end of input");
            --depth;
            return false;
        }
        char c = s[i];
        bool ok = false;
        if (c == '{') {
            ok = parseObject();
            typeOut = JSONType::OBJECT;
        } else if (c == '[') {
            ok = parseArray();
            typeOut = JSONType::ARRAY;
        } else if (c == '"') {
            ok = parseString();
            typeOut = JSONType::STRING;
            if (ok) ok = appendToken("string");
        } else if (c == '-' || (c >= '0' && c <= '9')) {
            ok = parseNumber();
            typeOut = JSONType::NUMBER;
            if (ok) ok = appendToken("number");
        } else if (match("true")) {
            typeOut = JSONType::BOOLEAN;
            ok = appendToken("boolean");
        } else if (match("false")) {
            typeOut = JSONType::BOOLEAN;
            ok = appendToken("boolean");
        } else if (match("null")) {
            typeOut = JSONType::NUL;
            ok = appendToken("null");
        } else {
            setError("Invalid value");
            ok = false;
        }
        --depth;
        return ok;
    }

    bool appendToken(const string& t) {
        if (!ensureOutputSpace(t.size())) return false;
        out += t;
        return true;
    }

    bool appendChar(char c) {
        if (!ensureOutputSpace(1)) return false;
        out.push_back(c);
        return true;
    }

    bool appendKeyPreview(const string& key) {
        string trimmed = key;
        if (trimmed.size() > MAX_KEY_PREVIEW) {
            trimmed.resize(MAX_KEY_PREVIEW);
            trimmed += "...";
        }
        // keys shown in quotes for clarity
        if (!appendChar('"')) return false;
        if (!ensureOutputSpace(trimmed.size())) return false;
        out += trimmed;
        if (!appendChar('"')) return false;
        return true;
    }

    bool parseStringContent(string* storeOpt) {
        // assumes s[i] == '"'
        if (i >= n || s[i] != '"') return false;
        ++i;
        size_t start = i;
        string tmp;
        while (i < n) {
            char c = s[i++];
            if (c == '"') {
                if (storeOpt) {
                    // We need the decoded form. For preview, we can store raw segments and simple escapes.
                    if (!tmp.empty()) storeOpt->append(tmp);
                }
                return true;
            }
            if (c == '\\') {
                if (i >= n) { setError("Invalid escape"); return false; }
                char e = s[i++];
                if (storeOpt) {
                    switch (e) {
                        case '"': tmp.push_back('"'); break;
                        case '\\': tmp.push_back('\\'); break;
                        case '/': tmp.push_back('/'); break;
                        case 'b': tmp.push_back('\b'); break;
                        case 'f': tmp.push_back('\f'); break;
                        case 'n': tmp.push_back('\n'); break;
                        case 'r': tmp.push_back('\r'); break;
                        case 't': tmp.push_back('\t'); break;
                        case 'u': {
                            // read 4 hex digits
                            if (i + 4 > n) { setError("Invalid unicode escape"); return false; }
                            for (int k = 0; k < 4; ++k) {
                                char h = s[i + k];
                                bool hex = (h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') || (h >= 'A' && h <= 'F');
                                if (!hex) { setError("Invalid unicode escape"); return false; }
                            }
                            // For preview, we won't decode unicode; we can append placeholder
                            tmp += "\\u";
                            tmp.append(s, i, 4);
                            i += 4;
                            break;
                        }
                        default:
                            setError("Invalid escape");
                            return false;
                    }
                } else {
                    if (e == 'u') {
                        if (i + 4 > n) { setError("Invalid unicode escape"); return false; }
                        for (int k = 0; k < 4; ++k) {
                            char h = s[i + k];
                            bool hex = (h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') || (h >= 'A' && h <= 'F');
                            if (!hex) { setError("Invalid unicode escape"); return false; }
                        }
                        i += 4;
                    }
                }
            } else {
                // regular char
                if (static_cast<unsigned char>(c) <= 0x1F) { setError("Unescaped control char in string"); return false; }
                if (storeOpt) tmp.push_back(c);
            }
            // Bound preview
            if (storeOpt && storeOpt->size() + tmp.size() > MAX_KEY_PREVIEW + 10) {
                // allow some slack, actual trimming later
                // do nothing; we still must consume entire string
            }
        }
        setError("Unterminated string");
        return false;
    }

    bool parseString() {
        return parseStringContent(nullptr);
    }

    bool parseNumber() {
        size_t start = i;
        if (i < n && s[i] == '-') ++i;
        if (i >= n) { setError("Invalid number"); return false; }
        if (s[i] == '0') {
            ++i;
        } else if (s[i] >= '1' && s[i] <= '9') {
            while (i < n && isdigit(static_cast<unsigned char>(s[i]))) ++i;
        } else {
            setError("Invalid number");
            return false;
        }
        if (i < n && s[i] == '.') {
            ++i;
            if (i >= n || !isdigit(static_cast<unsigned char>(s[i]))) { setError("Invalid fraction"); return false; }
            while (i < n && isdigit(static_cast<unsigned char>(s[i]))) ++i;
        }
        if (i < n && (s[i] == 'e' || s[i] == 'E')) {
            ++i;
            if (i < n && (s[i] == '+' || s[i] == '-')) ++i;
            if (i >= n || !isdigit(static_cast<unsigned char>(s[i]))) { setError("Invalid exponent"); return false; }
            while (i < n && isdigit(static_cast<unsigned char>(s[i]))) ++i;
        }
        (void)start;
        return true;
    }

    bool parseObject() {
        if (s[i] != '{') { setError("Expected {"); return false; }
        ++i;
        skipWS();
        if (!appendChar('{')) return false;
        bool first = true;
        if (i < n && s[i] == '}') {
            ++i;
            if (!appendChar('}')) return false;
            return true;
        }
        while (i < n) {
            skipWS();
            if (i >= n || s[i] != '"') { setError("Expected object key string"); return false; }
            string key;
            if (!parseStringContent(&key)) return false;
            skipWS();
            if (i >= n || s[i] != ':') { setError("Expected ':' after key"); return false; }
            ++i;
            if (!first) { if (!appendChar(',')) return false; }
            first = false;
            // write key preview and ':'
            if (!appendKeyPreview(key)) return false;
            if (!appendChar(':')) return false;

            skipWS();
            JSONType innerType = JSONType::INVALID;
            if (!parseValue(innerType)) return false;
            // For composite types, parseValue appended the inner structure (if object/array)
            // For primitives, appendToken already added.
            skipWS();
            if (i < n && s[i] == ',') {
                ++i;
                skipWS();
                continue;
            } else if (i < n && s[i] == '}') {
                ++i;
                if (!appendChar('}')) return false;
                return true;
            } else {
                setError("Expected ',' or '}' in object");
                return false;
            }
        }
        setError("Unterminated object");
        return false;
    }

    bool parseArray() {
        if (s[i] != '[') { setError("Expected ["); return false; }
        ++i;
        skipWS();
        if (!appendChar('[')) return false;
        bool first = true;
        size_t count = 0;
        if (i < n && s[i] == ']') {
            ++i;
            if (!appendChar(']')) return false;
            return true;
        }
        while (i < n) {
            if (!first) { if (!appendChar(',')) return false; }
            first = false;
            skipWS();
            JSONType innerType = JSONType::INVALID;
            size_t previewIndex = count;
            bool withinPreview = (previewIndex < MAX_ARRAY_PREVIEW);
            if (withinPreview) {
                if (!parseValue(innerType)) return false;
            } else {
                // parse value silently without appending
                string savedOut = std::move(out);
                out.clear();
                if (!parseValue(innerType)) return false;
                out = std::move(savedOut);
            }
            ++count;
            skipWS();
            if (i < n && s[i] == ',') {
                ++i;
                skipWS();
                continue;
            } else if (i < n && s[i] == ']') {
                ++i;
                if (count > MAX_ARRAY_PREVIEW) {
                    if (!appendToken("...")) return false;
                }
                if (!appendChar(']')) return false;
                return true;
            } else {
                setError("Expected ',' or ']' in array");
                return false;
            }
        }
        setError("Unterminated array");
        return false;
    }
};

ParseResult parseJsonStructure(const string& input) {
    ParseResult pr{};
    pr.ok = false;
    pr.rootType = JSONType::INVALID;
    pr.rootTypeName = "invalid";
    pr.structure = "";
    pr.error = "";

    if (input.size() > 1'000'000) {
        pr.error = "Input too large";
        return pr;
    }
    Parser p(input);
    p.skipWS();
    if (p.i >= p.n) {
        pr.error = "Empty input";
        return pr;
    }
    JSONType root = JSONType::INVALID;
    bool ok = p.parseValue(root);
    if (!ok) {
        pr.error = p.err.empty() ? "Parse error" : p.err;
        return pr;
    }
    p.skipWS();
    if (p.i != p.n) {
        pr.error = "Trailing characters after JSON value";
        return pr;
    }
    pr.ok = true;
    pr.rootType = root;
    pr.rootTypeName = jsonTypeName(root);
    pr.structure = p.out;
    return pr;
}

static void run_test(const string& json) {
    ParseResult r = parseJsonStructure(json);
    cout << "Input: " << json << "\n";
    if (r.ok) {
        cout << "OK Root=" << r.rootTypeName << "\n";
        cout << "Structure: " << r.structure << "\n";
    } else {
        cout << "ERROR: " << r.error << "\n";
    }
    cout << "----\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> tests = {
        "{\"a\":1,\"b\":[true,null,{\"c\":\"x\"}]}",
        "[1, 2, 3, 4, 5]",
        "\"hello world\"",
        "-12.34e+10",
        "{unquoted:1}"
    };
    for (const auto& t : tests) run_test(t);

    // Additional test to show null root
    run_test("null");

    return 0;
}