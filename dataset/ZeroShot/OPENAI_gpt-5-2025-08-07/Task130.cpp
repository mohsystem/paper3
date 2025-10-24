#include <bits/stdc++.h>
using namespace std;

struct Response {
    int status;
    map<string,string> headers;
    string body;
};

static const set<string> ALLOWED_ORIGINS = {"https://example.com", "https://app.example.com"};
static const set<string> ALLOWED_METHODS = {"GET","POST","OPTIONS"};
static const set<string> ALLOWED_HEADERS = {"content-type","authorization","x-requested-with"};
static const string EXPOSE_HEADERS = "X-Response-Time";
static const int MAX_AGE = 600;
static const size_t MAX_HEADER_LIST_LEN = 1024;
static const size_t MAX_HEADERS = 50;

static bool isToken(const string& s) {
    if (s.empty() || s.size() > 100) return false;
    for (char c : s) {
        if (!(isalnum(static_cast<unsigned char>(c)) || c=='-')) return false;
    }
    return true;
}

static optional<vector<string>> parseHeaderList(const string& headerList) {
    if (headerList.size() > MAX_HEADER_LIST_LEN) return nullopt;
    vector<string> out;
    string cur;
    for (size_t i=0;i<headerList.size();++i) {
        char c = headerList[i];
        if (c == ',') {
            // trim
            size_t start = cur.find_first_not_of(" \t\r\n");
            size_t end = cur.find_last_not_of(" \t\r\n");
            string t = (start==string::npos) ? "" : cur.substr(start, end-start+1);
            if (!t.empty()) {
                if (!isToken(t)) return nullopt;
                if (out.size() < MAX_HEADERS) out.push_back(t);
            }
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    // last token
    if (!cur.empty()) {
        size_t start = cur.find_first_not_of(" \t\r\n");
        size_t end = cur.find_last_not_of(" \t\r\n");
        string t = (start==string::npos) ? "" : cur.substr(start, end-start+1);
        if (!t.empty()) {
            if (!isToken(t)) return nullopt;
            if (out.size() < MAX_HEADERS) out.push_back(t);
        }
    }
    return out;
}

Response handleRequest(const string& method,
                       const string& origin,
                       const string& accessControlRequestMethod,
                       const string& accessControlRequestHeaders,
                       const string& path,
                       const string& requestBody) {
    string m = method;
    for (auto &c: m) c = toupper(static_cast<unsigned char>(c));
    string o = origin;
    // trim origin
    auto trim = [](const string& s)->string {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a==string::npos) return "";
        return s.substr(a, b-a+1);
    };
    o = trim(o);
    bool hasOrigin = !o.empty();

    Response resp;
    if (m == "OPTIONS") {
        resp.headers["Vary"] = "Origin, Access-Control-Request-Method, Access-Control-Request-Headers";
    } else {
        resp.headers["Vary"] = "Origin";
    }

    bool originAllowed = hasOrigin && ALLOWED_ORIGINS.count(o) > 0;

    if (m == "OPTIONS") {
        if (!originAllowed) {
            resp.status = 403;
            return resp;
        }
        string reqMethod = accessControlRequestMethod;
        for (auto &c: reqMethod) c = toupper(static_cast<unsigned char>(c));
        if (ALLOWED_METHODS.count(reqMethod) == 0 || reqMethod == "OPTIONS") {
            resp.status = 403;
            return resp;
        }
        auto parsed = parseHeaderList(accessControlRequestHeaders);
        if (!parsed.has_value()) {
            resp.status = 400;
            return resp;
        }
        for (const auto& h : parsed.value()) {
            string lower = h;
            for (auto &c: lower) c = tolower(static_cast<unsigned char>(c));
            if (ALLOWED_HEADERS.count(lower) == 0) {
                resp.status = 403;
                return resp;
            }
        }
        resp.headers["Access-Control-Allow-Origin"] = o;
        resp.headers["Access-Control-Allow-Credentials"] = "true";
        resp.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
        // echo requested headers (as provided)
        string echo;
        for (size_t i=0;i<parsed->size();++i) {
            if (i) echo += ", ";
            echo += parsed->at(i);
        }
        resp.headers["Access-Control-Allow-Headers"] = echo;
        resp.headers["Access-Control-Max-Age"] = to_string(MAX_AGE);
        resp.status = 204;
        return resp;
    }

    if (hasOrigin && !originAllowed) {
        resp.status = 403;
        resp.body = "{\"error\":\"CORS origin denied\"}";
        return resp;
    }

    if (path == "/api/data") {
        if (m == "GET") {
            long long ts = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            resp.body = string("{\"message\":\"Hello\",\"ts\":") + to_string(ts) + "}";
            resp.headers["Content-Type"] = "application/json; charset=utf-8";
            resp.headers["X-Response-Time"] = "1";
            resp.headers["Cache-Control"] = "no-store";
            if (originAllowed) {
                resp.headers["Access-Control-Allow-Origin"] = o;
                resp.headers["Access-Control-Allow-Credentials"] = "true";
                resp.headers["Access-Control-Expose-Headers"] = EXPOSE_HEADERS;
            }
            resp.status = 200;
            return resp;
        } else if (m == "POST") {
            string safe = requestBody.substr(0, min<size_t>(requestBody.size(), 1024));
            // simple JSON string escape
            string esc = "\"";
            for (char c : safe) {
                switch (c) {
                    case '\\': esc += "\\\\"; break;
                    case '"': esc += "\\\""; break;
                    case '\b': esc += "\\b"; break;
                    case '\f': esc += "\\f"; break;
                    case '\n': esc += "\\n"; break;
                    case '\r': esc += "\\r"; break;
                    case '\t': esc += "\\t"; break;
                    default:
                        if ((unsigned char)c < 0x20) {
                            char buf[7];
                            snprintf(buf, sizeof(buf), "\\u%04x", c);
                            esc += buf;
                        } else {
                            esc += c;
                        }
                }
            }
            esc += "\"";
            resp.body = string("{\"received\":") + esc + "}";
            resp.headers["Content-Type"] = "application/json; charset=utf-8";
            resp.headers["X-Response-Time"] = "1";
            resp.headers["Cache-Control"] = "no-store";
            if (originAllowed) {
                resp.headers["Access-Control-Allow-Origin"] = o;
                resp.headers["Access-Control-Allow-Credentials"] = "true";
                resp.headers["Access-Control-Expose-Headers"] = EXPOSE_HEADERS;
            }
            resp.status = 200;
            return resp;
        } else {
            resp.status = 405;
            return resp;
        }
    } else {
        resp.status = 404;
        return resp;
    }
}

static void printResult(const string& label, const Response& r) {
    cout << "=== " << label << " ===\n";
    cout << "Status: " << r.status << "\n";
    for (const auto& kv : r.headers) {
        cout << kv.first << ": " << kv.second << "\n";
    }
    if (!r.body.empty()) {
        cout << "Body: " << r.body << "\n";
    }
}

int main() {
    Response r1 = handleRequest("OPTIONS", "https://example.com", "GET", "Content-Type, X-Requested-With", "/api/data", "");
    printResult("Test1 Preflight OK", r1);

    Response r2 = handleRequest("GET", "https://app.example.com", "", "", "/api/data", "");
    printResult("Test2 GET with CORS", r2);

    Response r3 = handleRequest("POST", "https://evil.com", "", "", "/api/data", "{\"a\":1}");
    printResult("Test3 POST disallowed origin", r3);

    Response r4 = handleRequest("GET", "", "", "", "/api/data", "");
    printResult("Test4 GET no origin", r4);

    Response r5 = handleRequest("OPTIONS", "https://example.com", "DELETE", "Authorization", "/api/data", "");
    printResult("Test5 Preflight bad method", r5);

    return 0;
}