#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <libpq-fe.h>

static bool isValidHostname(const std::string& host) {
    if (host.empty() || host.size() > 253) return false;
    if (host.front() == '[' && host.back() == ']') {
        std::string inner = host.substr(1, host.size() - 2);
        if (inner.empty()) return false;
        for (char c : inner) {
            if (!std::isxdigit(static_cast<unsigned char>(c)) && c != ':' && c != '.') return false;
        }
        return true;
    }
    for (char c : host) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_')) return false;
    }
    return true;
}

static bool isValidDbName(const std::string& db) {
    if (db.empty() || db.size() > 63) return false;
    for (char c : db) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string normalizeSslMode(const std::string& mode) {
    std::string m = mode;
    for (auto& c : m) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (m == "disable" || m == "allow" || m == "prefer" || m == "require" || m == "verify-ca" || m == "verify-full")
        return m;
    return "require";
}

PGconn* get_postgres_connection(
    const std::string& host,
    int port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password,
    const std::string& sslmode,
    const std::string& sslrootcert,
    int connect_timeout_seconds
) {
    if (!isValidHostname(host)) {
        return nullptr;
    }
    if (port < 1 || port > 65535) {
        return nullptr;
    }
    if (!isValidDbName(dbname)) {
        return nullptr;
    }
    if (user.empty()) {
        return nullptr;
    }
    if (connect_timeout_seconds < 1 || connect_timeout_seconds > 600) {
        connect_timeout_seconds = 10;
    }

    std::string portStr = std::to_string(port);
    std::string timeoutStr = std::to_string(connect_timeout_seconds);
    std::string mode = normalizeSslMode(sslmode);

    const char* keywords[] = {
        "host", "port", "dbname", "user", "password",
        "sslmode", "sslrootcert", "connect_timeout", "application_name", nullptr
    };
    const char* values[] = {
        host.c_str(),
        portStr.c_str(),
        dbname.c_str(),
        user.c_str(),
        password.c_str(),
        mode.c_str(),
        sslrootcert.empty() ? nullptr : sslrootcert.c_str(),
        timeoutStr.c_str(),
        "Task74",
        nullptr
    };

    PGconn* conn = PQconnectdbParams(keywords, values, 0);
    if (!conn) return nullptr;
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

int main() {
    struct Test {
        std::string host;
        int port;
        std::string db;
        std::string user;
        std::string pass;
        std::string sslmode;
        std::string sslrootcert;
        int timeout;
    };

    auto getenvOr = [](const char* k, const char* defv) -> std::string {
        const char* v = std::getenv(k);
        return v ? std::string(v) : std::string(defv);
    };
    auto getenvOrInt = [&](const char* k, int defv) -> int {
        const char* v = std::getenv(k);
        if (!v) return defv;
        try { return std::stoi(v); } catch (...) { return defv; }
    };

    std::vector<Test> tests = {
        {"localhost", 5432, "postgres", "postgres", "", "require", "", 5},
        {"invalid.localdomain", 5432, "postgres", "postgres", "secret", "require", "", 5},
        {"localhost", 1, "postgres", "postgres", "secret", "require", "", 3},
        {"localhost", 5432, "invalid_db", "postgres", "secret", "require", "", 5},
        {
            getenvOr("PGHOST", "localhost"),
            getenvOrInt("PGPORT", 5432),
            getenvOr("PGDATABASE", "postgres"),
            getenvOr("PGUSER", "postgres"),
            getenvOr("PGPASSWORD", ""),
            getenvOr("PGSSLMODE", "require"),
            getenvOr("PGSSLROOTCERT", ""),
            getenvOrInt("PGCONNECT_TIMEOUT", 5)
        }
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        PGconn* conn = get_postgres_connection(
            tests[i].host, tests[i].port, tests[i].db, tests[i].user,
            tests[i].pass, tests[i].sslmode, tests[i].sslrootcert, tests[i].timeout
        );
        if (conn) {
            std::cout << "Test " << (i + 1) << ": Connection successful\n";
            PQfinish(conn);
        } else {
            std::cout << "Test " << (i + 1) << ": Connection failed or invalid input\n";
        }
    }
    return 0;
}