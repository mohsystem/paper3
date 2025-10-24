#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct Request {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

static std::string to_lower(const std::string &s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

static bool secure_rand_bytes(std::vector<unsigned char>& out) {
    if (out.empty()) return false;
    if (RAND_bytes(out.data(), static_cast<int>(out.size())) != 1) return false;
    return true;
}

static std::string hex_encode(const std::vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(data.size() * 2);
    for (unsigned char b : data) {
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

static bool constant_time_eq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}

static std::vector<unsigned char> pbkdf2_sha256(const std::string& password, const std::vector<unsigned char>& salt, int iterations, size_t dkLen) {
    std::vector<unsigned char> out(dkLen, 0);
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), static_cast<int>(dkLen), out.data()) != 1) {
        out.clear();
    }
    return out;
}

static bool password_policy_check(const std::string& pwd, std::string& err) {
    // Policy: min 12 chars, at least 1 upper, 1 lower, 1 digit, 1 special
    if (pwd.size() < 12) { err = "Password too short"; return false; }
    bool hasU=false, hasL=false, hasD=false, hasS=false;
    for (unsigned char c: pwd) {
        if (std::isupper(c)) hasU = true;
        else if (std::islower(c)) hasL = true;
        else if (std::isdigit(c)) hasD = true;
        else hasS = true;
    }
    if (!hasU) { err = "Missing uppercase"; return false; }
    if (!hasL) { err = "Missing lowercase"; return false; }
    if (!hasD) { err = "Missing digit"; return false; }
    if (!hasS) { err = "Missing special"; return false; }
    err.clear();
    return true;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        char c = s[i];
        if (c == '%' && i + 2 < s.size() && std::isxdigit(static_cast<unsigned char>(s[i+1])) && std::isxdigit(static_cast<unsigned char>(s[i+2]))) {
            int v = 0;
            for (size_t k = 1; k <= 2; ++k) {
                v <<= 4;
                char hc = s[i+k];
                if (hc >= '0' && hc <= '9') v |= (hc - '0');
                else if (hc >= 'a' && hc <= 'f') v |= (hc - 'a' + 10);
                else if (hc >= 'A' && hc <= 'F') v |= (hc - 'A' + 10);
            }
            out.push_back(static_cast<char>(v));
            i += 3;
        } else if (c == '+') {
            out.push_back(' ');
            i++;
        } else {
            out.push_back(c);
            i++;
        }
    }
    return out;
}

static std::unordered_map<std::string, std::string> parse_form_urlencoded(const std::string& body, size_t maxPairs = 32) {
    std::unordered_map<std::string, std::string> out;
    size_t start = 0;
    size_t pairs = 0;
    while (start < body.size() && pairs < maxPairs) {
        size_t amp = body.find('&', start);
        std::string kv = amp == std::string::npos ? body.substr(start) : body.substr(start, amp - start);
        size_t eq = kv.find('=');
        std::string k = url_decode(eq == std::string::npos ? kv : kv.substr(0, eq));
        std::string v = url_decode(eq == std::string::npos ? "" : kv.substr(eq + 1));
        if (!k.empty()) out[k] = v;
        if (amp == std::string::npos) break;
        start = amp + 1;
        pairs++;
    }
    return out;
}

static std::unordered_map<std::string, std::string> parse_cookies(const std::string& cookieHeader, size_t maxCookies = 16) {
    std::unordered_map<std::string, std::string> out;
    size_t start = 0;
    size_t count = 0;
    while (start < cookieHeader.size() && count < maxCookies) {
        size_t semi = cookieHeader.find(';', start);
        std::string part = semi == std::string::npos ? cookieHeader.substr(start) : cookieHeader.substr(start, semi - start);
        size_t eq = part.find('=');
        std::string k, v;
        if (eq != std::string::npos) {
            k = part.substr(0, eq);
            v = part.substr(eq + 1);
        } else {
            k = part;
            v = "";
        }
        // trim
        auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
        auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);} ).base(), s.end()); };
        ltrim(k); rtrim(k); ltrim(v); rtrim(v);
        if (!k.empty()) out[k] = v;
        if (semi == std::string::npos) break;
        start = semi + 1;
        count++;
    }
    return out;
}

static bool parse_request(const std::string& raw, Request& req) {
    size_t pos = raw.find("\r\n");
    if (pos == std::string::npos) return false;
    std::string requestLine = raw.substr(0, pos);
    size_t sp1 = requestLine.find(' ');
    if (sp1 == std::string::npos) return false;
    size_t sp2 = requestLine.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return false;
    req.method = requestLine.substr(0, sp1);
    req.path = requestLine.substr(sp1 + 1, sp2 - sp1 - 1);
    size_t headerStart = pos + 2;
    size_t headerEnd = raw.find("\r\n\r\n", headerStart);
    if (headerEnd == std::string::npos) return false;
    size_t cursor = headerStart;
    while (cursor < headerEnd) {
        size_t lineEnd = raw.find("\r\n", cursor);
        if (lineEnd == std::string::npos || lineEnd > headerEnd) break;
        std::string line = raw.substr(cursor, lineEnd - cursor);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string k = line.substr(0, colon);
            std::string v = line.substr(colon + 1);
            // trim
            auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
            auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);} ).base(), s.end()); };
            ltrim(k); rtrim(k); ltrim(v); rtrim(v);
            req.headers[to_lower(k)] = v;
        }
        cursor = lineEnd + 2;
    }
    req.body = raw.substr(headerEnd + 4);
    return true;
}

static std::string http_status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

static std::string build_response(const Response& res) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << res.status << " " << http_status_text(res.status) << "\r\n";
    // Security headers
    oss << "Content-Security-Policy: default-src 'none'\r\n";
    oss << "X-Content-Type-Options: nosniff\r\n";
    oss << "X-Frame-Options: DENY\r\n";
    oss << "Referrer-Policy: no-referrer\r\n";
    oss << "Cache-Control: no-store\r\n";
    for (const auto& kv : res.headers) {
        oss << kv.first << ": " << kv.second << "\r\n";
    }
    oss << "Content-Length: " << res.body.size() << "\r\n";
    oss << "\r\n";
    oss << res.body;
    return oss.str();
}

struct User {
    std::string username;
    std::vector<unsigned char> salt; // 16 bytes
    std::vector<unsigned char> pwd_hash; // 32 bytes
    std::chrono::system_clock::time_point pwd_set;
};

struct Session {
    std::string sid; // hex(32 bytes)
    std::string username;
    std::chrono::system_clock::time_point expiry;
};

class App {
public:
    App(): iterations(210000), session_ttl_secs(1200) {}

    bool register_user(const std::string& username, const std::string& password, std::string& err) {
        if (username.empty() || username.size() > 64) { err = "Invalid username"; return false; }
        if (users.find(username) != users.end()) { err = "User exists"; return false; }
        if (!password_policy_check(password, err)) return false;
        std::vector<unsigned char> salt(16);
        if (!secure_rand_bytes(salt)) { err = "RNG failure"; return false; }
        std::vector<unsigned char> hash = pbkdf2_sha256(password, salt, iterations, 32);
        if (hash.empty()) { err = "Hash failure"; return false; }
        User u;
        u.username = username;
        u.salt = std::move(salt);
        u.pwd_hash = std::move(hash);
        u.pwd_set = std::chrono::system_clock::now();
        users[username] = std::move(u);
        return true;
    }

    bool verify_credentials(const std::string& username, const std::string& password) const {
        auto it = users.find(username);
        if (it == users.end()) return false;
        const User& u = it->second;
        std::vector<unsigned char> hash = pbkdf2_sha256(password, u.salt, iterations, 32);
        if (hash.empty()) return false;
        if (u.pwd_hash.size() != hash.size()) return false;
        return CRYPTO_memcmp(u.pwd_hash.data(), hash.data(), u.pwd_hash.size()) == 0;
    }

    std::string create_session(const std::string& username) {
        std::vector<unsigned char> sid_bytes(32);
        if (!secure_rand_bytes(sid_bytes)) return "";
        std::string sid = hex_encode(sid_bytes);
        Session s;
        s.sid = sid;
        s.username = username;
        s.expiry = std::chrono::system_clock::now() + std::chrono::seconds(session_ttl_secs);
        sessions[sid] = std::move(s);
        return sid;
    }

    bool terminate_session(const std::string& sid) {
        auto it = sessions.find(sid);
        if (it == sessions.end()) return false;
        sessions.erase(it);
        return true;
    }

    std::string username_from_session(const std::string& sid) {
        cleanup_expired();
        auto it = sessions.find(sid);
        if (it == sessions.end()) return "";
        if (std::chrono::system_clock::now() > it->second.expiry) {
            sessions.erase(it);
            return "";
        }
        return it->second.username;
    }

    std::string process_http_request(const std::string& raw) {
        Request req;
        Response res;
        res.status = 500;
        res.headers["Content-Type"] = "text/plain; charset=utf-8";
        if (!parse_request(raw, req)) {
            res.status = 400;
            res.body = "Malformed request";
            return build_response(res);
        }
        if (req.path == "/register" && req.method == "POST") {
            std::string ctype = "";
            auto it = req.headers.find("content-type");
            if (it != req.headers.end()) ctype = to_lower(it->second);
            if (ctype.find("application/x-www-form-urlencoded") == std::string::npos) {
                res.status = 400; res.body = "Unsupported Content-Type"; return build_response(res);
            }
            auto form = parse_form_urlencoded(req.body);
            std::string user = form.count("username") ? form["username"] : "";
            std::string pass = form.count("password") ? form["password"] : "";
            std::string err;
            if (!register_user(user, pass, err)) {
                res.status = 400; res.body = "Registration failed";
            } else {
                res.status = 201; res.body = "Registered";
            }
            return build_response(res);
        } else if (req.path == "/login" && req.method == "POST") {
            std::string ctype = "";
            auto it = req.headers.find("content-type");
            if (it != req.headers.end()) ctype = to_lower(it->second);
            if (ctype.find("application/x-www-form-urlencoded") == std::string::npos) {
                res.status = 400; res.body = "Unsupported Content-Type"; return build_response(res);
            }
            auto form = parse_form_urlencoded(req.body);
            std::string user = form.count("username") ? form["username"] : "";
            std::string pass = form.count("password") ? form["password"] : "";
            if (!verify_credentials(user, pass)) {
                res.status = 401; res.body = "Unauthorized";
                return build_response(res);
            }
            std::string sid = create_session(user);
            if (sid.empty()) {
                res.status = 500; res.body = "Session error"; return build_response(res);
            }
            res.status = 200; res.body = "Logged in";
            std::string cookie = "SID=" + sid + "; HttpOnly; Secure; SameSite=Strict; Path=/";
            res.headers["Set-Cookie"] = cookie;
            return build_response(res);
        } else if (req.path == "/me" && req.method == "GET") {
            std::string sid;
            auto it = req.headers.find("cookie");
            if (it != req.headers.end()) {
                auto cookies = parse_cookies(it->second);
                if (cookies.count("SID")) sid = cookies["SID"];
            }
            if (sid.empty()) {
                res.status = 401; res.body = "Unauthorized"; return build_response(res);
            }
            std::string username = username_from_session(sid);
            if (username.empty()) {
                res.status = 401; res.body = "Unauthorized"; return build_response(res);
            }
            res.status = 200;
            res.body = "Hello, " + username;
            return build_response(res);
        } else if (req.path == "/logout" && req.method == "POST") {
            std::string sid;
            auto it = req.headers.find("cookie");
            if (it != req.headers.end()) {
                auto cookies = parse_cookies(it->second);
                if (cookies.count("SID")) sid = cookies["SID"];
            }
            if (sid.empty() || !terminate_session(sid)) {
                res.status = 401; res.body = "Unauthorized"; return build_response(res);
            }
            res.status = 200; res.body = "Logged out";
            // Expire cookie
            res.headers["Set-Cookie"] = "SID=; HttpOnly; Secure; SameSite=Strict; Path=/; Max-Age=0";
            return build_response(res);
        } else {
            res.status = 404; res.body = "Not Found"; return build_response(res);
        }
    }

private:
    void cleanup_expired() {
        auto now = std::chrono::system_clock::now();
        std::vector<std::string> toErase;
        toErase.reserve(sessions.size());
        for (const auto& kv : sessions) {
            if (now > kv.second.expiry) toErase.push_back(kv.first);
        }
        for (const auto& k : toErase) sessions.erase(k);
    }

    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, Session> sessions;
    const int iterations;
    const int session_ttl_secs;
};

static std::string make_request(const std::string& method, const std::string& path,
                                const std::unordered_map<std::string,std::string>& headers,
                                const std::string& body) {
    std::ostringstream oss;
    oss << method << " " << path << " HTTP/1.1\r\n";
    for (const auto& kv : headers) {
        oss << kv.first << ": " << kv.second << "\r\n";
    }
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "\r\n";
    oss << body;
    return oss.str();
}

static std::string extract_header(const std::string& resp, const std::string& name) {
    std::string lname = to_lower(name);
    size_t pos = 0;
    while (true) {
        size_t lineEnd = resp.find("\r\n", pos);
        if (lineEnd == std::string::npos) break;
        if (lineEnd == pos) break; // end of headers
        std::string line = resp.substr(pos, lineEnd - pos);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string k = line.substr(0, colon);
            std::string v = line.substr(colon + 1);
            auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
            auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);} ).base(), s.end()); };
            std::string kl = to_lower(k);
            ltrim(v); rtrim(v);
            if (kl == lname) return v;
        }
        pos = lineEnd + 2;
    }
    return "";
}

int main() {
    App app;

    // Test 1: Register user with strong password
    {
        std::string body = "username=alice&password=Str0ng!Passw0rd#";
        std::unordered_map<std::string,std::string> headers = {{"Host","localhost"},{"Content-Type","application/x-www-form-urlencoded"}};
        std::string req = make_request("POST","/register",headers,body);
        std::string resp = app.process_http_request(req);
        std::cout << "Test1 Status: " << resp.substr(9,3) << "\n";
    }

    // Test 2: Login with wrong password
    std::string cookieSid;
    {
        std::string body = "username=alice&password=WrongPass!123";
        std::unordered_map<std::string,std::string> headers = {{"Host","localhost"},{"Content-Type","application/x-www-form-urlencoded"}};
        std::string req = make_request("POST","/login",headers,body);
        std::string resp = app.process_http_request(req);
        std::cout << "Test2 Status: " << resp.substr(9,3) << "\n";
    }

    // Test 3: Login with correct password and access /me
    {
        std::string body = "username=alice&password=Str0ng!Passw0rd#";
        std::unordered_map<std::string,std::string> headers = {{"Host","localhost"},{"Content-Type","application/x-www-form-urlencoded"}};
        std::string req = make_request("POST","/login",headers,body);
        std::string resp = app.process_http_request(req);
        std::cout << "Test3 Login Status: " << resp.substr(9,3) << "\n";
        std::string setCookie = extract_header(resp, "Set-Cookie");
        // Extract SID from Set-Cookie
        std::string sid;
        auto cookies = parse_cookies(setCookie);
        if (cookies.count("SID")) sid = cookies["SID"];
        cookieSid = sid;
        std::unordered_map<std::string,std::string> h2 = {{"Host","localhost"}};
        if (!sid.empty()) h2["Cookie"] = "SID=" + sid;
        std::string req2 = make_request("GET","/me",h2,"");
        std::string resp2 = app.process_http_request(req2);
        std::cout << "Test3 /me Status: " << resp2.substr(9,3) << "\n";
    }

    // Test 4: Logout
    {
        std::unordered_map<std::string,std::string> headers = {{"Host","localhost"}};
        if (!cookieSid.empty()) headers["Cookie"] = "SID=" + cookieSid;
        std::string req = make_request("POST","/logout",headers,"");
        std::string resp = app.process_http_request(req);
        std::cout << "Test4 Logout Status: " << resp.substr(9,3) << "\n";
    }

    // Test 5: Access /me after logout should be unauthorized
    {
        std::unordered_map<std::string,std::string> headers = {{"Host","localhost"}};
        if (!cookieSid.empty()) headers["Cookie"] = "SID=" + cookieSid;
        std::string req = make_request("GET","/me",headers,"");
        std::string resp = app.process_http_request(req);
        std::cout << "Test5 /me after logout Status: " << resp.substr(9,3) << "\n";
    }

    return 0;
}