#include <iostream>
#include <string>
#include <vector>
#include <libpq-fe.h>

PGconn* get_postgres_connection(const std::string& host, int port, const std::string& database, const std::string& user, const std::string& password) {
    std::string conninfo = "host=" + host +
                           " port=" + std::to_string(port) +
                           " dbname=" + database +
                           " user=" + user +
                           " password=" + password +
                           " connect_timeout=3";
    PGconn* conn = PQconnectdb(conninfo.c_str());
    return conn;
}

struct TestCase {
    std::string label;
    std::string host;
    int port;
    std::string db;
    std::string user;
    std::string pass;
};

int main() {
    std::vector<TestCase> tests = {
        {"Local default port, default db with empty password", "localhost", 5432, "postgres", "postgres", ""},
        {"Local default port with sample credentials", "127.0.0.1", 5432, "postgres", "user", "pass"},
        {"Remote host example", "db.example.com", 5432, "mydb", "myuser", "mypass"},
        {"Wrong port", "localhost", 6543, "postgres", "postgres", "postgres"},
        {"Nonexistent database", "localhost", 5432, "nonexistent", "postgres", "postgres"}
    };

    for (const auto& t : tests) {
        std::cout << "Test: " << t.label << std::endl;
        PGconn* conn = get_postgres_connection(t.host, t.port, t.db, t.user, t.pass);
        if (PQstatus(conn) == CONNECTION_OK) {
            std::cout << "  Success: Connected." << std::endl;
        } else {
            std::cout << "  Failed: " << PQerrorMessage(conn) << std::endl;
        }
        PQfinish(conn);
    }

    return 0;
}