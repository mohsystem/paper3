#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <libpq-fe.h>

static const char* get_env_or_null(const char* key) {
    const char* v = std::getenv(key);
    return (v && *v) ? v : nullptr;
}

bool is_moderator(const std::string& username) {
    if (username.empty()) {
        throw std::invalid_argument("username cannot be empty");
    }

    const char* host = get_env_or_null("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* db   = get_env_or_null("DB_NAME");
    const char* user = get_env_or_null("DB_USER");
    const char* pass = get_env_or_null("DB_PASSWORD");
    const char* sslm = std::getenv("DB_SSLMODE");

    if (!host || !db || !user || !pass) {
        throw std::runtime_error("Missing required DB environment variables");
    }
    const char* default_port = "5432";
    const char* default_ssl  = "prefer";

    const char* keywords[] = {"host","port","dbname","user","password","sslmode",nullptr};
    const char* values[]   = {host, port ? port : default_port, db, user, pass, sslm ? sslm : default_ssl, nullptr};

    PGconn* conn = PQconnectdbParams(keywords, values, 0);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn) ? PQerrorMessage(conn) : "connection failed";
        PQfinish(conn);
        throw std::runtime_error("DB connection error");
    }

    // Optional: set a statement timeout
    {
        PGresult* r = PQexec(conn, "SET statement_timeout = '5s'");
        if (PQresultStatus(r) != PGRES_COMMAND_OK) {
            // ignore; do not leak details
        }
        PQclear(r);
    }

    const char* sql = "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1";
    const char* paramValues[1] = { username.c_str() };
    int paramLengths[1] = { 0 };
    int paramFormats[1] = { 0 };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, paramValues, paramLengths, paramFormats, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQfinish(conn);
        throw std::runtime_error("Query failed");
    }

    bool result = false;
    if (PQntuples(res) > 0) {
        char* v = PQgetvalue(res, 0, 0);
        if (v) {
            std::string s(v);
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            if (s == "t" || s == "true" || s == "1" || s == "yes")
                result = true;
        }
    }

    PQclear(res);
    PQfinish(conn);
    return result;
}

int main() {
    const char* tests[] = {"alice","bob","charlie","diana","eve"};
    for (const char* u : tests) {
        try {
            bool res = is_moderator(u);
            std::cout << "User " << u << " moderator? " << (res ? "true" : "false") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error checking user " << u << ": " << e.what() << std::endl;
        }
    }
    return 0;
}