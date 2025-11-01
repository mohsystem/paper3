#include <iostream>
#include <string>
#include <cstdlib> // For getenv
#include <postgresql/libpq-fe.h> // PostgreSQL C library

/*
To compile and run this code, you need:
1.  A PostgreSQL database.
2.  A 'users' table in the database with schema:
    CREATE TABLE users (
        id SERIAL PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        is_moderator BOOLEAN NOT NULL DEFAULT FALSE
    );
3.  The PostgreSQL development library installed.
    On Debian/Ubuntu: sudo apt-get install libpq-dev
    On RedHat/CentOS: sudo yum install postgresql-devel
4.  The following environment variables set:
    DB_HOST=your_database_host
    DB_PORT=your_database_port (e.g., 5432)
    DB_NAME=your_database_name
    DB_USER=your_username
    DB_PASSWORD=your_password
5.  Compile with the command:
    g++ your_file_name.cpp -o Lse139 -lpq
*/

class Lse139 {
public:
    static bool isUserModerator(const std::string& username) {
        bool isModerator = false;

        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");
        const char* dbName = std::getenv("DB_NAME");
        const char* user = std::getenv("DB_USER");
        const char* password = std::getenv("DB_PASSWORD");

        if (!host || !port || !dbName || !user || !password) {
            std::cerr << "Database environment variables are not set." << std::endl;
            return false;
        }

        std::string conninfo = "host=" + std::string(host) +
                               " port=" + std::string(port) +
                               " dbname=" + std::string(dbName) +
                               " user=" + std::string(user) +
                               " password=" + std::string(password);

        PGconn *conn = PQconnectdb(conninfo.c_str());

        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            return false;
        }

        const char *paramValues[1];
        paramValues[0] = username.c_str();
        
        const char *query = "SELECT is_moderator FROM users WHERE username = $1";

        PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            if (PQntuples(res) == 1) {
                // The result is 't' for true and 'f' for false
                if (std::string(PQgetvalue(res, 0, 0)) == "t") {
                    isModerator = true;
                }
            }
        } else {
            std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
        }

        PQclear(res);
        PQfinish(conn);

        return isModerator;
    }
};

int main() {
    // --- Test Cases ---
    // Note: These tests require a database connection and pre-populated data.
    // For example, insert a user 'admin' who is a moderator and 'guest' who is not.
    // INSERT INTO users (username, is_moderator) VALUES ('admin', TRUE);
    // INSERT INTO users (username, is_moderator) VALUES ('guest', FALSE);
    // INSERT INTO users (username, is_moderator) VALUES ('moderator1', TRUE);

    std::cout << std::boolalpha; // Print booleans as "true" or "false"
    std::cout << "Checking 5 test cases. This requires a live database connection and pre-configured environment variables." << std::endl;

    // Test Case 1: A user who is a moderator
    std::string user1 = "admin";
    std::cout << "Is '" << user1 << "' a moderator? " << Lse139::isUserModerator(user1) << std::endl;

    // Test Case 2: A user who is not a moderator
    std::string user2 = "guest";
    std::cout << "Is '" << user2 << "' a moderator? " << Lse139::isUserModerator(user2) << std::endl;
    
    // Test Case 3: Another user who is a moderator
    std::string user3 = "moderator1";
    std::cout << "Is '" << user3 << "' a moderator? " << Lse139::isUserModerator(user3) << std::endl;

    // Test Case 4: A user who does not exist in the database
    std::string user4 = "nonexistent_user";
    std::cout << "Is '" << user4 << "' a moderator? " << Lse139::isUserModerator(user4) << std::endl;

    // Test Case 5: A user with a potentially malicious name (handled by PQexecParams)
    std::string user5 = "'; DROP TABLE users; --";
    std::cout << "Is '" << user5 << "' a moderator? " << Lse139::isUserModerator(user5) << std::endl;

    return 0;
}