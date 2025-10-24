#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sqlite3.h>

// Security limits
static const size_t MAX_REQUEST_SIZE = 8192;
static const size_t MAX_BODY_SIZE = 4096;
static const size_t MAX_FIELD_LEN = 256;

// Database path for this program
static const char* DB_PATH = "users_cpp.db";

// Utility: trim spaces
static inline std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) b++;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) e--;
    return s.substr(b, e - b);
}

// Utility: lowercase copy
static inline std::string toLower(const std::string& s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

// Validate username: [A-Za-z0-9_], len 3..32
static bool validateUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

// Validate email: simple rule set, len 3..254, exactly one '@', domain has '.', allowed chars
static bool validateEmail(const std::string& email) {
    if (email.size() < 3 || email.size() > 254) return false;
    if (email.find(' ') != std::string::npos) return false;
    size_t at = email.find('@');
    if (at == std::string::npos || email.find('@', at + 1) != std::string::npos) return false;
    std::string local = email.substr(0, at);
    std::string domain = email.substr(at + 1);
    if (local.empty() || domain.empty()) return false;
    if (domain.front() == '.' || domain.back() == '.') return false;
    if (domain.find('.') == std::string::npos) return false;
    auto allowed = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_' || c == '+';
    };
    for (char c : local) if (!allowed(c)) return false;
    for (char c : domain) if (!allowed(c)) return false;
    return true;
}

// Validate age: 13..120
static bool validateAge(int age) {
    return age >= 13 && age <= 120;
}

// Simple JSON string extractor: expects "key":"value"
static bool jsonExtractString(const std::string& body, const std::string& key, std::string& out) {
    out.clear();
    std::string needle = "\"" + key + "\"";
    size_t kpos = body.find(needle);
    if (kpos == std::string::npos) return false;
    size_t colon = body.find(':', kpos + needle.size());
    if (colon == std::string::npos) return false;
    // Skip whitespace
    size_t i = colon + 1;
    while (i < body.size() && std::isspace(static_cast<unsigned char>(body[i]))) i++;
    if (i >= body.size() || body[i] != '\"') return false;
    i++;
    std::string val;
    for (; i < body.size(); ++i) {
        char c = body[i];
        if (c == '\\') {
            if (i + 1 < body.size()) {
                char n = body[i + 1];
                // Simple escape handling for \" and \\ only
                if (n == '\"' || n == '\\') {
                    val.push_back(n);
                    i++;
                    continue;
                }
            }
            return false; // reject unknown escapes
        } else if (c == '\"') {
            out = val;
            return true;
        } else {
            if (val.size() >= MAX_FIELD_LEN) return false;
            val.push_back(c);
        }
    }
    return false;
}

// Simple JSON int extractor: expects "key":number
static bool jsonExtractInt(const std::string& body, const std::string& key, int& out) {
    out = 0;
    std::string needle = "\"" + key + "\"";
    size_t kpos = body.find(needle);
    if (kpos == std::string::npos) return false;
    size_t colon = body.find(':', kpos + needle.size());
    if (colon == std::string::npos) return false;
    size_t i = colon + 1;
    while (i < body.size() && std::isspace(static_cast<unsigned char>(body[i]))) i++;
    if (i >= body.size()) return false;
    bool neg = false;
    if (body[i] == '-') { neg = true; i++; }
    if (i >= body.size() || !std::isdigit(static_cast<unsigned char>(body[i]))) return false;
    long long val = 0;
    while (i < body.size() && std::isdigit(static_cast<unsigned char>(body[i]))) {
        val = val * 10 + (body[i] - '0');
        if (val > std::numeric_limits<int>::max()) return false;
        i++;
    }
    if (neg) val = -val;
    if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) return false;
    out = static_cast<int>(val);
    return true;
}

// Build HTTP response
static std::string buildResponse(int status, const std::string& message, const std::string& extraJson = "") {
    std::ostringstream body;
    body << "{";
    body << "\"message\":\"";
    for (char c : message) {
        if (c == '"' || c == '\\') body << '\\' << c;
        else if (static_cast<unsigned char>(c) < 0x20) body << ' ';
        else body << c;
    }
    body << "\"";
    if (!extraJson.empty()) body << "," << extraJson;
    body << "}";
    std::string bodyStr = body.str();

    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " ";
    switch (status) {
        case 201: oss << "Created"; break;
        case 200: oss << "OK"; break;
        case 400: oss << "Bad Request"; break;
        case 404: oss << "Not Found"; break;
        case 405: oss << "Method Not Allowed"; break;
        case 411: oss << "Length Required"; break;
        case 413: oss << "Payload Too Large"; break;
        case 415: oss << "Unsupported Media Type"; break;
        case 422: oss << "Unprocessable Entity"; break;
        case 409: oss << "Conflict"; break;
        case 500: oss << "Internal Server Error"; break;
        default: oss << "Error"; break;
    }
    oss << "\r\nContent-Type: application/json; charset=utf-8\r\n";
    oss << "Content-Length: " << bodyStr.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << bodyStr;
    return oss.str();
}

// Init DB
static bool initDB(std::string& err) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        err = "Failed to open DB";
        if (db) sqlite3_close(db);
        return false;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "email TEXT NOT NULL,"
        "age INTEGER NOT NULL CHECK(age >= 0)"
        ");";
    char* zErr = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErr);
    if (rc != SQLITE_OK) {
        err = "Failed to create table";
        if (zErr) sqlite3_free(zErr);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

// Insert user with prepared statement
static bool insertUser(const std::string& username, const std::string& email, int age, std::string& err, bool& conflict) {
    conflict = false;
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        err = "DB open error";
        if (db) sqlite3_close(db);
        return false;
    }
    const char* sql = "INSERT INTO users(username, email, age) VALUES(?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        err = "Prepare failed";
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), static_cast<int>(username.size()), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), static_cast<int>(email.size()), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, age);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_CONSTRAINT || rc == SQLITE_CONSTRAINT_UNIQUE) {
            conflict = true;
        } else {
            err = "Insert failed";
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

std::string handleRequest(const std::string& requestText) {
    if (requestText.size() > MAX_REQUEST_SIZE) {
        return buildResponse(413, "Request too large");
    }

    // Split headers and body
    size_t headerEnd = requestText.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return buildResponse(400, "Malformed HTTP request");
    }
    std::string headerPart = requestText.substr(0, headerEnd);
    std::string bodyPart = requestText.substr(headerEnd + 4);

    // Request line
    size_t lineEnd = headerPart.find("\r\n");
    std::string requestLine = (lineEnd == std::string::npos) ? headerPart : headerPart.substr(0, lineEnd);
    std::istringstream rl(requestLine);
    std::string method, path, version;
    rl >> method >> path >> version;
    if (method.empty() || path.empty() || version.empty()) {
        return buildResponse(400, "Invalid request line");
    }
    if (method != "POST") {
        return buildResponse(405, "Only POST allowed");
    }
    if (path != "/users") {
        return buildResponse(404, "Not Found");
    }
    if (version.rfind("HTTP/", 0) != 0) {
        return buildResponse(400, "Invalid HTTP version");
    }

    // Parse headers
    std::map<std::string, std::string> headers;
    size_t pos = lineEnd == std::string::npos ? headerPart.size() : lineEnd + 2;
    while (pos < headerPart.size()) {
        size_t next = headerPart.find("\r\n", pos);
        std::string line = headerPart.substr(pos, (next == std::string::npos ? headerPart.size() : next) - pos);
        pos = (next == std::string::npos) ? headerPart.size() : next + 2;
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = toLower(trim(line.substr(0, colon)));
            std::string value = trim(line.substr(colon + 1));
            headers[name] = value;
        }
    }

    // Content-Length required
    if (headers.find("content-length") == headers.end()) {
        return buildResponse(411, "Content-Length required");
    }
    // Validate Content-Length number
    const std::string& clStr = headers["content-length"];
    if (clStr.size() > 10) return buildResponse(400, "Invalid Content-Length");
    for (char c : clStr) if (!std::isdigit(static_cast<unsigned char>(c))) return buildResponse(400, "Invalid Content-Length");
    size_t contentLen = 0;
    try {
        contentLen = static_cast<size_t>(std::stoul(clStr));
    } catch (...) {
        return buildResponse(400, "Invalid Content-Length");
    }
    if (contentLen > MAX_BODY_SIZE) return buildResponse(413, "Payload too large");
    if (contentLen != bodyPart.size()) return buildResponse(400, "Content-Length mismatch");

    // Content-Type
    if (headers.find("content-type") == headers.end()) {
        return buildResponse(415, "Content-Type required");
    }
    std::string ct = toLower(headers["content-type"]);
    // allow potential charset param
    if (ct.find("application/json") != 0) {
        return buildResponse(415, "Unsupported Media Type");
    }

    // Parse JSON
    std::string username, email;
    int age = 0;
    if (!jsonExtractString(bodyPart, "username", username)) {
        return buildResponse(422, "Missing or invalid 'username'");
    }
    if (!jsonExtractString(bodyPart, "email", email)) {
        return buildResponse(422, "Missing or invalid 'email'");
    }
    if (!jsonExtractInt(bodyPart, "age", age)) {
        return buildResponse(422, "Missing or invalid 'age'");
    }

    // Validate fields
    if (!validateUsername(username)) {
        return buildResponse(422, "Invalid username: 3-32 chars, letters/digits/underscore");
    }
    if (!validateEmail(email)) {
        return buildResponse(422, "Invalid email format");
    }
    if (!validateAge(age)) {
        return buildResponse(422, "Invalid age: must be 13..120");
    }

    // Initialize DB
    std::string err;
    if (!initDB(err)) {
        return buildResponse(500, "Database initialization error");
    }

    bool conflict = false;
    if (!insertUser(username, email, age, err, conflict)) {
        if (conflict) {
            return buildResponse(409, "Username already exists");
        }
        return buildResponse(500, "Database insert error");
    }

    std::ostringstream extra;
    extra << "\"user\":{"
          << "\"username\":\"";
    for (char c : username) {
        if (c == '"' || c == '\\') extra << '\\' << c;
        else extra << c;
    }
    extra << "\",\"email\":\"";
    for (char c : email) {
        if (c == '"' || c == '\\') extra << '\\' << c;
        else extra << c;
    }
    extra << "\",\"age\":" << age << "}";

    return buildResponse(201, "User created", extra.str());
}

// 5 test cases
int main() {
    {
        std::string body = "{\"username\":\"alice_01\",\"email\":\"alice@example.com\",\"age\":30}";
        std::ostringstream req;
        req << "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\n\r\n" << body;
        std::cout << "Test 1: Valid create\n" << handleRequest(req.str()) << "\n\n";
    }
    {
        std::string body = "{\"username\":\"alice_01\",\"email\":\"alice2@example.com\",\"age\":31}";
        std::ostringstream req;
        req << "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << body.size() << "\r\n\r\n" << body;
        std::cout << "Test 2: Duplicate username\n" << handleRequest(req.str()) << "\n\n";
    }
    {
        std::string body = "{\"username\":\"bob\",\"email\":\"bob_at_example.com\",\"age\":25}";
        std::ostringstream req;
        req << "POST /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\n\r\n" << body;
        std::cout << "Test 3: Invalid email\n" << handleRequest(req.str()) << "\n\n";
    }
    {
        std::string body = "";
        std::ostringstream req;
        req << "GET /users HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\n\r\n" << body;
        std::cout << "Test 4: Wrong method\n" << handleRequest(req.str()) << "\n\n";
    }
    {
        std::string body = "{\"username\":\"carol\",\"email\":\"carol@example.com\",\"age\":22}";
        std::ostringstream req;
        req << "POST /unknown HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\n\r\n" << body;
        std::cout << "Test 5: Wrong path\n" << handleRequest(req.str()) << "\n\n";
    }
    return 0;
}