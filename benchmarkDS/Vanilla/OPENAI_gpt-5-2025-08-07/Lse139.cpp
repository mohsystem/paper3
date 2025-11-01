#include <libpq-fe.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

static std::string getEnvOrEmpty(const char* key) {
    const char* v = std::getenv(key);
    return v ? std::string(v) : std::string();
}

bool isModerator(const std::string& username) {
    if (username.empty()) return false;
    std::string host = getEnvOrEmpty("DB_HOST");
    std::string port = getEnvOrEmpty("DB_PORT");
    std::string db   = getEnvOrEmpty("DB_NAME");
    std::string user = getEnvOrEmpty("DB_USER");
    std::string pwd  = getEnvOrEmpty("DB_PASSWORD");
    if (port.empty()) port = "5432";
    if (host.empty() || db.empty() || user.empty()) return false;

    std::string conninfo = "host=" + host + " port=" + port + " dbname=" + db + " user=" + user + " password=" + pwd;
    PGconn* conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return false;
    }

    const char* sql = "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1";
    const char* paramValues[1] = { username.c_str() };
    PGresult* res = PQexecParams(conn, sql, 1, nullptr, paramValues, nullptr, nullptr, 0);

    bool result = false;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) >= 1) {
        char* val = PQgetvalue(res, 0, 0);
        if (val) {
            if (std::strcmp(val, "t") == 0 || std::strcmp(val, "true") == 0 || std::strcmp(val, "1") == 0) {
                result = true;
            }
        }
    }

    PQclear(res);
    PQfinish(conn);
    return result;
}

int main() {
    std::string testUsers[5] = {"alice", "bob", "charlie", "dana", "eric"};
    for (const auto& u : testUsers) {
        bool r = isModerator(u);
        std::cout << u << " -> " << (r ? "true" : "false") << std::endl;
    }
    return 0;
}