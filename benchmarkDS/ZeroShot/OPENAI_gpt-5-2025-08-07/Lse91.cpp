#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <sstream>
#include <cstring>
#include <cctype>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

static const int PORT_CPP = 8082;
static const int MAX_BODY = 2048;
static const int ITERATIONS = 100000;
static const int SALT_LEN = 16;
static const int KEY_LEN = 32;
static const std::string DB_PATH_CPP = "users_cpp.db";

bool init_db(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

bool is_valid_username(const std::string& u) {
    static const std::regex re("^[A-Za-z0-9_]{3,50}$");
    return std::regex_match(u, re);
}

bool is_valid_password(const std::string& p) {
    if (p.size() < 8 || p.size() > 200) return false;
    bool hasLetter = false, hasDigit = false;
    for (unsigned char c : p) {
        if (std::isalpha(c)) hasLetter = true;
        if (std::isdigit(c)) hasDigit = true;
    }
    return hasLetter && hasDigit;
}

std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        unsigned char b = data[i];
        out.push_back(hex[b >> 4]);
        out.push_back(hex[b & 0x0F]);
    }
    return out;
}

bool hash_password(const std::string& password, std::string& salt_hex, std::string& hash_hex) {
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return false;
    unsigned char out[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(), salt, SALT_LEN, ITERATIONS, EVP_sha256(), KEY_LEN, out) != 1) {
        return false;
    }
    salt_hex = to_hex(salt, SALT_LEN);
    hash_hex = to_hex(out, KEY_LEN);
    OPENSSL_cleanse(out, KEY_LEN);
    return true;
}

std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out.push_back(' ');
        } else if (s[i] == '%' && i + 2 < s.size() && std::isxdigit(s[i+1]) && std::isxdigit(s[i+2])) {
            std::string hex = s.substr(i+1, 2);
            char c = (char) strtol(hex.c_str(), nullptr, 16);
            out.push_back(c);
            i += 2;
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

void parse_form(const std::string& body, std::string& username, std::string& password) {
    std::istringstream ss(body);
    std::string part;
    while (std::getline(ss, part, '&')) {
        auto pos = part.find('=');
        if (pos != std::string::npos) {
            std::string k = url_decode(part.substr(0, pos));
            std::string v = url_decode(part.substr(pos + 1));
            if (k == "username") username = v;
            else if (k == "password") password = v;
        }
    }
}

std::string register_user(const std::string& db_path, const std::string& username, const std::string& password) {
    if (!is_valid_username(username)) return "ERR: invalid username";
    if (!is_valid_password(password)) return "ERR: weak password";

    std::string salt_hex, hash_hex;
    if (!hash_password(password, salt_hex, hash_hex)) return "ERR: hashing failed";

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "ERR: database error";
    }
    const char* insert_sql = "INSERT INTO users(username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return "ERR: database error";
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash_hex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_hex.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (rc == SQLITE_DONE) return "OK: registered";
    if (rc == SQLITE_CONSTRAINT) return "ERR: username exists";
    return "ERR: database error";
}

void send_response(int client, int code, const std::string& text) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << code << " \r\n";
    ss << "Content-Type: text/plain; charset=utf-8\r\n";
    ss << "Content-Length: " << text.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << text;
    std::string resp = ss.str();
    send(client, resp.c_str(), resp.size(), 0);
}

void run_server(const std::string& db_path, int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(srv);
        return;
    }
    if (listen(srv, 10) < 0) {
        close(srv);
        return;
    }
    std::cout << "C++ server started on http://127.0.0.1:" << port << std::endl;
    char buf[4096];
    while (true) {
        int client = accept(srv, nullptr, nullptr);
        if (client < 0) continue;
        ssize_t n = recv(client, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(client); continue; }
        buf[n] = '\0';
        std::string req(buf, (size_t)n);
        // Simple parse
        if (req.rfind("POST /register ", 0) != 0) {
            send_response(client, 404, "Not Found");
            close(client);
            continue;
        }
        // Find headers end
        size_t headers_end = req.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        std::string headers = req.substr(0, headers_end);
        std::string content_type;
        int content_length = -1;
        std::istringstream hs(headers);
        std::string line;
        while (std::getline(hs, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                std::string k = line.substr(0, pos);
                std::string v = line.substr(pos+1);
                // trim
                auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
                auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);}).base(), s.end()); };
                ltrim(k); rtrim(k); ltrim(v); rtrim(v);
                std::string kl = k;
                for (auto& c: kl) c = std::tolower(c);
                if (kl == "content-type") content_type = v;
                else if (kl == "content-length") {
                    try { content_length = std::stoi(v); } catch (...) { content_length = -1; }
                }
            }
        }
        if (content_length < 0 || content_length > MAX_BODY) {
            send_response(client, 413, "Payload Too Large");
            close(client);
            continue;
        }
        std::string body = req.substr(headers_end + 4);
        while ((int)body.size() < content_length) {
            ssize_t m = recv(client, buf, sizeof(buf), 0);
            if (m <= 0) break;
            body.append(buf, buf + m);
        }
        if ((int)body.size() < content_length) {
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        std::string username, password;
        parse_form(body.substr(0, content_length), username, password);
        std::string res = register_user(db_path, username, password);
        if (res.rfind("OK", 0) == 0) send_response(client, 201, "Registered");
        else if (res.find("exists") != std::string::npos) send_response(client, 409, "Conflict");
        else send_response(client, 400, "Bad Request");
        close(client);
    }
    close(srv);
}

int main() {
    if (!init_db(DB_PATH_CPP)) {
        std::cerr << "DB init failed\n";
        return 1;
    }
    std::thread srv(run_server, DB_PATH_CPP, PORT_CPP);
    srv.detach();

    // 5 Test cases (direct function calls)
    std::cout << "[Test1] " << register_user(DB_PATH_CPP, "alice_01", "StrongPass123!") << std::endl;
    std::cout << "[Test2] " << register_user(DB_PATH_CPP, "alice_01", "AnotherStrong123!") << std::endl;
    std::cout << "[Test3] " << register_user(DB_PATH_CPP, "bad user!", "StrongPass123!") << std::endl;
    std::cout << "[Test4] " << register_user(DB_PATH_CPP, "bob", "123") << std::endl;
    std::cout << "[Test5] " << register_user(DB_PATH_CPP, "charlie_02", "Y3tAn0ther$trong") << std::endl;

    // Keep running
    while (true) std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}