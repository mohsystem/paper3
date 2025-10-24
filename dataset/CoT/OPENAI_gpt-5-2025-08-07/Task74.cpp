// Chain-of-Through:
// 1) Problem: Provide a function returning a PostgreSQL connection (libpq PGconn*).
// 2) Security: No hardcoded secrets; avoid logging credentials; enforce SSL by default; timeouts; escape conninfo values.
// 3) Secure coding: Build conninfo with proper escaping; set sslmode, connect_timeout, application_name.
// 4) Review: Ensure PQfinish is called; avoid leaking secrets; validate inputs.
// 5) Output: Final secure function with 5 test cases.

#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <libpq-fe.h>

static std::string escape_conninfo_value(const std::string& v) {
    // Conninfo values can be single-quoted with backslash escapes
    std::string out;
    out.reserve(v.size() * 2 + 2);
    out.push_back('\'');
    for (char c : v) {
        if (c == '\'' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    out.push_back('\'');
    return out;
}

PGconn* get_postgres_connection(
    const std::string& host,
    int port,
    const std::string& database,
    const std::string& user,
    const std::string& password,
    bool require_ssl = true,
    int connect_timeout = 5
) {
    if (host.empty()) throw std::invalid_argument("host must not be empty");
    if (database.empty()) throw std::invalid_argument("database must not be empty");
    if (user.empty()) throw std::invalid_argument("user must not be empty");
    if (port <= 0 || port > 65535) throw std::invalid_argument("port must be in range 1..65535");
    int timeout = std::max(1, connect_timeout);

    std::string sslmode = require_ssl ? "require" : "prefer";

    std::string conninfo;
    conninfo.reserve(256);
    conninfo += "host=" + escape_conninfo_value(host);
    conninfo += " port=" + std::to_string(port);
    conninfo += " dbname=" + escape_conninfo_value(database);
    conninfo += " user=" + escape_conninfo_value(user);
    if (!password.empty()) {
        conninfo += " password=" + escape_conninfo_value(password);
    }
    conninfo += " sslmode=" + sslmode;
    conninfo += " connect_timeout=" + std::to_string(timeout);
    conninfo += " application_name='Task74App'";

    PGconn* conn = PQconnectdb(conninfo.c_str());
    return conn; // Caller must check status and PQfinish
}

static void run_test(const std::string& label,
                     const std::string& host, int port,
                     const std::string& db, const std::string& user,
                     const std::string& password, bool require_ssl, int timeout) {
    std::cout << "Test: " << label << "\n";
    PGconn* conn = nullptr;
    try {
        conn = get_postgres_connection(host, port, db, user, password, require_ssl, timeout);
        if (PQstatus(conn) == CONNECTION_OK) {
            std::cout << "  Connection established successfully\n";
        } else {
            std::cout << "  Failed to connect: " << PQerrorMessage(conn);
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    if (conn) {
        PQfinish(conn);
        std::cout << "  Connection closed\n";
    }
}

int main() {
    const char* envUser = std::getenv("PGUSER");
    const char* envPass = std::getenv("PGPASSWORD");
    std::string user = envUser ? envUser : "postgres";
    std::string pass = envPass ? envPass : "";

    // 5 test cases (these may fail depending on the environment)
    run_test("Local default (SSL required, short timeout)", "127.0.0.1", 5432, "postgres", user, pass, true, 2);
    run_test("Invalid port", "127.0.0.1", 1, "postgres", user, pass, true, 2);
    run_test("Invalid host", "no-such-host.invalid", 5432, "postgres", user, pass, true, 2);
    run_test("Non-SSL prefer", "localhost", 5432, "postgres", user, pass, false, 2);
    run_test("Custom DB name", "localhost", 5432, "mydb", user, pass, true, 2);

    return 0;
}