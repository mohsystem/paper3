#include <bits/stdc++.h>
using namespace std;

struct Response {
    int status;
    unordered_map<string, string> headers;
    string body;
};

static const unordered_set<string> ALLOWED_ORIGINS = {
    "https://example.com",
    "https://app.local"
};
static const unordered_set<string> ALLOWED_METHODS = {"GET", "POST"};
static const string ALLOWED_HEADERS_DEFAULT = "Content-Type, Authorization, X-Requested-With";

static string toUpper(const string& s) {
    string r = s;
    for (auto& c : r) c = (char)toupper((unsigned char)c);
    return r;
}

static bool originAllowed(const string& origin) {
    return !origin.empty() && ALLOWED_ORIGINS.count(origin) > 0;
}

static string getHeaderIgnoreCase(const unordered_map<string, string>& headers, const string& key) {
    for (auto& kv : headers) {
        string k = kv.first;
        string k2 = k;
        string key2 = key;
        transform(k2.begin(), k2.end(), k2.begin(), ::tolower);
        transform(key2.begin(), key2.end(), key2.begin(), ::tolower);
        if (k2 == key2) return kv.second;
    }
    return "";
}

Response handleRequest(const string& methodIn, const string& origin, const string& path, const unordered_map<string, string>& requestHeaders) {
    string method = toUpper(methodIn);
    Response resp;
    resp.status = 200;
    resp.body = "";

    if (method == "OPTIONS") {
        if (!originAllowed(origin)) {
            resp.status = 403;
            resp.body = "Forbidden: Origin not allowed";
            return resp;
        }
        string reqMethod = getHeaderIgnoreCase(requestHeaders, "Access-Control-Request-Method");
        if (reqMethod.empty()) {
            resp.status = 400;
            resp.body = "Bad Request: Missing Access-Control-Request-Method";
            return resp;
        }
        string reqMethodU = toUpper(reqMethod);
        if (ALLOWED_METHODS.count(reqMethodU) == 0) {
            resp.status = 405;
            resp.body = "Method Not Allowed for CORS preflight";
            return resp;
        }
        string reqHeaders = getHeaderIgnoreCase(requestHeaders, "Access-Control-Request-Headers");
        resp.headers["Access-Control-Allow-Origin"] = origin;
        resp.headers["Vary"] = "Origin";
        // join allowed methods
        string methodsJoined;
        for (auto it = ALLOWED_METHODS.begin(); it != ALLOWED_METHODS.end(); ++it) {
            if (!methodsJoined.empty()) methodsJoined += ", ";
            methodsJoined += *it;
        }
        resp.headers["Access-Control-Allow-Methods"] = methodsJoined;
        resp.headers["Access-Control-Allow-Headers"] = reqHeaders.empty() ? ALLOWED_HEADERS_DEFAULT : reqHeaders;
        resp.headers["Access-Control-Max-Age"] = "600";
        resp.status = 204;
        resp.body = "";
        return resp;
    }

    if (path != "/hello") {
        resp.status = 404;
        resp.body = "Not Found";
        return resp;
    }

    if (ALLOWED_METHODS.count(method) == 0) {
        resp.status = 405;
        resp.body = "Method Not Allowed";
        return resp;
    }

    if (!origin.empty()) {
        if (!originAllowed(origin)) {
            resp.status = 403;
            resp.body = "Forbidden: Origin not allowed";
            return resp;
        }
        resp.headers["Access-Control-Allow-Origin"] = origin;
        resp.headers["Vary"] = "Origin";
        resp.headers["Access-Control-Expose-Headers"] = "X-RateLimit-Remaining";
    }
    resp.headers["Content-Type"] = "application/json";
    resp.headers["X-RateLimit-Remaining"] = "42";
    resp.status = 200;
    resp.body = string("{\"message\":\"Hello from CORS-enabled API\",\"method\":\"") + method + "\"}";
    return resp;
}

int main() {
    vector<unordered_map<string, string>> tests;

    // Test 1
    tests.push_back({
        {"METHOD", "GET"},
        {"ORIGIN", "https://example.com"},
        {"PATH", "/hello"}
    });

    // Test 2
    tests.push_back({
        {"METHOD", "OPTIONS"},
        {"ORIGIN", "https://example.com"},
        {"PATH", "/hello"},
        {"Access-Control-Request-Method", "POST"},
        {"Access-Control-Request-Headers", "Content-Type, X-Test"}
    });

    // Test 3
    tests.push_back({
        {"METHOD", "GET"},
        {"ORIGIN", "https://evil.com"},
        {"PATH", "/hello"}
    });

    // Test 4
    tests.push_back({
        {"METHOD", "POST"},
        {"ORIGIN", "https://app.local"},
        {"PATH", "/hello"}
    });

    // Test 5
    tests.push_back({
        {"METHOD", "OPTIONS"},
        {"ORIGIN", "https://example.com"},
        {"PATH", "/hello"},
        {"Access-Control-Request-Method", "PUT"}
    });

    int idx = 1;
    for (auto& t : tests) {
        string method = t.count("METHOD") ? t["METHOD"] : "GET";
        string origin = t.count("ORIGIN") ? t["ORIGIN"] : "";
        string path = t.count("PATH") ? t["PATH"] : "/hello";
        unordered_map<string, string> reqHeaders;
        for (auto& kv : t) {
            if (kv.first != "METHOD" && kv.first != "ORIGIN" && kv.first != "PATH") {
                reqHeaders[kv.first] = kv.second;
            }
        }
        Response r = handleRequest(method, origin, path, reqHeaders);
        cout << "=== C++ Test " << idx++ << " ===\n";
        cout << "Status: " << r.status << "\n";
        for (auto& h : r.headers) {
            cout << h.first << ": " << h.second << "\n";
        }
        cout << "Body: " << r.body << "\n";
    }

    return 0;
}