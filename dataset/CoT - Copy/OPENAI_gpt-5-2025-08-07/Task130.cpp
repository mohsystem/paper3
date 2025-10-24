#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

struct Response {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;
};

static const std::set<std::string> ALLOWED_ORIGINS = {
    "https://trustedorigin.com",
    "http://localhost:3000"
};
static const std::set<std::string> ALLOWED_METHODS = {"GET", "POST"};
static const std::set<std::string> ALLOWED_HEADERS = {
    "content-type", "authorization", "x-requested-with"
};

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string to_upper(const std::string& s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::toupper(c); });
    return out;
}

static std::string to_lower(const std::string& s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

static bool is_origin_allowed(const std::string& origin) {
    if (origin.empty()) return false;
    return ALLOWED_ORIGINS.find(trim(origin)) != ALLOWED_ORIGINS.end();
}

static std::vector<std::string> parse_header_list(const std::string& header_list) {
    std::vector<std::string> out;
    std::string token;
    std::stringstream ss(header_list);
    while (std::getline(ss, token, ',')) {
        std::string t = to_lower(trim(token));
        if (!t.empty()) out.push_back(t);
    }
    return out;
}

static std::string join_csv(const std::vector<std::string>& items) {
    std::ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i) oss << ", ";
        oss << items[i];
    }
    return oss.str();
}

Response handleRequest(const std::string& method,
                       const std::string& path,
                       const std::string& origin,
                       const std::string& accessControlRequestMethod,
                       const std::string& accessControlRequestHeaders) {
    std::string m = to_upper(trim(method));
    std::string p = trim(path);

    Response res;
    res.status = 200;
    res.headers = {
        {"X-Content-Type-Options", "nosniff"},
        {"X-Frame-Options", "DENY"},
        {"Referrer-Policy", "no-referrer"},
        {"Cache-Control", "no-store"}
    };
    res.body = "";

    bool originAllowed = is_origin_allowed(origin);

    if (m == "OPTIONS") {
        if (!originAllowed) {
            res.status = 403;
            return res;
        }
        std::string reqMethod = to_upper(trim(accessControlRequestMethod));
        if (ALLOWED_METHODS.find(reqMethod) == ALLOWED_METHODS.end()) {
            res.status = 405;
            return res;
        }

        std::vector<std::string> requested = parse_header_list(accessControlRequestHeaders);
        std::vector<std::string> allowedBack;
        if (!requested.empty()) {
            for (const auto& h : requested) {
                if (ALLOWED_HEADERS.find(h) != ALLOWED_HEADERS.end()) {
                    allowedBack.push_back(h);
                }
            }
        } else {
            for (const auto& h : ALLOWED_HEADERS) allowedBack.push_back(h);
        }

        res.headers["Access-Control-Allow-Origin"] = trim(origin);
        res.headers["Vary"] = "Origin";
        // Methods
        std::vector<std::string> methods;
        for (const auto& mm : ALLOWED_METHODS) methods.push_back(mm);
        res.headers["Access-Control-Allow-Methods"] = join_csv(methods);
        if (!allowedBack.empty()) {
            res.headers["Access-Control-Allow-Headers"] = join_csv(allowedBack);
        }
        res.headers["Access-Control-Max-Age"] = "600";
        res.status = 204;
        res.body = "";
        return res;
    }

    if (p != "/api/data") {
        res.status = 404;
        res.body = "{\"error\":\"not found\"}";
        return res;
    }
    if (ALLOWED_METHODS.find(m) == ALLOWED_METHODS.end()) {
        res.status = 405;
        res.body = "{\"error\":\"method not allowed\"}";
        return res;
    }

    if (originAllowed) {
        res.headers["Access-Control-Allow-Origin"] = trim(origin);
        res.headers["Vary"] = "Origin";
        res.headers["Access-Control-Expose-Headers"] = "Content-Type, Content-Length";
    }
    res.headers["Content-Type"] = "application/json; charset=utf-8";
    res.status = 200;
    res.body = std::string("{\"message\":\"Hello from CORS protected API\",\"path\":\"") + p + "\"}";
    return res;
}

static void printResponse(const std::string& title, const Response& r) {
    std::cout << "== " << title << " ==\n";
    std::cout << "Status: " << r.status << "\n";
    for (const auto& kv : r.headers) {
        std::cout << kv.first << ": " << kv.second << "\n";
    }
    std::cout << "Body: " << r.body << "\n\n";
}

int main() {
    Response t1 = handleRequest("OPTIONS", "/api/data", "https://trustedorigin.com", "GET", "Content-Type, X-Requested-With");
    Response t2 = handleRequest("OPTIONS", "/api/data", "https://evil.com", "GET", "Content-Type");
    Response t3 = handleRequest("GET", "/api/data", "http://localhost:3000", "", "");
    Response t4 = handleRequest("OPTIONS", "/api/data", "http://localhost:3000", "DELETE", "Content-Type");
    Response t5 = handleRequest("GET", "/api/data", "https://unknown.com", "", "");

    printResponse("Test1 Allowed Origin Preflight", t1);
    printResponse("Test2 Disallowed Origin Preflight", t2);
    printResponse("Test3 Allowed Origin Actual GET", t3);
    printResponse("Test4 Not Allowed Method Preflight", t4);
    printResponse("Test5 Disallowed Origin Actual GET", t5);

    return 0;
}