#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>

// Connect to the database and initialize schema/data
sqlite3* connectAndInitDb() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        return nullptr;
    }
    const char* create_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT, is_moderator INTEGER);";
    const char* insert_sql =
        "INSERT INTO users (id, username, is_moderator) VALUES "
        "(1,'alice',1),(2,'bob',0),(3,'carol',1),(4,'dave',0),(5,'eve',0);";
    char* err = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return nullptr;
    }
    if (sqlite3_exec(db, insert_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return nullptr;
    }
    return db;
}

// Execute a query to check if the user is a moderator
bool isModerator(sqlite3* db, long userId) {
    const char* sql = "SELECT is_moderator FROM users WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(userId));
    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        result = (val == 1);
    }
    sqlite3_finalize(stmt);
    return result;
}

static std::string urlDecode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int v = 0;
            std::istringstream iss(s.substr(i + 1, 2));
            iss >> std::hex >> v;
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

static std::string getQueryParam(const std::string& query, const std::string& key) {
    std::string k = key + "=";
    size_t pos = 0;
    while (pos < query.size()) {
        size_t amp = query.find('&', pos);
        std::string pair = query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        if (pair.rfind(k, 0) == 0) {
            return urlDecode(pair.substr(k.size()));
        }
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

// Define a simple web app route and server
void runServer(sqlite3* db, int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }
    if (listen(server_fd, 16) < 0) {
        close(server_fd);
        return;
    }

    // Simple loop for a short duration or few connections for demo
    auto handle_client = [&](int cfd) {
        char buf[2048];
        ssize_t n = read(cfd, buf, sizeof(buf) - 1);
        if (n <= 0) { close(cfd); return; }
        buf[n] = '\0';
        std::string req(buf);
        // Parse first line: GET /isModerator?userId=... HTTP/1.1
        std::istringstream iss(req);
        std::string method, path, version;
        iss >> method >> path >> version;

        bool ok = false;
        bool result = false;
        if (method == "GET") {
            size_t qpos = path.find('?');
            std::string route = (qpos == std::string::npos) ? path : path.substr(0, qpos);
            std::string query = (qpos == std::string::npos) ? "" : path.substr(qpos + 1);
            if (route == "/isModerator") {
                std::string uidStr = getQueryParam(query, "userId");
                if (!uidStr.empty()) {
                    try {
                        long uid = std::stol(uidStr);
                        result = isModerator(db, uid);
                    } catch (...) {
                        result = false;
                    }
                }
                ok = true;
            }
        }

        std::string body;
        int status = 200;
        if (ok) {
            body = result ? "true" : "false";
        } else {
            status = 404;
            body = "Not Found";
        }
        std::ostringstream resp;
        resp << "HTTP/1.1 " << status << " OK\r\n"
             << "Content-Type: text/plain; charset=utf-8\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        std::string out = resp.str();
        write(cfd, out.c_str(), out.size());
        close(cfd);
    };

    // Accept a few connections then exit to keep demo short
    int max_conn = 5;
    while (max_conn-- > 0) {
        int cfd = accept(server_fd, nullptr, nullptr);
        if (cfd >= 0) {
            handle_client(cfd);
        }
    }
    close(server_fd);
}

int main() {
    sqlite3* db = connectAndInitDb();
    if (!db) {
        std::cerr << "Failed to init DB\n";
        return 1;
    }

    std::thread srv([&]{
        runServer(db, 8082);
    });
    srv.detach();

    std::cout << "C++ server started on http://localhost:8082/isModerator?userId={id}\n";

    // 5 test cases
    std::vector<long> ids = {1, 2, 3, 4, 999};
    for (auto id : ids) {
        bool res = isModerator(db, id);
        std::cout << "Test userId=" << id << " -> isModerator=" << (res ? "true" : "false") << "\n";
    }

    // Small sleep to allow manual testing quickly
    usleep(3000 * 1000);
    sqlite3_close(db);
    return 0;
}