#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

struct Response {
    int status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

static std::vector<std::string> ALLOWED_ORIGINS = {
    "https://example.com",
    "https://app.example.com"
};
static std::vector<std::string> ALLOWED_METHODS = {"GET", "POST", "OPTIONS"};
static std::vector<std::string> ALLOWED_HEADERS = {"content-type", "x-requested-with"};
static const int MAX_AGE_SECONDS = 600;
static const bool ALLOW_CREDENTIALS = false;

static std::string toLower(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}
static std::string toUpper(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::toupper(c)));
    return out;
}

static std::string getHeader(const std::unordered_map<std::string, std::string> &headers, const std::string &key) {
    for (const auto &kv : headers) {
        if (toLower(kv.first) == toLower(key)) {
            return kv.second;
        }
    }
    return "";
}

static bool contains(const std::vector<std::string>& vec, const std::string& val, bool caseSensitive = true) {
    for (const auto &x : vec) {
        if (caseSensitive) {
            if (x == val) return true;
        } else {
            if (toLower(x) == toLower(val)) return true;
        }
    }
    return false;
}

static bool isValidToken(const std::string &s, size_t maxLen) {
    if (s.empty() || s.size() > maxLen) return false;
    return true;
}

static void setCorsHeaders(std::unordered_map<std::string, std::string> &outHeaders, const std::string &origin) {
    outHeaders["Vary"] = "Origin";
    outHeaders["Access-Control-Allow-Origin"] = origin;
    if (ALLOW_CREDENTIALS) {
        outHeaders["Access-Control-Allow-Credentials"] = "true";
    }
    outHeaders["Access-Control-Expose-Headers"] = "Content-Type";
}

static std::string join(const std::vector<std::string>& v, const std::string &sep) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        out += v[i];
        if (i + 1 < v.size()) out += sep;
    }
    return out;
}

static std::vector<std::string> sortedVec(const std::vector<std::string>& v, bool ci = false) {
    std::vector<std::string> out = v;
    if (ci) {
        std::sort(out.begin(), out.end(), [](const std::string &a, const std::string &b){
            return toLower(a) < toLower(b);
        });
    } else {
        std::sort(out.begin(), out.end());
    }
    return out;
}

Response processRequest(const std::string &methodIn, const std::string &pathIn,
                        const std::unordered_map<std::string, std::string> &headers, const std::string &body) {
    std::unordered_map<std::string, std::string> outHeaders;

    std::string method = toUpper(methodIn);
    std::string path = pathIn;

    if (!isValidToken(method, 16) || !isValidToken(path, 128) || (path.empty() || path[0] != '/')) {
        return {400, outHeaders, "Bad Request"};
    }

    std::string origin = getHeader(headers, "Origin");
    if (!origin.empty() && origin.size() > 200) {
        return {400, outHeaders, "Invalid Origin"};
    }

    bool isPreflight = (method == "OPTIONS") && !origin.empty() && !getHeader(headers, "Access-Control-Request-Method").empty();

    if (isPreflight) {
        // Preflight handling
        if (!contains(ALLOWED_ORIGINS, origin, true)) {
            return {403, outHeaders, "CORS preflight failed: origin not allowed"};
        }
        std::string reqMethod = toUpper(getHeader(headers, "Access-Control-Request-Method"));
        if (!contains(ALLOWED_METHODS, reqMethod, true)) {
            return {403, outHeaders, "CORS preflight failed: method not allowed"};
        }
        std::string acrh = getHeader(headers, "Access-Control-Request-Headers");
        if (!acrh.empty()) {
            // parse by comma
            std::string acc = acrh;
            size_t start = 0;
            while (start < acc.size()) {
                size_t pos = acc.find(',', start);
                std::string token = acc.substr(start, (pos == std::string::npos ? acc.size() : pos) - start);
                // trim
                size_t l = 0, r = token.size();
                while (l < r && std::isspace(static_cast<unsigned char>(token[l]))) l++;
                while (r > l && std::isspace(static_cast<unsigned char>(token[r - 1]))) r--;
                std::string h = toLower(token.substr(l, r - l));
                if (!h.empty() && !contains(ALLOWED_HEADERS, h, false)) {
                    return {403, outHeaders, std::string("CORS preflight failed: header not allowed -> ") + h};
                }
                if (pos == std::string::npos) break;
                start = pos + 1;
            }
        }
        setCorsHeaders(outHeaders, origin);
        auto methSorted = sortedVec(ALLOWED_METHODS, true);
        auto hdrSorted = sortedVec(ALLOWED_HEADERS, true);
        outHeaders["Access-Control-Allow-Methods"] = join(methSorted, ", ");
        outHeaders["Access-Control-Allow-Headers"] = join(hdrSorted, ", ");
        outHeaders["Access-Control-Max-Age"] = std::to_string(MAX_AGE_SECONDS);
        return {204, outHeaders, ""};
    } else {
        // Actual request
        if (!origin.empty()) {
            if (!contains(ALLOWED_ORIGINS, origin, true)) {
                return {403, outHeaders, "CORS error: origin not allowed"};
            }
            setCorsHeaders(outHeaders, origin);
        }

        if (method == "GET" && path == "/data") {
            outHeaders["Content-Type"] = "application/json; charset=UTF-8";
            return {200, outHeaders, "{\"message\":\"ok\"}"};
        } else if (path != "/data") {
            return {404, outHeaders, "Not Found"};
        } else {
            return {405, outHeaders, "Method Not Allowed"};
        }
    }
}

static std::unordered_map<std::string, std::string> headersOf(const std::vector<std::pair<std::string, std::string>>& kv) {
    std::unordered_map<std::string, std::string> m;
    for (const auto &p : kv) {
        if (p.first.size() <= 64 && p.second.size() <= 512) {
            m[p.first] = p.second;
        }
    }
    return m;
}

static void printResponse(const std::string &label, const Response &r) {
    std::cout << label << "\n";
    std::cout << "Status: " << r.status << "\n";
    std::cout << "Headers:\n";
    for (const auto &kv : r.headers) {
        std::cout << "  " << kv.first << ": " << kv.second << "\n";
    }
    std::cout << "Body: " << r.body << "\n\n";
}

int main() {
    // Test 1: Allowed origin GET
    Response r1 = processRequest(
        "GET", "/data",
        headersOf({{"Origin", "https://example.com"}}),
        ""
    );
    printResponse("Test 1:", r1);

    // Test 2: Disallowed origin
    Response r2 = processRequest(
        "GET", "/data",
        headersOf({{"Origin", "https://evil.com"}}),
        ""
    );
    printResponse("Test 2:", r2);

    // Test 3: Preflight allowed
    Response r3 = processRequest(
        "OPTIONS", "/data",
        headersOf({
            {"Origin", "https://app.example.com"},
            {"Access-Control-Request-Method", "GET"},
            {"Access-Control-Request-Headers", "Content-Type, X-Requested-With"}
        }),
        ""
    );
    printResponse("Test 3:", r3);

    // Test 4: Preflight with disallowed header
    Response r4 = processRequest(
        "OPTIONS", "/data",
        headersOf({
            {"Origin", "https://example.com"},
            {"Access-Control-Request-Method", "GET"},
            {"Access-Control-Request-Headers", "X-Secret"}
        }),
        ""
    );
    printResponse("Test 4:", r4);

    // Test 5: No Origin header
    Response r5 = processRequest(
        "GET", "/data",
        headersOf({{"User-Agent", "TestClient/1.0"}}),
        ""
    );
    printResponse("Test 5:", r5);

    return 0;
}