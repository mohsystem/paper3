#include <iostream>
#include <string>
#include <regex>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sqlite3.h>

static std::regex USER_PATTERN("^[A-Za-z0-9_]{1,32}$");

bool is_valid_username(const std::string& u) {
    return std::regex_match(u, USER_PATTERN);
}

// Accepts input as parameters and returns output
bool is_moderator(sqlite3* db, const std::string& username) {
    if (!db || !is_valid_username(username)) return false;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT is_moderator FROM users WHERE username = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        result = (val == 1);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int v = 0;
            sscanf(s.substr(i + 1, 2).c_str(), "%x", (unsigned int*)&v);
            out.push_back(static_cast<char>(v));
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::string get_query_param(const std::string& query, const std::string& key) {
    std::string k = key + "=";
    size_t pos = 0;
    while (pos < query.size()) {
        size_t amp = query.find('&', pos);
        std::string pair = query.substr(pos, (amp == std::string::npos) ? std::string::npos : amp - pos);
        if (pair.rfind(k, 0) == 0) {
            return url_decode(pair.substr(k.size()));
        }
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

void handle_client(int client_fd, sqlite3* db) {
    char buffer[2048];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) { close(client_fd); return; }
    buffer[n] = '\0';
    std::string req(buffer);
    // Very simple parse: "GET /is_moderator?user=... HTTP/1.1"
    std::string line = req.substr(0, req.find("\r\n"));
    std::string method, path, version;
    {
        size_t p1 = line.find(' ');
        size_t p2 = line.find(' ', p1 + 1);
        method = (p1 != std::string::npos) ? line.substr(0, p1) : "";
        path = (p1 != std::string::npos && p2 != std::string::npos) ? line.substr(p1 + 1, p2 - p1 - 1) : "";
        version = (p2 != std::string::npos) ? line.substr(p2 + 1) : "";
    }
    int status = 200;
    std::string body = "{\"isModerator\":false}";
    if (method == "GET") {
        std::string route = path;
        std::string query;
        size_t qpos = path.find('?');
        if (qpos != std::string::npos) {
            route = path.substr(0, qpos);
            query = path.substr(qpos + 1);
        }
        if (route == "/is_moderator") {
            std::string user = get_query_param(query, "user");
            bool res = is_moderator(db, user);
            body = std::string("{\"isModerator\":") + (res ? "true" : "false") + "}";
        } else {
            status = 404;
            body = "{\"error\":\"Not Found\"}";
        }
    } else {
        status = 405;
        body = "{\"error\":\"Method Not Allowed\"}";
    }

    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
             status, body.size());
    send(client_fd, header, strlen(header), 0);
    send(client_fd, body.c_str(), body.size(), 0);
    close(client_fd);
}

void server_loop(sqlite3* db, int port) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) < 0) { close(sfd); return; }
    if (listen(sfd, 8) < 0) { close(sfd); return; }
    // Accept a few connections then exit
    for (int i = 0; i < 5; ++i) {
        int cfd = accept(sfd, nullptr, nullptr);
        if (cfd >= 0) handle_client(cfd, db);
    }
    close(sfd);
}

void init_db(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS users("
        " username TEXT PRIMARY KEY,"
        " is_moderator INTEGER NOT NULL CHECK(is_moderator IN (0,1))"
        ");";
    sqlite3_exec(db, create_sql, nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('alice',1)", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('bob',0)", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('charlie',0)", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('admin_1',1)", nullptr, nullptr, nullptr);
}

int main() {
    // Open in-memory SQLite
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "DB open failed\n";
        return 1;
    }
    init_db(db);

    // Start server in background
    std::thread srv([&]() { server_loop(db, 8081); });

    // 5 test cases (direct function calls)
    std::vector<std::string> tests = {"alice", "bob", "charlie", "admin_1", "unknown_user"};
    for (const auto& u : tests) {
        bool r = is_moderator(db, u);
        std::cout << u << " -> " << (r ? "true" : "false") << std::endl;
    }

    srv.join();
    sqlite3_close(db);
    return 0;
}