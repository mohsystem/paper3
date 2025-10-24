#include <bits/stdc++.h>
using namespace std;

/*
Chain-of-Through process:

1) Problem: Implement API-like handler that accepts user JSON and stores to a database (JSONL file).
2) Security: Limit body size, validate/sanitize inputs, strict content type, append-only writing.
3) Implementation: Minimal JSON key extraction, validators, JSON escaping, file append with fstreams.
4) Review: Fixed DB path, handle errors, no dynamic code execution, bounds checking.
5) Output: Final secure code with tests.
*/

static const size_t MAX_BODY_SIZE = 4096;
static const string DB_PATH = "users_db.jsonl";

struct Response {
    int statusCode;
    string body;
};

struct User {
    string id;
    string name;
    string email;
    int age;
    long long createdAt;
};

static string sanitize(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c != '\r' && c != '\n') out.push_back(c);
    }
    // trim
    size_t start = 0, end = out.size();
    while (start < end && isspace(static_cast<unsigned char>(out[start]))) start++;
    while (end > start && isspace(static_cast<unsigned char>(out[end-1]))) end--;
    return out.substr(start, end - start);
}

static string jsonEscape(const string& s) {
    string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else out.push_back(c);
        }
    }
    return out;
}

static bool validateName(const string& name) {
    if (name.empty() || name.size() > 100) return false;
    for (unsigned char c : name) {
        if (!(isalnum(c) || c == ' ' || c == '-' || c == '\'' || c == '_' || c == '.')) return false;
    }
    return true;
}

static bool validateEmail(const string& email) {
    if (email.size() < 3 || email.size() > 254) return false;
    size_t at = email.find('@');
    if (at == string::npos || at == 0 || at != email.rfind('@')) return false;
    string local = email.substr(0, at);
    string domain = email.substr(at + 1);
    if (local.empty() || domain.size() < 3) return false;
    if (domain.find('.') == string::npos) return false;
    auto isAllowedLocal = [](char c) {
        string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._%+-";
        return allowed.find(c) != string::npos;
    };
    for (char c : local) if (!isAllowedLocal(c)) return false;
    auto isAllowedDomain = [](char c) {
        string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-";
        return allowed.find(c) != string::npos;
    };
    for (char c : domain) if (!isAllowedDomain(c)) return false;
    if (domain.front() == '-' || domain.back() == '-' || domain.find("..") != string::npos) return false;
    size_t lastDot = domain.rfind('.');
    if (lastDot == string::npos || lastDot + 2 >= domain.size()) return false;
    string tld = domain.substr(lastDot + 1);
    if (tld.size() < 2 || tld.size() > 24) return false;
    for (char c : tld) if (!isalpha(static_cast<unsigned char>(c))) return false;
    return true;
}

static bool validateAge(int age) {
    return age >= 0 && age <= 150;
}

static string genUUID() {
    // Simple pseudo-UUID (not cryptographically strong)
    static random_device rd;
    static mt19937_64 gen(rd());
    auto rnd = [&]() { return (uint64_t)gen(); };
    uint64_t a = rnd(), b = rnd();
    char buf[37];
    snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%04x-%012llx",
             (unsigned)(a & 0xffffffffu),
             (unsigned)((a >> 32) & 0xffffu),
             (unsigned)((a >> 48) & 0x0fffu) | 0x4000u,
             (unsigned)((b) & 0x3fffu) | 0x8000u,
             (unsigned long long)((b >> 14) & 0xFFFFFFFFFFFFull));
    return string(buf);
}

static long long nowEpoch() {
    return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
}

static int skipWs(const string& s, int i) {
    while (i < (int)s.size()) {
        char c = s[i];
        if (c==' '||c=='\n'||c=='\r'||c=='\t') i++;
        else break;
    }
    return i;
}

static int indexOfKey(const string& json, const string& quotedKey) {
    bool inStr = false, esc = false;
    for (int i = 0; i <= (int)json.size() - (int)quotedKey.size(); i++) {
        char c = json[i];
        if (inStr) {
            if (esc) esc = false;
            else if (c == '\\') esc = true;
            else if (c == '"') inStr = false;
        } else {
            if (c == '"') {
                if (json.compare(i, quotedKey.size(), quotedKey) == 0) {
                    return i + (int)quotedKey.size();
                } else {
                    inStr = true;
                }
            }
        }
    }
    return -1;
}

static string extractJsonString(const string& json, const string& key) {
    string k = "\"" + key + "\"";
    int idx = indexOfKey(json, k);
    if (idx < 0) throw runtime_error("Missing key: " + key);
    int colon = skipWs(json, idx);
    if (colon >= (int)json.size() || json[colon] != ':') throw runtime_error("Invalid JSON near: " + key);
    int i = skipWs(json, colon + 1);
    if (i >= (int)json.size() || json[i] != '"') throw runtime_error("Expected string for: " + key);
    i++;
    string out;
    bool esc = false;
    while (i < (int)json.size()) {
        char c = json[i++];
        if (esc) {
            switch (c) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    if (i + 4 <= (int)json.size()) {
                        string hex = json.substr(i, 4);
                        int val = 0;
                        if (sscanf(hex.c_str(), "%x", &val) == 1) {
                            out.push_back((char)val);
                            i += 4;
                        } else throw runtime_error("Invalid unicode escape");
                    } else throw runtime_error("Invalid unicode escape length");
                    break;
                }
                default: throw runtime_error("Invalid escape");
            }
            esc = false;
        } else {
            if (c == '\\') esc = true;
            else if (c == '"') break;
            else out.push_back(c);
        }
    }
    return out;
}

static int extractJsonInt(const string& json, const string& key) {
    string k = "\"" + key + "\"";
    int idx = indexOfKey(json, k);
    if (idx < 0) throw runtime_error("Missing key: " + key);
    int colon = skipWs(json, idx);
    if (colon >= (int)json.size() || json[colon] != ':') throw runtime_error("Invalid JSON near: " + key);
    int i = skipWs(json, colon + 1);
    bool neg = false;
    if (i < (int)json.size() && json[i] == '-') { neg = true; i++; }
    int start = i;
    while (i < (int)json.size() && isdigit(static_cast<unsigned char>(json[i]))) i++;
    if (i == start) throw runtime_error("Expected number for: " + key);
    long long val = 0;
    try {
        val = stoll(json.substr(start, i - start));
    } catch (...) {
        throw runtime_error("Invalid number for: " + key);
    }
    if (val < INT_MIN || val > INT_MAX) throw runtime_error("Number out of range for: " + key);
    return (int)val;
}

static User parseUserJson(const string& body) {
    string t = body;
    // Basic check braces
    auto l = t.find_first_not_of(" \r\n\t");
    auto r = t.find_last_not_of(" \r\n\t");
    if (l == string::npos || t[l] != '{' || t[r] != '}') throw runtime_error("Invalid JSON");
    string name = sanitize(extractJsonString(t, "name"));
    string email = sanitize(extractJsonString(t, "email"));
    int age = extractJsonInt(t, "age");
    User u;
    u.id = genUUID();
    u.name = name;
    u.email = email;
    u.age = age;
    u.createdAt = nowEpoch();
    return u;
}

static string userToJson(const User& u) {
    ostringstream oss;
    oss << "{\"id\":\"" << jsonEscape(u.id) << "\","
        << "\"name\":\"" << jsonEscape(u.name) << "\","
        << "\"email\":\"" << jsonEscape(u.email) << "\","
        << "\"age\":" << u.age << ","
        << "\"createdAt\":" << u.createdAt << "}";
    return oss.str();
}

static bool dbAppendUser(const User& u, const string& dbPath) {
    string line = userToJson(u) + "\n";
    ofstream ofs;
    ofs.open(dbPath, ios::out | ios::app);
    if (!ofs.is_open()) return false;
    ofs << line;
    bool ok = ofs.good();
    ofs.close();
    return ok;
}

Response handle_request(const string& method, const string& path, const map<string,string>& headers, const string& body) {
    try {
        if (method.empty() || path.empty()) {
            return {400, "{\"error\":\"Bad Request\"}"};
        }
        string m = method;
        for (auto& c : m) c = toupper(static_cast<unsigned char>(c));
        if (m != "POST") {
            return {405, "{\"error\":\"Method Not Allowed\"}"};
        }
        if (path != "/users") {
            return {404, "{\"error\":\"Not Found\"}"};
        }
        if (body.size() > MAX_BODY_SIZE) {
            return {413, "{\"error\":\"Payload Too Large\"}"};
        }
        string ct;
        auto it = headers.find("Content-Type");
        if (it != headers.end()) ct = it->second;
        else {
            auto it2 = headers.find("content-type");
            if (it2 != headers.end()) ct = it2->second;
        }
        string ctl = ct;
        for (auto& c : ctl) c = tolower(static_cast<unsigned char>(c));
        if (ctl.rfind("application/json", 0) != 0) {
            return {415, "{\"error\":\"Unsupported Media Type\"}"};
        }
        User u = parseUserJson(body);
        if (!validateName(u.name)) return {400, "{\"error\":\"Invalid name\"}"};
        if (!validateEmail(u.email)) return {400, "{\"error\":\"Invalid email\"}"};
        if (!validateAge(u.age)) return {400, "{\"error\":\"Invalid age\"}"};
        if (!dbAppendUser(u, DB_PATH)) return {500, "{\"error\":\"Internal Server Error\"}"};
        string resp = string("{\"message\":\"User created\",\"id\":\"") + jsonEscape(u.id) + "\"}";
        return {201, resp};
    } catch (const runtime_error& e) {
        string err = "{\"error\":\"" + jsonEscape(e.what()) + "\"}";
        return {400, err};
    } catch (...) {
        return {500, "{\"error\":\"Internal Server Error\"}"};
    }
}

int main() {
    map<string,string> headers;
    headers["Content-Type"] = "application/json";
    vector<string> tests = {
        "{\"name\":\"Alice Smith\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"name\":\"Bob\",\"email\":\"bob[at]example.com\",\"age\":25}",
        "{\"email\":\"charlie@example.com\",\"age\":40}",
        "{\"name\":\"Diana\",\"email\":\"diana@example.com\",\"age\":-5}",
        "{\"name\":\"Edward\",\"email\":\"edward@example.com\",\"age\":28}"
    };
    auto r1 = handle_request("POST", "/users", headers, tests[0]);
    cout << r1.statusCode << " " << r1.body << "\n";
    auto r2 = handle_request("POST", "/users", headers, tests[1]);
    cout << r2.statusCode << " " << r2.body << "\n";
    auto r3 = handle_request("POST", "/users", headers, tests[2]);
    cout << r3.statusCode << " " << r3.body << "\n";
    auto r4 = handle_request("POST", "/users", headers, tests[3]);
    cout << r4.statusCode << " " << r4.body << "\n";
    auto r5 = handle_request("GET", "/users", headers, tests[4]);
    cout << r5.statusCode << " " << r5.body << "\n";
    return 0;
}