#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <libpq-fe.h>

static std::string getEnvOrDefault(const char* key, const std::string& defVal) {
    const char* v = std::getenv(key);
    if (!v || std::string(v).empty()) return defVal;
    return std::string(v);
}

bool isModerator(const std::string& username) {
    std::string host = getEnvOrDefault("DB_HOST", "localhost");
    std::string port = getEnvOrDefault("DB_PORT", "5432");
    std::string db   = getEnvOrDefault("DB_NAME", "postgres");
    std::string user = getEnvOrDefault("DB_USER", "postgres");
    std::string pass = getEnvOrDefault("DB_PASS", "");

    std::string conninfo = "host=" + host + " port=" + port + " dbname=" + db + " user=" + user + " password=" + pass;
    PGconn* conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        if (conn) PQfinish(conn);
        return false;
    }

    const char* paramValues[1];
    paramValues[0] = username.c_str();

    PGresult* res = PQexecParams(
        conn,
        "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1",
        1,              // number of params
        nullptr,        // param types
        paramValues,    // param values
        nullptr,        // param lengths
        nullptr,        // param formats
        0               // result format: 0 = text
    );

    bool result = false;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        char* val = PQgetvalue(res, 0, 0);
        std::string sval = val ? std::string(val) : std::string("");
        std::string lower = sval;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        result = (lower == "t" || lower == "true" || lower == "1");
    }

    if (res) PQclear(res);
    PQfinish(conn);
    return result;
}

int main() {
    std::string users[5] = {"alice", "bob", "charlie", "diana", "eve"};
    for (const auto& u : users) {
        bool v = isModerator(u);
        std::cout << "User: " << u << " -> " << (v ? "true" : "false") << std::endl;
    }
    return 0;
}